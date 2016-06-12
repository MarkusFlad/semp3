/* 
 * File:   Frontend.hpp
 * Author: markus
 *
 * Created on 31. Dezember 2015, 15:31
 */

#ifndef FRONTEND_HPP
#define	FRONTEND_HPP

#include <thread>
#include <exception>
#include <memory>

class OneButtonPlaybackController;

/**
 * Class that encapsulates the front-end which operates the playback controller.
 * On the raspberry pi this is the single button. On desktop systems this is
 * the emulated single button (key 'p' for press down button and any other key
 * for release the button.
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
        OneButtonPlaybackController& playbackController);
    /**
     * Destructor.
     */
    ~Frontend();
protected:
    Frontend (OneButtonPlaybackController& playbackController);
    static void pollKeyboard();
    static void onButtonPressed();
    static void onButtonReleased();
private:
    static std::shared_ptr<Frontend> _instance;
    OneButtonPlaybackController& _playbackController;
    std::thread _keyboardPollingThread;
};

#endif	/* FRONTEND_HPP */

