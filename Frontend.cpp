/* 
 * File:   Frontend.cpp
 * Author: markus
 * 
 * Created on 31. Dezember 2015, 15:31
 */

#include "Frontend.hpp"
#include "OneButtonPlaybackController.hpp"
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
        OneButtonPlaybackController& playbackController) {
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
Frontend::Frontend(OneButtonPlaybackController& playbackController)
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
    // Set terminal to raw mode 
    if (system("stty raw")) {
        cerr << "stty raw failed" << endl;
    }
    for (;;) {
        if (getchar() == 'p') {
            onButtonPressed();
        } else {
            onButtonReleased();
        }
    }
#endif    
}
void Frontend::onButtonPressed() {
    _instance->_playbackController.setCurrentButtonPosition(
        Button::Position::PRESSED);
}

void Frontend::onButtonReleased() {
    _instance->_playbackController.setCurrentButtonPosition(
        Button::Position::RELEASED);
}
