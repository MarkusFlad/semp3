#include "PlaybackController.hpp"
#include <boost/filesystem/operations.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

using std::string;
using std::vector;
using std::map;
using std::find;
using std::sort;
using std::ifstream;
using std::ofstream;
using std::endl;
using std::cout;
using std::istringstream;
using boost::filesystem::path;
using boost::filesystem::directory_iterator;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::system::error_code;

//==============================================================================
//-------------------------- PlaybackController --------------------------------
//==============================================================================
PlaybackController::PlaybackController (const path& albumsPath,
        Mp3Player& mp3Player)
: _albumsPath (albumsPath)
, _mp3Player (mp3Player)
, _mp3PlayerListener (*this, 100 /* Frames until storing the title position */)
, _currentTitlePosition (path(), 0)
, _secondsPlayed (0.0)
, _presentingAlbums (false) {
    for (auto itAF = directory_iterator(albumsPath); itAF != directory_iterator();
         ++itAF) {
        const path& file = *itAF;
        if (is_directory(file)) {
            _albums.push_back(file);
        }
    }
    sort (_albums.begin(), _albums.end());
    path currentAlbumFile = getCurrentAlbumFile(albumsPath);
    ifstream icaf (currentAlbumFile.string());
    string line;
    getline (icaf, line);
    _currentAlbum = line;
    icaf.close();
    for (auto album : _albums) {
        DirectoryList& mp3Files = _albumMap[album];
        for (auto itFile = directory_iterator(album);
             itFile != directory_iterator(); ++itFile) {
            const path& file = *itFile;
            if (file.extension() == ".mp3") {
                mp3Files.push_back(file);
            }
        }
        sort (mp3Files.begin(), mp3Files.end());
        if (album == _currentAlbum) {
            _currentTitlePosition = getCurrentTitlePosition (album, mp3Files);
        }
    }
    if (!exists (_currentAlbum)) {
        DirectoryList::const_iterator albumsBegin = _albums.begin();
        if (albumsBegin != _albums.end()) {
            _currentAlbum = *albumsBegin;
            _currentTitlePosition = getCurrentTitlePosition (_currentAlbum,
                    _albumMap[_currentAlbum]);
        }
    }
    _mp3Player.addListener(&_mp3PlayerListener);
    _mp3Player.load(_currentTitlePosition.getTitle());
    _mp3Player.jumpTo(_currentTitlePosition.getFrameCount());
}
void PlaybackController::setCurrentTitlePosition (const TitlePosition&
                                                  titlePosition) {
    _currentTitlePosition = titlePosition;
    updateCurrentTitleFile();
}
void PlaybackController::setCurrentTitlePosition (int frameCount) {
    // During the album selection the current title position is not stored.
    if (_presentingAlbums) {
        return;
    }
    // Note: This is an efficient way to set the new title position while
    // keeping the immutable semantics of the TitlePosition class.
    _currentTitlePosition = TitlePosition (std::move(_currentTitlePosition),
                                           frameCount);
    updateCurrentTitleFile();
}
void PlaybackController::updateCurrentTitleFile () {
    path currentTitleFile = getCurrentTitleFile (_currentAlbum);
    ofstream ocff (currentTitleFile.string());
    ocff << _currentTitlePosition.getFrameCount() << endl;
    ocff << _currentTitlePosition.getTitle().string() << endl;
    ocff.close();
}
path PlaybackController::getCurrentAlbumFile (const path& albums) {
    path currentAlbumFile = albums;
    currentAlbumFile /= "current-album.cfg";
    return currentAlbumFile;
}
path PlaybackController::getCurrentTitleFile (const path& album) {
    path currentTitleFile = album;
    currentTitleFile /= "current-title.cfg";
    return currentTitleFile;
}
PlaybackController::TitlePosition PlaybackController::getCurrentTitlePosition (
                            const Path& album, const DirectoryList& mp3Files) {
    path currentTitleFile = getCurrentTitleFile (album);
    ifstream ictf (currentTitleFile.string());
    string frameCountLine;
    getline (ictf, frameCountLine);
    istringstream issFrameCount (frameCountLine);
    int frameCount;
    issFrameCount >> frameCount;
    string title;
    getline (ictf, title);
    ictf.close();
    TitlePosition titlePosition (path(title), frameCount);
    if (!exists (titlePosition.getTitle())) {
        DirectoryList::const_iterator mp3FilesBegin = mp3Files.begin();
        if (mp3FilesBegin != mp3Files.end()) {
            titlePosition = TitlePosition (*mp3FilesBegin, 0);
        }
    }
    return titlePosition;
}
void PlaybackController::pause() {
    _mp3Player.pause();
}
void PlaybackController::next() {
    const DirectoryList& mp3Files = _albumMap[_currentAlbum];
    auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                           _currentTitlePosition.getTitle());
    if (itCurrent != mp3Files.end()) {
        itCurrent++;
        if (itCurrent != mp3Files.end()) {
            const path& currentTitle = *itCurrent;
            setCurrentTitlePosition (TitlePosition (currentTitle, 0));
            _mp3Player.load (currentTitle);
        }
    }
}
void PlaybackController::back() {
    if (_secondsPlayed > 30.0) {
        _mp3Player.jumpToBegin();
    } else {
        const DirectoryList& mp3Files = _albumMap[_currentAlbum];
        auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                               _currentTitlePosition.getTitle());
        if (itCurrent != mp3Files.begin()) {
            itCurrent--;
            const path& currentTitle = *itCurrent;
            setCurrentTitlePosition (TitlePosition (currentTitle, 0));
            _mp3Player.load (currentTitle);
        } else {
            _mp3Player.jumpToBegin();
        }
    }
}
void PlaybackController::presentNextAlbum() {
    _presentingAlbums = true;
    auto itCurrent = find (_albums.begin(), _albums.end(), _currentAlbum);
    if (itCurrent == _albums.end()) {
        itCurrent = _albums.begin();
    } else {
        itCurrent++;
    }
    if (itCurrent == _albums.end()) {
        itCurrent = _albums.begin();
    }
    if (itCurrent == _albums.end()) {
        return;
    }
    const path& currentAlbum = *itCurrent;
    map<path, DirectoryList>::const_iterator itMap =
            _albumMap.find(currentAlbum);
    if (itMap == _albumMap.end()) {
        return;
    }
    const DirectoryList& mp3Files = itMap->second;
    if (mp3Files.size() == 0) {
        return;
    }
    const path& firstTitleInAlbum = *(mp3Files.begin());
    _mp3Player.load (firstTitleInAlbum);
    path currentAlbumFile = getCurrentAlbumFile (_albumsPath);
    ofstream ocff (currentAlbumFile.string());
    ocff << currentAlbum.string() << endl;
    ocff.close();
    _currentAlbum = currentAlbum;
}
void PlaybackController::resumeAlbum() {
    _presentingAlbums = false;
    map<path, DirectoryList>::const_iterator itMap =
            _albumMap.find(_currentAlbum);
    if (itMap == _albumMap.end()) {
        return;
    }
    const DirectoryList& mp3Files = itMap->second;
    _currentTitlePosition = getCurrentTitlePosition (_currentAlbum, mp3Files);
    _mp3Player.load(_currentTitlePosition.getTitle());
    _mp3Player.jumpTo(_currentTitlePosition.getFrameCount());
}
//==============================================================================
//----------------- PlaybackController::Mp3PlayerListener-----------------------
//==============================================================================
PlaybackController::Mp3PlayerListener::Mp3PlayerListener (
        PlaybackController& playbackController, int titlePositionUpdateCycle)
