#include "PlaybackController.hpp"
#include <boost/filesystem/operations.hpp>
#include <boost/optional.hpp>
#include <boost/none.hpp>
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

using std::string;
using std::max;
using std::min;
using std::vector;
using std::queue;
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
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::seconds;
using boost::posix_time::time_duration;

//==============================================================================
//-------------------------- PlaybackController --------------------------------
//==============================================================================
const string PlaybackController::CURRENT_ALBUM_FILENAME ("current-album.cfg");
const string PlaybackController::CURRENT_TITLE_FILENAME ("current-title.cfg");
const time_duration PlaybackController::FPFI_DURATION (seconds(3));

PlaybackController::PlaybackController (const path& albumsPath,
                                        const path& spokenNumbersPath,
                                        Mp3Player& mp3Player)
: _albumsPath (albumsPath)
, _mp3Player (mp3Player)
, _albumMap (getAlbumMap(albumsPath))
, _currentTitlePosition (boost::none)
, _titlePositionUpdateCycle (100 /* Frames until storing the title position */)
, _frameCountOfLastUpdateCycle (0)
, _frameCountPlayed (0)
, _frameCountTotal (0)
, _secondsPlayed (0.0)
, _fastPlayFactor (0)
, _fastForwardWaitsForLoadCompleted (false)
, _fastBackwardsWaitsForLoadCompleted (false)
, _fastBackwardsWaitsForJumpCompleted (false)
, _numberOfFastPlayedTitles (0)
, _fastPlayFactorUpdateTime (microsec_clock::local_time())
, _paused (false)
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
    for (auto itSN = directory_iterator(spokenNumbersPath);
         itSN != directory_iterator(); ++itSN) {
        const path& file = *itSN;
        if (!is_directory(file) && file.extension() == ".mp3") {
            istringstream iss(file.filename().string());
            int number;
            iss >> number;
            _spokenNumberMap[number] = file;
        }
    }
    _mp3Player.addListener(this);
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
optional<path> PlaybackController::getFirstTitle () const {
    auto itFoundInMap = _albumMap.find(_currentAlbum);
    if (itFoundInMap != _albumMap.end()) {
        const DirectoryList& mp3Files = itFoundInMap->second;
        auto itFirst = mp3Files.begin();
        if (itFirst != mp3Files.end()) {
            return optional<path>(*itFirst);
        }
    }
    return boost::none;
}
optional<path> PlaybackController::getNextTitle (int stepSize,
                                                 bool wrapAround) const {
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        auto itFoundInMap = _albumMap.find(_currentAlbum);
        if (itFoundInMap != _albumMap.end()) {
            const DirectoryList& mp3Files = itFoundInMap->second;
            auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                                   currentTitlePosition.getTitle());
            if (itCurrent != mp3Files.end()) {
                int i=0;
                while (i < stepSize && itCurrent != mp3Files.end()) {
                    i++;
                    itCurrent++;
                }
                if (wrapAround && itCurrent == mp3Files.end()) {
                    itCurrent = mp3Files.begin();
                }
                if (itCurrent != mp3Files.end()) {
                    return optional<path>(*itCurrent);
                }
            }
        }
    }
    return boost::none;
}
optional<path> PlaybackController::getPreviousTitle (int stepSize) const {
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        auto itFoundInMap = _albumMap.find(_currentAlbum);
        if (itFoundInMap != _albumMap.end()) {
            const DirectoryList& mp3Files = itFoundInMap->second;
            auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                                   currentTitlePosition.getTitle());
            if (itCurrent != mp3Files.begin()) {
                int i=0;
                while (i < stepSize && itCurrent != mp3Files.begin()) {
                    i++;
                    itCurrent--;
                }
                return optional<path>(*itCurrent);
            }
        }
    }
    return boost::none;
}
bool PlaybackController::isLastTitle () const {
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        auto itFoundInMap = _albumMap.find(_currentAlbum);
        if (itFoundInMap != _albumMap.end()) {
            const DirectoryList& mp3Files = itFoundInMap->second;
            auto itCurrent = find (mp3Files.begin(), mp3Files.end(),
                                   currentTitlePosition.getTitle());
            if (itCurrent != mp3Files.end()) {
                itCurrent++;
                return (itCurrent == mp3Files.end());
            }
        }
    }
    return false;
}
int PlaybackController::getCurrentTitleNumber () const {
    int n=0;
    if (_currentTitlePosition) {
        TitlePosition currentTitlePosition = _currentTitlePosition.get();
        auto itFoundInMap = _albumMap.find(_currentAlbum);
        if (itFoundInMap != _albumMap.end()) {
            const DirectoryList& mp3Files = itFoundInMap->second;
            for (auto it = mp3Files.begin(); it != mp3Files.end(); ++it) {
                path file = *it;
                if (!is_directory(file) && file.extension() == ".mp3") {
                    n++;
                    if (file == currentTitlePosition.getTitle()) {
                        break;
                    }
                }
            }
        }
    }
    return n;
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
void PlaybackController::startFastPlay (int factor) {
    if (_fastPlayFactor != factor) {
        _fastPlayFactor = factor;
        _fastPlayFactorUpdateTime = microsec_clock::local_time();
        _fastForwardWaitsForLoadCompleted = false;
        _fastBackwardsWaitsForLoadCompleted = false;
        _fastBackwardsWaitsForJumpCompleted = false;
        _numberOfFastPlayedTitles = 0;
        _numbersToSay = queue<int>();
        // Probably last frame has not been stored, therefore store for sure.
        setCurrentTitlePosition(_frameCountPlayed);
    }
}
void PlaybackController::stopFastPlay () {
    startFastPlay(0);
}
void PlaybackController::say(int number) {
    if (number <= 100) {
        _numbersToSay.push(number);
    } else {
        int hundred = number / 100 * 100;
        _numbersToSay.push(hundred);
        int rest = number % hundred;
        _numbersToSay.push(rest);
    }
    sayNextNumber();
}
void PlaybackController::sayNextNumber() {
    int nextNumber = _numbersToSay.front();
    _mp3Player.load(_spokenNumberMap[nextNumber]);
}
bool PlaybackController::resume() {
    stopFastPlay();
    _paused = false;
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
    stopFastPlay();
    if (_paused) {
        resume();
    } else {
        _paused = true;
        say (getCurrentTitleNumber());
    }
}
bool PlaybackController::next (bool wrapAround) {
    stopFastPlay();
    optional<path> nextTitle = getNextTitle(1, wrapAround);
    if (nextTitle) {
        setCurrentTitlePosition (TitlePosition (nextTitle.get(), 0));
        _mp3Player.load (nextTitle.get());
        return true;
    }
    return false;
}
bool PlaybackController::back() {
    stopFastPlay();
    if (_secondsPlayed > 30.0) {
        _mp3Player.jumpToBegin();
        return true;
    } else {
        optional<path> previousTitle = getPreviousTitle(1);
        if (previousTitle) {
            setCurrentTitlePosition (TitlePosition (previousTitle.get(), 0));
            _mp3Player.load (previousTitle.get());
        } else {
            _mp3Player.jumpToBegin();
        }
        return true;
    }
    return false;
}
void PlaybackController::fastForward () {
    if (isLastTitle() && _frameCountPlayed + 1 >= _frameCountTotal) {
        optional<path> firstTitle = getFirstTitle();
        if (firstTitle) {
            TitlePosition firstTitleTP = TitlePosition(firstTitle.get(), 0);
            setCurrentTitlePosition (firstTitleTP);
            _mp3Player.load (firstTitle.get());
            _fastForwardWaitsForLoadCompleted = true;
        }
    }
    startFastPlay(2);
}
void PlaybackController::fastBackwards () {
    startFastPlay(-2);
    if (isLastTitle() && _frameCountPlayed + 1 >= _frameCountTotal) {
        path currentTitle = _currentTitlePosition.get().getTitle();
        TitlePosition currentTitlePosition = TitlePosition(currentTitle, 0);
        setCurrentTitlePosition (currentTitlePosition);
        _mp3Player.load (currentTitle);
        _fastBackwardsWaitsForLoadCompleted = true;
        return;
    }
}
void PlaybackController::jumpToAlbum (int n) {
    stopFastPlay();
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
    stopFastPlay();
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
    stopFastPlay();
    _presentingAlbums = false;
    resume();
}
void PlaybackController::mpg123Version (const string& message) {
}
void PlaybackController::titleLoaded (const Mp3Title& title) {
    if (_numbersToSay.empty()) {
        cout << "Playing title " << title.toString() << endl;
        _frameCountOfLastUpdateCycle = 0;
    }
}
void PlaybackController::playStatus (int framecount, int framesLeft,
        float seconds, float secondsLeft) {
    _frameCountPlayed = framecount;
    _frameCountTotal = framecount + framesLeft;
    _secondsPlayed = seconds;
    if (framecount - _frameCountOfLastUpdateCycle > _titlePositionUpdateCycle) {
        _frameCountOfLastUpdateCycle = framecount;
        setCurrentTitlePosition(framecount);
    }
    if (_fastPlayFactor == 0) {
        return;
    }
    ptime tNow = microsec_clock::local_time();
    time_duration timeSinceLastFactorUpdate = tNow - _fastPlayFactorUpdateTime;
    if (timeSinceLastFactorUpdate > FPFI_DURATION && _fastPlayFactor < 8192) {
        _fastPlayFactor <<= 1;
        _fastPlayFactorUpdateTime = tNow;
    }
    float secondsTotal = seconds + secondsLeft;
    int framesPerSecond = _frameCountTotal /
        (static_cast<int>(secondsTotal) + 1);
    int titleStepSize = 1;
    if (_numberOfFastPlayedTitles > 10 && _numberOfFastPlayedTitles <= 100) {
        titleStepSize = 10;
    } else if (_numberOfFastPlayedTitles > 100) {
        titleStepSize = 100;
    }
    if (!_numbersToSay.empty()) {
        return;
    } else if (_fastForwardWaitsForLoadCompleted) {
        if (_mp3Player.isLoadCompleted()) {
            _fastForwardWaitsForLoadCompleted = false;
            _numberOfFastPlayedTitles+=titleStepSize;
        } else {
            return;
        }
    } else if (_fastBackwardsWaitsForLoadCompleted) {
        if (_mp3Player.isLoadCompleted()) {
            _mp3Player.jumpTo(_frameCountTotal - framesPerSecond);
            _fastBackwardsWaitsForLoadCompleted = false;
            _fastBackwardsWaitsForJumpCompleted = true;
            _numberOfFastPlayedTitles+=titleStepSize;
        }
        return;
    } else  if (_fastBackwardsWaitsForJumpCompleted) {
        if (!_mp3Player.isJumpToCompleted()) {
            return;
        } else {
            _fastBackwardsWaitsForJumpCompleted = false;
        }
    }
    int framesPerEighthOfSecond = framesPerSecond >> 3;
    int frameJump = framesPerEighthOfSecond * _fastPlayFactor;
    int nextFrameCount = framecount + frameJump;
    if (nextFrameCount > _frameCountTotal) {
        optional<path> nextTitle = getNextTitle(titleStepSize,
                                                false /* no wrap-around. */);
        if (nextTitle) {
            TitlePosition nextTP = TitlePosition(nextTitle.get(), 0);
            setCurrentTitlePosition (nextTP);
            say (getCurrentTitleNumber());
            return;
        } else {
            nextFrameCount = _frameCountTotal - framesPerSecond;
        }
    } else if (nextFrameCount < 0) {
        optional<path> previousTitle = getPreviousTitle(titleStepSize);
        if (previousTitle) {
            TitlePosition previousTP = TitlePosition(previousTitle.get(), 0);
            setCurrentTitlePosition (previousTP);
            say (getCurrentTitleNumber());
            return;
        } else {
            nextFrameCount = framesPerSecond;
        }
    }
    _mp3Player.jumpTo(nextFrameCount);
}
void PlaybackController::playingStopped (bool endOfSongReached) {
    if (!_numbersToSay.empty()) {
        _numbersToSay.pop();
    }
    if (!_numbersToSay.empty()) {
        sayNextNumber();
    } else if (!_paused && !_presentingAlbums) {
        if (_fastPlayFactor == 0) {
            next(false /* no wrap-around */);
        } else {
            TitlePosition currentTitlePosition = _currentTitlePosition.get();
            _mp3Player.load(currentTitlePosition.getTitle());
            if (_fastPlayFactor > 0) {
                _fastForwardWaitsForLoadCompleted = true;
            } else if (_fastPlayFactor < 0) {
                _fastBackwardsWaitsForLoadCompleted = true;
            }
        }
    }
}
void PlaybackController::playingPaused() {
}
void PlaybackController::playingUnpaused() {
}
void PlaybackController::playingErrorOccurred (
        const string& errorMessage) {
}
void PlaybackController::mpg123CommunicationProblem (
        const error_code& error) {
}
void PlaybackController::mpg123Terminated (
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
