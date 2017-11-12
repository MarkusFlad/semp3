#include "ThreeControlsPlaybackController.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>

using boost::asio::io_service;
using boost::filesystem::path;
using boost::posix_time::time_duration;
using boost::posix_time::milliseconds;

//==============================================================================
//-------------------------- PlaybackController --------------------------------
//==============================================================================
ThreeControlsPlaybackController::ThreeControlsPlaybackController (
        const path& albumsPath, Mp3Player& mp3Player, io_service& ioService,
        const time_duration& longPressDuration)
: _playbackController (albumsPath, mp3Player)
, _button1 (milliseconds(10), milliseconds(1000), ioService)
, _button2 (milliseconds(10), milliseconds(1000), ioService)
, _rotarySwitch (milliseconds(10), ioService)
, _button1Listener (*this)
, _button2Listener (*this)
, _rotarySwitchListener (*this)
, _longPressDuration (longPressDuration) {
    _button1.addListener(&_button1Listener);
    _button2.addListener(&_button2Listener);
    _rotarySwitch.addListener(&_rotarySwitchListener);
}
bool ThreeControlsPlaybackController::resume() {
    return _playbackController.resume();
}
void ThreeControlsPlaybackController::setCurrentButton1Position (
    Button::Position currentPosition) {
    _button1.setCurrentPosition(currentPosition);
}
void ThreeControlsPlaybackController::setCurrentButton2Position (
    Button::Position currentPosition) {
    _button2.setCurrentPosition(currentPosition);
}
void ThreeControlsPlaybackController::setCurrentRotarySwitchPosition (
        RotarySwitch::Position currentPosition) {
    _rotarySwitch.setCurrentPosition(currentPosition);
}
//==============================================================================
//------------------ PlaybackController::Button1Listener------------------------
//==============================================================================
ThreeControlsPlaybackController::Button1Listener::Button1Listener (
        ThreeControlsPlaybackController& playbackController)
: _tcpc (playbackController)
, _playsFastBackwards (false) {
}
void ThreeControlsPlaybackController::Button1Listener::buttonPressed(
        const Button& button) {
    _playsFastBackwards = false;
}
void ThreeControlsPlaybackController::Button1Listener::buttonStillPressed(
    const Button& button, const time_duration& duration) {
    if (duration > _tcpc._longPressDuration && !_playsFastBackwards) {
        _tcpc._playbackController.fastBackwards();
        _playsFastBackwards = true;
    }
}
void ThreeControlsPlaybackController::Button1Listener::buttonReleased(
        const Button& button, const time_duration& pressDuration) {
    if (pressDuration < _tcpc._longPressDuration) {
        _tcpc._playbackController.pause();
    } else {
        _tcpc._playbackController.resume();
    }
    _playsFastBackwards = false;
}
//==============================================================================
//------------------ PlaybackController::Button2Listener------------------------
//==============================================================================
ThreeControlsPlaybackController::Button2Listener::Button2Listener (
        ThreeControlsPlaybackController& playbackController)
: _tcpc (playbackController)
, _playsFastForward (false) {
}
void ThreeControlsPlaybackController::Button2Listener::buttonPressed(
        const Button& button) {
    _playsFastForward = false;
}
void ThreeControlsPlaybackController::Button2Listener::buttonStillPressed(
    const Button& button, const time_duration& duration) {
    if (duration >= _tcpc._longPressDuration && !_playsFastForward) {
        _tcpc._playbackController.fastForward();
        _playsFastForward = true;
    }
}
void ThreeControlsPlaybackController::Button2Listener::buttonReleased(
        const Button& button, const time_duration& pressDuration) {
    if (pressDuration < _tcpc._longPressDuration) {
        _tcpc._playbackController.pause();
    } else {
        _tcpc._playbackController.resume();
    }
    _playsFastForward = false;
}
//==============================================================================
//---------------- PlaybackController::RotarySwitchListener---------------------
//==============================================================================
ThreeControlsPlaybackController::RotarySwitchListener::RotarySwitchListener (
        ThreeControlsPlaybackController& playbackController)
: _tcpc (playbackController) {
}
void ThreeControlsPlaybackController::RotarySwitchListener::positionChanged (
        RotarySwitch::Position position) {
    _tcpc._playbackController.jumpToAlbum (position.getValue());
}
