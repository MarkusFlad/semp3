#ifndef THREE_CONTROLS_PLAYBACK_CONTROLLER_HPP
#define	THREE_CONTROLS_PLAYBACK_CONTROLLER_HPP

#include "PlaybackController.hpp"
#include "Button.hpp"
#include "RotarySwitch.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>

namespace boost {
    namespace asio {
        class io_service;
    }
}
/**
 * Class that controls the play back of the titles from different albums.
 * Uses an Mp3Player to play the titles. Can be externally controlled by two
 * buttons (Button 1 and Button 2) and a rotary switch with 12 positions.
 * Pressing Button 1 or Button 2 a short time pauses and continues the play
 * back.
 * Pressing Button 1 a long time either restarts the current title or goes one
 * title back. This depends on how long the title has been played - if the title
 * had been played for less than 30 seconds the play back of the previous
 * title starts. Else the play back of the current title is restarted.
 * Pressing Button 2 a long time goes to the next title.
 * The rotary switch position selects the current album.
 */
class ThreeControlsPlaybackController {
public:
    typedef boost::filesystem::path Path;
    typedef boost::posix_time::time_duration TimeDuration;
    typedef boost::posix_time::seconds Seconds;
    /**
     * Constructor.
     * @see PlaybackController#PlaybackController
     * @param ioService The io_service that shall be used for asynchronous
     *                  scheduling of actions - especially for perfoming actions
     *                  acording to button positions.
     *                  Has to be the same io_service as used for the mp3Player.
     * @param longPressDuration When a button is pressed longer than this
     *                          duration the action for a long button press is
     *                          performed.
     */
    ThreeControlsPlaybackController (const Path& albumsPath,
            Mp3Player& mp3Player, boost::asio::io_service& ioService,
            const TimeDuration& longPressDuration = Seconds (1));
    /**
     * Play the current album, title and frame if the given albums-path is
     * valid.
     * @see PlaybackController#play
     */
    bool resume();
    /**
     * Tell the play back controller the current position of the Button 1.
     * Note that an anti-chatter mechanism is implemented. If the method is
     * called with a high frequency only the last call is used for controlling
     * the play back.
     * This method is thread safe.
     * @param currentPosition The current physical position of the button.
     */
    void setCurrentButton1Position (Button::Position currentPosition);
    /**
     * Tell the play back controller the current position of the Button 2.
     * Note that an anti-chatter mechanism is implemented. If the method is
     * called with a high frequency only the last call is used for controlling
     * the play back.
     * This method is thread safe.
     * @param currentPosition The current physical position of the button.
     */
    void setCurrentButton2Position (Button::Position currentPosition);
    /**
     * Tell the play back controller the position of the rotary switch.
     * Note that an anti-chatter mechanism is implemented. If the method is
     * called with a high frequency only the last call is used for controlling
     * the play back.
     * This method is thread safe.
     * @param currentPosition The current physical position of the rotary
     *                        switch.
     */
    void setCurrentRotarySwitchPosition (
            RotarySwitch::Position currentPosition);

private:
    /**
     * Listener for the Button 1. Note that its methods are called in the
     * Mp3Player's io_service;
     */
    class Button1Listener : public virtual Button::IListener {
    public:
        Button1Listener (ThreeControlsPlaybackController& playbackController);
        void buttonPressed (const Button& button) override;
        void buttonStillPressed (const Button& button,
                const TimeDuration& duration);
        void buttonReleased (const Button& button,
            const TimeDuration& pressDuration) override;
    private:
        ThreeControlsPlaybackController& _tcpc;
        bool _playsFastBackwards;
    };
    /**
     * Listener for the Button 2. Note that its methods are called in the
     * Mp3Player's io_service;
     */
    class Button2Listener : public virtual Button::IListener {
    public:
        Button2Listener (ThreeControlsPlaybackController& playbackController);
        void buttonPressed (const Button& button) override;
        void buttonStillPressed (const Button& button,
            const TimeDuration& duration);
        void buttonReleased (const Button& button,
            const TimeDuration& pressDuration) override;
    private:
        ThreeControlsPlaybackController& _tcpc;
        bool _playsFastForward;
    };
    /**
     * Listener for the Rotary Switch. Note that its methods are called in the
     * Mp3Player's io_service;
     */
    class RotarySwitchListener : public virtual RotarySwitch::IListener {
    public:
        RotarySwitchListener (ThreeControlsPlaybackController& playbackController);
        void positionChanged (RotarySwitch::Position position) override;
    private:
        ThreeControlsPlaybackController& _tcpc;
    };
private:
    PlaybackController _playbackController;
    Button _button1;
    Button _button2;
    RotarySwitch _rotarySwitch;
    Button1Listener _button1Listener;
    Button2Listener _button2Listener;
    RotarySwitchListener _rotarySwitchListener;
    const TimeDuration _longPressDuration;
};

#endif	/* THREE_CONTROLS_PLAYBACK_CONTROLLER_HPP */
