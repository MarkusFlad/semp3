/* 
 * File:   Frontend.hpp
 * Author: markus
 *
 * Created on 31. Dezember 2015, 15:31
 */

#ifndef FRONTEND_HPP
#define	FRONTEND_HPP

#include "RotarySwitch.hpp"
#include <thread>
#include <exception>
#include <memory>

class ThreeControlsPlaybackController;

/**
 * Class that encapsulates the front-end which operates the playback controller.
 * On the raspberry pi this are two buttons button and a rotary switch with
 * 12 positions. On desktop systems this is emulated: Button 1 is defined to
 * be pressed down if key 'j' has been pressed, Button s is defined to be
 * pressed down if key 'k' has been pressed. If the same key is pressed again
 * the corresponding button is defined to be not pressed.
 * The switch is simply emulated by the key '1'..'9' for rotary switch position
 * 1 .. 9 and key 'a'..'c' represent switch position 10 .. 12.
 */
class Frontend {
public:
    /**
     * Creates the Frontend singleton if it does not already exist.
     * @param playbackController The playback controller that is operated by
     *                           the Frontend.
     * @return Pointer to the Frontend singelton.
     */
    static std::shared_ptr<Frontend> create(
            ThreeControlsPlaybackController& playbackController);
    /**
     * Destructor.
     */
    ~Frontend();
protected:
    Frontend (ThreeControlsPlaybackController& playbackController);
    static void pollKeyboard();
    static void onButton1Pressed();
    static void onButton1Released();
    static void onButton2Pressed();
    static void onButton2Released();
    static void onRotarySwitchPosition();
private:
    static std::shared_ptr<Frontend> _instance;
    ThreeControlsPlaybackController& _playbackController;
    std::thread _keyboardPollingThread;
    static RotarySwitch::Position _currentRotarySwitchPosition;
};

#endif	/* FRONTEND_HPP */

