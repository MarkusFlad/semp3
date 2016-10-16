/* 
 * File:   Frontend.cpp
 * Author: markus
 * 
 * Created on 31. Dezember 2015, 15:31
 */

#include "Frontend.hpp"
#include "ThreeControlsPlaybackController.hpp"
#include "RotarySwitch.hpp"
#ifdef USE_WIRING_PI
#include <wiringPi.h>
#endif
#include <exception>
#include <iostream>

using std::cerr;
using std::endl;
using std::shared_ptr;

std::shared_ptr<Frontend> Frontend::_instance;

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
    wiringPiSetup();
    wiringPiISR (0, INT_EDGE_RISING, &Frontend::onButtonPressed);
    wiringPiISR (0, INT_EDGE_FALLING, &Frontend::onButtonReleased);
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
                onRotarySwtichPosition(RotarySwitch::Position(c - static_cast<int>('1') + 1));
                break;
            case 'a':
            case 'b':
            case 'c':
                onRotarySwtichPosition(RotarySwitch::Position(c - static_cast<int>('a') + 1));
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
void Frontend::onRotarySwtichPosition(RotarySwitch::Position position) {
    _instance->_playbackController.setCurrentRotarySwitchPosition(position);
}
