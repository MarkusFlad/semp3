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
using std::endl;
using std::shared_ptr;

std::shared_ptr<Frontend> Frontend::_instance;
RotarySwitch::Position Frontend::_currentRotarySwitchPosition(1);
#ifndef USE_WIRING_PI
bool Frontend::_keyboardButton1Pressed = false;
bool Frontend::_keyboardButton2Pressed = false;
#endif    

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
    const std::array<GpioPin,12> ROTARY_SWITCH_GPIO_PINS = {
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
        wiringPiISR (pin, INT_EDGE_RISING,&Frontend::onRotarySwitchPosition);
    }
    int pin0 = getWiringPiPin(GpioPin::GPIO_0);
    pullUpDnControl (pin0, PUD_DOWN);
    wiringPiISR (pin0, INT_EDGE_BOTH,
            &Frontend::onButton1);
    int pin1 = getWiringPiPin(GpioPin::GPIO_1);
    pullUpDnControl (pin1, PUD_DOWN);
    wiringPiISR (pin1, INT_EDGE_BOTH,
            &Frontend::onButton2);
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
                if (_keyboardButton1Pressed) {
                    _keyboardButton1Pressed = false;
                } else {
                    _keyboardButton1Pressed = true;
                }
                onButton1();
                break;
            case 'k':
                if (_keyboardButton2Pressed) {
                    _keyboardButton2Pressed = false;
                } else {
                    _keyboardButton2Pressed = true;
                }
                onButton2();
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
void Frontend::onButton1() {
#ifndef USE_WIRING_PI
    if (_keyboardButton1Pressed) {
#else
    if (digitalRead(getWiringPiPin (GpioPin::GPIO_0)) == HIGH) {
#endif
        _instance->_playbackController.setCurrentButton1Position(
            Button::Position::PRESSED);
    } else {
        _instance->_playbackController.setCurrentButton1Position(
            Button::Position::RELEASED);
    }
}
void Frontend::onButton2() {
#ifndef USE_WIRING_PI
    if (_keyboardButton2Pressed) {
#else
    if (digitalRead(getWiringPiPin (GpioPin::GPIO_1)) == HIGH) {
#endif
        _instance->_playbackController.setCurrentButton2Position(
            Button::Position::PRESSED);
    } else {
        _instance->_playbackController.setCurrentButton2Position(
            Button::Position::RELEASED);
    }
}
void Frontend::onRotarySwitchPosition() {
#ifdef USE_WIRING_PI
    int numberOfPositions = 0;
    if (digitalRead(getWiringPiPin (GpioPin::GPIO_2)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(1);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_3)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(2);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_4)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(3);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_5)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(4);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_6)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(5);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_7)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(6);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_21)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(7);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_22)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(8);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_23)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(9);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_24)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(10);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_25)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(11);
        numberOfPositions++;
    }
    if (digitalRead (getWiringPiPin (GpioPin::GPIO_26)) == HIGH) {
        _currentRotarySwitchPosition = RotarySwitch::Position(12);
        numberOfPositions++;
    }
    if (numberOfPositions > 1) {
        cerr << "WiringPi interrupt. Several GPIOs of the rotary switch are set." << endl;
    }
#endif
    _instance->_playbackController.setCurrentRotarySwitchPosition(
        _currentRotarySwitchPosition);
}
