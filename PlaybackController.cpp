#include "PlaybackController.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>

using std::string;
using std::max;
using std::min;
using std::vector;
using std::distance;
using std::advance;
using std::map;
using std::find;
using std::sort;
using std::endl;
using std::cout;
using std::istringstream;
using std::ostringstream;
using boost::optional;
using boost::filesystem::path;
using boost::filesystem::directory_iterator;
using boost::filesystem::exists;
using boost::filesystem::is_directory;
using boost::system::error_code;

//==============================================================================
//-------------------------- PlaybackController --------------------------------
//==============================================================================
const string PlaybackController::CURRENT_ALBUM_FILENAME ("current-album.cfg");
const string PlaybackController::CURRENT_TITLE_FILENAME ("current-title.cfg");

PlaybackController::PlaybackController (const path& albumsPath,
        Mp3Player& mp3Player)
: _albumsPath (albumsPath)
, _mp3Player (mp3Player)
, _mp3PlayerListener (*this, 100 /* Frames until storing the title position */)
, _albumMap (getAlbumMap(albumsPath))
, _currentTitlePosition (boost::none)
, _secondsPlayed (0.0)
, _presentingAlbums (false) {
    for (auto albumMapping : _albumMap) {
        _albums.push_back (albumMapping.first);
    }
    sort (_albums.begin(), _albums.end());
    _currentAlbumInfo = RebootSafeString (albumsPath, CURRENT_ALBUM_FILENAME);
    _currentAlbum = _currentAlbumInfo.getValue();
    if (!exists (_currentAlbum)) {
        DirectoryList::const_iterator albumsBegin = _albums.begin();
        if (albumsBegin != _albums.end()) {
            _currentAlbum = *albumsBegin;
        }
    }
    _mp3Player.addListener(&_mp3PlayerListener);
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
    _currentTitlePosition = optional<TitlePosition>(
            TitlePosition(std::move(_currentTitlePosition.get()), frameCount));
    updateCurrentTitleFile();
}
optional<PlaybackController::TitlePosition> PlaybackController::
        getCurrentTitlePosition (const Path& album) {
    _currentTitleInfo = RebootSafeString(album, CURRENT_TITLE_FILENAME);
    istringstream iss (_currentTitleInfo.getValue());
    string frameCountLine;
    getline (iss, frameCountLine);
    istringstream issFrameCount (frameCountLine);
    int frameCount;
    issFrameCount >> frameCount;
    string title;
    getline (iss, title);
    const TitlePosition titlePosition (path(title), frameCount);
    if (!exists (titlePosition.getTitle())) {
        map<path, DirectoryList>::const_iterator itMap = _albumMap.find(album);
        if (itMap == _albumMap.end()) {
            return boost::none;
        }
        const DirectoryList& mp3Files = itMap->second;
        DirectoryList::const_iterator mp3FilesBegin = mp3Files.begin();
        if (mp3FilesBegin != mp3Files.end()) {
            return optional<TitlePosition> (TitlePosition (*mp3FilesBegin, 0));
        } else {
            return boost::none;
        }
    }
    return titlePosition;
}
void PlaybackController::updateCurrentTitleFile () {
    if (!_currentTitleInfo.isValid()) {
        _currentTitleInfo = RebootSafeString(_currentAlbum,
                CURRENT_TITLE_FILENAME);
    }
    ostringstream ost;
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        ost << currentTitlePosition.getFrameCount() << endl;
        ost << currentTitlePosition.getTitle().string() << endl;
        _currentTitleInfo = RebootSafeString(_currentTitleInfo, ost.str());
    }
}
map<path, vector<path>> PlaybackController::getAlbumMap(const Path& albums) {
    map<path, vector<path>> albumMap;
    for (auto itAF = directory_iterator(albums); itAF != directory_iterator();
         ++itAF) {
        const path& file = *itAF;
        if (is_directory(file)) {
            map<Path, DirectoryList> nextAlbumMap = getAlbumMap(file);
            albumMap.insert(nextAlbumMap.begin(), nextAlbumMap.end());
        } else if (file.extension() == ".mp3") {
            DirectoryList& mp3Files = albumMap[albums];
            mp3Files.push_back(file);
        }
    }
    map<Path, DirectoryList>::iterator itFound = albumMap.find(albums);
    if (itFound != albumMap.end()) {
        DirectoryList& mp3Files = itFound->second;
        sort (mp3Files.begin(), mp3Files.end());
    }
    return albumMap;
}
bool PlaybackController::resume() {
    _currentTitlePosition = getCurrentTitlePosition (_currentAlbum);
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        _mp3Player.load(currentTitlePosition.getTitle());
        _mp3Player.jumpTo(currentTitlePosition.getFrameCount());
        return true;
    } else {
        return false;
    }
}
void PlaybackController::pause() {
    _mp3Player.pause();
}
bool PlaybackController::next() {
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        const DirectoryList& mp3Files = _albumMap[_currentAlbum];
        auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                               currentTitlePosition.getTitle());
        if (itCurrent != mp3Files.end()) {
            itCurrent++;
            if (itCurrent != mp3Files.end()) {
                const path& currentTitle = *itCurrent;
                setCurrentTitlePosition (TitlePosition (currentTitle, 0));
                _mp3Player.load (currentTitle);
            }
        }
        return true;
    }
    return false;
}
bool PlaybackController::back() {
    if (_secondsPlayed > 30.0) {
        _mp3Player.jumpToBegin();
        return true;
    } else if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        const DirectoryList& mp3Files = _albumMap[_currentAlbum];
        auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                               currentTitlePosition.getTitle());
        if (itCurrent != mp3Files.begin()) {
            itCurrent--;
            const path& currentTitle = *itCurrent;
            setCurrentTitlePosition (TitlePosition (currentTitle, 0));
            _mp3Player.load (currentTitle);
        } else {
            _mp3Player.jumpToBegin();
        }
        return true;
    }
    return false;
}
void PlaybackController::jumpToAlbum (int n) {
    auto itCurrent = _albums.begin();
    if (n > 0) {
        advance (itCurrent, min<int> (_albums.size() - 1, n - 1));
    }
    if (itCurrent != _albums.end()) {
        _currentAlbum = *itCurrent;
        _currentAlbumInfo = RebootSafeString (_currentAlbumInfo,
                                              _currentAlbum.string());
        resume();
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
    _currentAlbumInfo = RebootSafeString (_currentAlbumInfo, currentAlbum.string());
    _currentAlbum = currentAlbum;
}
void PlaybackController::resumeAlbum() {
    _presentingAlbums = false;
    resume();
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
