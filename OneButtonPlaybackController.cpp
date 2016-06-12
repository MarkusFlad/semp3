#include "OneButtonPlaybackController.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>

using boost::asio::io_service;
using boost::filesystem::path;
using boost::posix_time::time_duration;
using boost::posix_time::milliseconds;

//==============================================================================
//-------------------------- PlaybackController --------------------------------
//==============================================================================
OneButtonPlaybackController::OneButtonPlaybackController (
        const path& albumsPath, Mp3Player& mp3Player, io_service& ioService,
        const time_duration& longPressDuration,
        const time_duration& veryLongPressDuration)
: _playbackController (albumsPath, mp3Player)
, _button (milliseconds(10), ioService)
, _buttonListener (*this)
, _longPressDuration (longPressDuration)
, _veryLongPressDuration (veryLongPressDuration) {
    _button.addListener(&_buttonListener);
}
void OneButtonPlaybackController::setCurrentButtonPosition (
    Button::Position currentPosition) {
    _button.setCurrentPosition(currentPosition);
}
//==============================================================================
//------------------ PlaybackController::ButtonListener-------------------------
//==============================================================================
OneButtonPlaybackController::ButtonListener::ButtonListener (
        OneButtonPlaybackController& playbackController)
: _obc (playbackController)
, _albumSelection (false) {
}
void OneButtonPlaybackController::ButtonListener::buttonPressed(
        const Button& button) {
}
void OneButtonPlaybackController::ButtonListener::buttonReleased(
        const Button& button, const time_duration& pressDuration) {
    if (pressDuration > _obc._veryLongPressDuration) {
        _albumSelection = true;
        _obc._playbackController.pause();
    } else if (_albumSelection) {
        if (pressDuration < _obc._longPressDuration) {
            _obc._playbackController.presentNextAlbum();
        } else {
            _albumSelection = false;
            _obc._playbackController.resumeAlbum();
        }
    } else {
        if (pressDuration < _obc._longPressDuration) {
            _obc._playbackController.pause();
        } else {
            _obc._playbackController.back();
        }
    }
}
