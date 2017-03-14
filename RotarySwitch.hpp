#ifndef ROTARY_SWITCH_HPP
#define	ROTARY_SWITCH_HPP

#include <mutex>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/error_code.hpp>

/**
 * Class that allows using asynchronous rotary switch events in the boost asio
 * io_service. Also handles chatter issues.
 */
class RotarySwitch {
public:
    /**
     * Position of the rotary switch.
     */
    class Position {
    public:
        /**
         * Constructor.
         * @param value The position value is an integer in the range 1 .. 12.
         *              If the given number is smaller than 1 the position is
         *              1, if the position is bigger than 12 the position is 12.
         */
        inline Position (int value);
        /**
         * Get the position value.
         * @return The value represented by this position. A value in the range
         *         1 .. 12.
         */
        inline int getValue() const;
        /**
         * Check if the given position is the same than this position.
         * @param position The position to be compared to this one.
         * @return True if the positions are the same, false if not.
         **/
        inline bool operator==(Position position) const;
    private:
        int _value;
    };
    typedef boost::posix_time::time_duration TimeDuration;
    /**
     * Interface that has to be implemented by listeners on rotary switch
     * events.
     * The listeners are called in the context of the boost asio io_service.
     */
    class IListener {
    public:
        /**
         * Virtual destructor.
         */
        virtual ~IListener() {}
        /**
         * Called when the rotary switch position has been changed.
         * @param position The new button position.
         */
        virtual void positionChanged (Position position) = 0;
    };
    /**
     * Constructor.
     * @param samplingCycle The time cycle in which the rotary switch position
     *                      is checked and the listeners are called.
     * @param ioService The boost asio io_service used as the context for
     *                  calling the listeners.
     */
    RotarySwitch (const TimeDuration& samplingCycle,
                  boost::asio::io_service& ioService);
    /**
     * Set the current rotary switch position. Thread safe method. Note that
     * calling this method does not call the button listeners directly. Instead
     * the button listeners are called after samplingCycle has elapsed. This
     * means calling this method more often than the samplingCycle is ignored
     * and only the last call is considered. This eliminates chatter issues.
     * @param currentPosition The current (physical) rotary switch position.
     */
    void setCurrentPosition (Position currentPosition);
    /*
     * Get the last position that has been updated within the samplingCycle.
     * @return The position of the rotary switch.
     */
    Position getPosition() const;
    /**
     * Add a rotary switch listener that shall be called in the context of the
     * given boost asio io_service when the the rotary switch position changes.
     * @param listener
     */
    void addListener (IListener* listener);

protected:
    void handleSampling (const boost::system::error_code& error);
    
private:
    boost::asio::deadline_timer _samplingTimer;
    TimeDuration _samplingCycle;
    boost::posix_time::ptime _positionUpdateTime;
    Position _currentPosition;
    Position _position;
    std::vector<IListener*> _listeners;
    mutable std::mutex _mutex;
};

//==============================================================================
//------------------------ RotarySwitch::Position ------------------------------
//==============================================================================
RotarySwitch::Position::Position (int value)
: _value (1) {
    if (value > 12) {
        _value = 12;
    } if (value > 1) {
        _value = value;
    }
}
int RotarySwitch::Position::getValue() const {
    return _value;
}
bool RotarySwitch::Position::operator==(Position position) const {
    return _value == position._value;
}

#endif	/* ROTARY_SWITCH_HPP */

