#include "RotarySwitch.hpp"
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

RotarySwitch::RotarySwitch (const time_duration& samplingCycle,
                            io_service& ioService)
: _samplingTimer(ioService)
, _samplingCycle (samplingCycle)
, _positionUpdateTime(microsec_clock::local_time())
, _currentPosition (Position(1))
, _position (Position(1)) {
}
void RotarySwitch::setCurrentPosition (Position currentPosition) {
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
        _samplingTimer.async_wait(bind(&RotarySwitch::handleSampling, this, error));
    } else {
        _position = _currentPosition;
        for (IListener* listener : _listeners) {
            listener->positionChanged(currentPosition);
        }
        _positionUpdateTime = tNow;
    }
}
RotarySwitch::Position RotarySwitch::getPosition() const {
    lock_guard<mutex> lock (_mutex);
    return _position;
}
void RotarySwitch::addListener (IListener* listener) {
    _listeners.push_back(listener);
}
void RotarySwitch::handleSampling (const boost::system::error_code& error) {
    if (!error) {
        setCurrentPosition (_currentPosition);
    } else {
        cerr << "Error in RotarySwitch::handleSampling() - " << error << endl;
    }
}