: _playbackController (playbackController)
, _titlePositionUpdateCycle (titlePositionUpdateCycle)
, _frameCountOfLastUpdateCycle (0)
, _secondsPlayed (playbackController._secondsPlayed) {
}
void PlaybackController::Mp3PlayerListener::mpg123Version (
        const string& message) {
}
void PlaybackController::Mp3PlayerListener::titleLoaded (
        const Mp3Title& title) {
    cout << "Playing title " << title.toString() << endl;
    _frameCountOfLastUpdateCycle = 0;
}
void PlaybackController::Mp3PlayerListener::playStatus (int framecount,
        int framesLeft, float seconds, float secondsLeft) {
    _secondsPlayed = seconds;
    if (framecount - _frameCountOfLastUpdateCycle > _titlePositionUpdateCycle) {
        _frameCountOfLastUpdateCycle = framecount;
        _playbackController.setCurrentTitlePosition(framecount);
    }
}
void PlaybackController::Mp3PlayerListener::playingStopped (
        bool endOfSongReached) {
    if (!_playbackController._presentingAlbums) {
        _playbackController.next();
    }
}
void PlaybackController::Mp3PlayerListener::playingPaused() {
}
void PlaybackController::Mp3PlayerListener::playingUnpaused() {
}
void PlaybackController::Mp3PlayerListener::playingErrorOccurred (
        const string& errorMessage) {
}
void PlaybackController::Mp3PlayerListener::mpg123CommunicationProblem (
        const error_code& error) {
}
void PlaybackController::Mp3PlayerListener::mpg123Terminated (
        int waitpidStatus) {
}
//==============================================================================
//------------------- PlaybackController::TitlePosition ------------------------
//==============================================================================
PlaybackController::TitlePosition::TitlePosition (const Path& title,
        int frameCount)
: _title (title)
, _frameCount (frameCount) {
}
PlaybackController::TitlePosition::TitlePosition (TitlePosition&& titlePosition,
                                                  int frameCount)
: _title (titlePosition._title)
, _frameCount (frameCount) {
}
PlaybackController::Path PlaybackController::TitlePosition::getTitle() const {
    return _title;
}
int PlaybackController::TitlePosition::getFrameCount() const {
    return _frameCount;
}
