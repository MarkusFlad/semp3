#include "Button.hpp"
#include <boost/bind.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/date_time/posix_time/posix_time_types.hpp>
#include <boost/date_time/posix_time/posix_time_config.hpp>
#include <iostream>

using std::cerr;
using std::endl;
using std::mutex;
using std::lock_guard;
using boost::asio::io_service;
using boost::bind;
using boost::asio::placeholders::error;
using boost::asio::deadline_timer;
using boost::posix_time::ptime;
using boost::posix_time::microsec_clock;
using boost::posix_time::milliseconds;
using boost::posix_time::time_duration;

Button::Button (const time_duration& samplingCycle,
        const TimeDuration& buttonPressedCheckCycle,
        io_service& ioService)
: _samplingTimer(ioService)
, _samplingCycle (samplingCycle)
, _positionUpdateTime(microsec_clock::local_time())
, _currentPosition (Position::RELEASED)
, _position (Position::RELEASED)
, _buttonPressedCheckTimer (ioService)
, _checkButtonPressedCycle (buttonPressedCheckCycle)
, _buttonPressedDuration (milliseconds(0)) {
}
void Button::setCurrentPosition (Position currentPosition) {
    lock_guard<mutex> lock (_mutex);
    if (currentPosition == _currentPosition) {
        return;
    }
    _currentPosition = currentPosition;
    ptime tNow = microsec_clock::local_time();
    time_duration timeSinceLastCall = tNow - _positionUpdateTime;
    if (timeSinceLastCall < _samplingCycle) {
        time_duration timeToWait = _samplingCycle - timeSinceLastCall;
        _samplingTimer.expires_from_now (timeToWait);
        _samplingTimer.async_wait(bind(&Button::handleSampling, this, error));
    } else {
        _position = _currentPosition;
        if (_position == Position::PRESSED) {
            for (IListener* listener : _listeners) {
                listener->buttonPressed (*this);
            }
            _buttonPressedDuration = milliseconds(0);
            _buttonPressedCheckTimer.expires_from_now(_checkButtonPressedCycle);
            _buttonPressedCheckTimer.async_wait(
                    bind(&Button::handleButtonPressed, this, error));
        } else {
            time_duration pressTime = tNow - _positionUpdateTime;
            for (IListener* listener : _listeners) {
                listener->buttonReleased (*this, pressTime);
            }
        }
        _positionUpdateTime = tNow;
    }
}
Button::Position Button::getPosition() const {
    lock_guard<mutex> lock (_mutex);
    return _position;
}
void Button::addListener (IListener* listener) {
    _listeners.push_back(listener);
}
void Button::handleSampling (const boost::system::error_code& error) {
    if (!error) {
        setCurrentPosition (_currentPosition);
    } else {
        cerr << "Error in Button::handleSampling() - " << error << endl;
    }
}
void Button::handleButtonPressed (const boost::system::error_code& error) {
    lock_guard<mutex> lock (_mutex);
    if (!error) {
        if (Position::PRESSED == _currentPosition) {
            _buttonPressedDuration += _checkButtonPressedCycle;
            for (IListener* listener : _listeners) {
                listener->buttonStillPressed(*this, _buttonPressedDuration);
            }
            _buttonPressedCheckTimer.expires_from_now(_checkButtonPressedCycle);
            _buttonPressedCheckTimer.async_wait(
                    bind(&Button::handleButtonPressed, this, error));
        }
    } else {
        cerr << "Error in Button::handleSampling() - " << error << endl;
    }
}
