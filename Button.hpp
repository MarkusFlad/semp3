#ifndef BUTTON_HPP
#define	BUTTON_HPP

#include <mutex>
#include <vector>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/system/error_code.hpp>

/**
 * Class that allows using asynchronous button events in the boost asio
 * io_service. Also handles chatter issues.
 */
class Button {
public:
    /**
     * Position of the button.
     */
    enum class Position {
        RELEASED,
        PRESSED
    };
    typedef boost::posix_time::time_duration TimeDuration;
    /**
     * Interface that has to be implemented by listeners on button events.
     * The listeners are called in the context of the boost asio io_service.
     */
    class IListener {
    public:
        /**
         * Virtual destructor.
         */
        virtual ~IListener() {}
        /**
         * Called when the button has been pressed down.
         * @param button The button that is pressed down.
         */
        virtual void buttonPressed (const Button& button) = 0;
        /**
         * Called when the button is released.
         * @param button The button that is released.
         * @param pressDuration The time duration how long the button had been
         *                      pressed down.
         */
        virtual void buttonReleased (const Button& button,
            const TimeDuration& pressDuration) = 0;
    };
    /**
     * Constructor.
     * @param samplingCycle The time cycle in which the button position is
     *                      checked and the listeners are called.
     * @param ioService The boost asio io_service used as the context for
     *                  calling the listeners.
     */
    Button (const TimeDuration& samplingCycle,
            boost::asio::io_service& ioService);
    /**
     * Set the current button position. Thread safe method. Note that calling
     * this method does not call the button listeners directly. Instead the
     * button listeners are called after samplingCycle has elapsed. This means
     * calling this method more often than the samplingCycle is ignored and only
     * the last call is considered. This eliminates chatter issues.
     * @param currentPosition The current (physical) button position.
     */
    void setCurrentPosition (Position currentPosition);
    /*
     * Get the last position that has been updated within the samplingCycle.
     * @return The position of the button.
     */
    Position getPosition() const;
    /**
     * Add a button listener that shall be called in the context of the
     * given boost asio io_service when the the button position changes.
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

#endif	/* BUTTON_HPP */

