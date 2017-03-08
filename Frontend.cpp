/* 
 * File:   Frontend.cpp
 * Author: markus
 * 
 * Created on 31. Dezember 2015, 15:31
 */

#include "Frontend.hpp"
#include "ThreeControlsPlaybackController.hpp"
#include "RotarySwitch.hpp"
#include "WiringPiPin.hpp"
#ifdef USE_WIRING_PI
#include <wiringPi.h>
#endif
#include <exception>
#include <iostream>
#include <array>

using std::cerr;
using std::cout;
using std::endl;
using std::shared_ptr;

std::shared_ptr<Frontend> Frontend::_instance;
RotarySwitch::Position Frontend::_currentRotarySwitchPosition(1);

shared_ptr<Frontend> Frontend::create(
        ThreeControlsPlaybackController& playbackController) {
    if (!_instance) {
        _instance.reset (new Frontend(playbackController));
    }
    return _instance;
}
Frontend::~Frontend() {
#ifndef USE_WIRING_PI
    if (system("stty cooked")) {
        cerr << "stty cooked failed" << endl;
    }
#endif    
    _keyboardPollingThread.join();
}
Frontend::Frontend(ThreeControlsPlaybackController& playbackController)
: _playbackController (playbackController)
, _keyboardPollingThread (&Frontend::pollKeyboard) {
#ifdef USE_WIRING_PI
    const std::array<GpioPin,26> ROTARY_SWITCH_GPIO_PINS = {
        GpioPin::GPIO_2,
        GpioPin::GPIO_3,
        GpioPin::GPIO_4,
        GpioPin::GPIO_5,
        GpioPin::GPIO_6,
        GpioPin::GPIO_7,
        GpioPin::GPIO_21,
        GpioPin::GPIO_22,
        GpioPin::GPIO_23,
        GpioPin::GPIO_24,
        GpioPin::GPIO_25,
        GpioPin::GPIO_26};

    wiringPiSetup();
    for (GpioPin gpioPin : ROTARY_SWITCH_GPIO_PINS) {
        int pin = getWiringPiPin(gpioPin);
        pinMode (pin, INPUT);
        pullUpDnControl (pin, PUD_DOWN);
        wiringPiISR (pin, INT_EDGE_FALLING,&Frontend::onRotarySwitchPosition);
    }
    wiringPiISR (getWiringPiPin (GpioPin::GPIO_0), INT_EDGE_RISING,
            &Frontend::onButton1Pressed);
    wiringPiISR (getWiringPiPin (GpioPin::GPIO_0), INT_EDGE_FALLING,
            &Frontend::onButton1Released);
    wiringPiISR (getWiringPiPin (GpioPin::GPIO_1), INT_EDGE_RISING,
            &Frontend::onButton2Pressed);
    wiringPiISR (getWiringPiPin (GpioPin::GPIO_1), INT_EDGE_FALLING,
            &Frontend::onButton2Released);
#endif
}
void Frontend::pollKeyboard() {
#ifndef USE_WIRING_PI
    bool button1Pressed = false;
    bool button2Pressed = false;
    // Set terminal to raw mode 
    if (system("stty raw")) {
        cerr << "stty raw failed" << endl;
    }
    for (;;) {
        int c = getchar();
        switch (c) {
            case 'j':
                if (button1Pressed) {
                    onButton1Released();
                    button1Pressed = false;
                } else {
                    onButton1Pressed();
                    button1Pressed = true;
                }
                break;
            case 'k':
                if (button2Pressed) {
                    onButton2Released();
                    button2Pressed = false;
                } else {
                    onButton2Pressed();
                    button2Pressed = true;
                }
                break;
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
                _currentRotarySwitchPosition =
                        RotarySwitch::Position(c - static_cast<int>('1') + 1);
                onRotarySwitchPosition();
                break;
            case 'a':
            case 'b':
            case 'c':
                _currentRotarySwitchPosition =
                        RotarySwitch::Position(c - static_cast<int>('a') + 1);
                onRotarySwitchPosition();
                break;
        }
    }
#endif    
}
void Frontend::onButton1Pressed() {
    _instance->_playbackController.setCurrentButton1Position(
        Button::Position::PRESSED);
}
void Frontend::onButton1Released() {
    _instance->_playbackController.setCurrentButton1Position(
        Button::Position::RELEASED);
}
void Frontend::onButton2Pressed() {
    _instance->_playbackController.setCurrentButton2Position(
        Button::Position::PRESSED);
}
void Frontend::onButton2Released() {
    _instance->_playbackController.setCurrentButton2Position(
        Button::Position::RELEASED);
}
void Frontend::onRotarySwitchPosition() {
#ifdef USE_WIRING_PI
    if (digitalRead(getWiringPiPin (GpioPin::GPIO_2)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(1);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_3)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(2);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_4)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(3);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_5)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(4);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_6)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(5);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_7)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(6);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_21)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(7);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_22)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(8);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_23)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(9);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_24)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(10);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_25)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(11);
    } else if (digitalRead (getWiringPiPin (GpioPin::GPIO_26)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(12);
    }
#endif
    cout << "Rotary Switch Position is " << _currentRotarySwitchPosition.getValue() << endl;
    _instance->_playbackController.setCurrentRotarySwitchPosition(
        _currentRotarySwitchPosition);
}
