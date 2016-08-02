#ifndef ONE_BUTTON_PLAYBACK_CONTROLLER_HPP
#define	ONE_BUTTON_PLAYBACK_CONTROLLER_HPP

#include "PlaybackController.hpp"
#include "Button.hpp"
#include <boost/date_time/posix_time/posix_time_duration.hpp>

namespace boost {
    namespace asio {
        class io_service;
    }
}
/**
 * Class that controls the play back of the titles from different albums.
 * Uses an Mp3Player to play the titles. Can be externally controlled by just
 * one button.
 * Pressing the button a short time pauses and continues the play back. Pressing
 * the button a long time either restarts the current title or goes one title
 * back. This depends on how long the title has been played - if the title
 * had been played for less than 30 seconds the play back of the previous
 * title starts. Else the play back of the current title is restarted.
 * When the button is pressed a very long time it is possible to select the
 * album. In this mode pressing the button a short time means switching between
 * the albums. When the album has been found the album can be selected by
 * pressing the button a long time.
 */
class OneButtonPlaybackController {
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
     * @param longPressDuration When the button is pressed longer than this
     *                          duration and shorter than the given
     *                          veryLongPress Duration the action for a long
     *                          button press is performed.
     * @param veryLongPressDuration When the button is pressed longer than this
     *                              duration the action for a very long press is
     *                              performed.
     */
    OneButtonPlaybackController (const Path& albumsPath, Mp3Player& mp3Player,
            boost::asio::io_service& ioService,
            const TimeDuration& longPressDuration = Seconds (1),
            const TimeDuration& veryLongPressDuration = Seconds (10));
    /**
     * Play the current album, title and frame if the given albums-path is
     * valid.
     * @see PlaybackController#play
     */
    bool resume();
    /**
     * Tell the play back controller the current position of the button.
     * Note that an anti-chatter mechanism is implemented. If the method is
     * called with a high frequency only the last call is used for controlling
     * the play back.
     * This method is thread safe.
     * @param currentPosition The current physical position of the button.
     */
    void setCurrentButtonPosition (Button::Position currentPosition);

private:
    /**
     * Listener for the Button. Note that its methods are called in the
     * Mp3Player's io_service;
     */
    class ButtonListener : public virtual Button::IListener {
    public:
        ButtonListener (OneButtonPlaybackController& playbackController);
        void buttonPressed (const Button& button) override;
        void buttonReleased (const Button& button,
            const TimeDuration& pressDuration) override;
    private:
        OneButtonPlaybackController& _obc;
        bool _albumSelection;
    };
private:
    PlaybackController _playbackController;
    Button _button;
    ButtonListener _buttonListener;
    const TimeDuration _longPressDuration;
    const TimeDuration _veryLongPressDuration;
};

#endif	/* ONE_BUTTON_PLAYBACK_CONTROLLER_HPP */
