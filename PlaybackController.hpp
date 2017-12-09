#ifndef PLAYBACK_CONTROLLER_HPP
#define	PLAYBACK_CONTROLLER_HPP

#include "Mp3Player.hpp"
#include "RebootSafeString.hpp"
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>
#include <boost/optional.hpp>
#include <boost/filesystem/path.hpp>
#include <vector>
#include <map>

namespace boost {
    namespace system {
        class error_code;
    }
}

/**
 * Class that controls the play back of the titles from different albums.
 * Uses an Mp3Player to play the titles.
 */
class PlaybackController : public virtual Mp3Player::IListener {
public:
    typedef boost::filesystem::path Path;
    /**
     * Constructor attaches to the given Mp3Player. Starts the play back of
     * the first title in the first album found in the given albums directory.
     * If there is a current album file the first title in this album directory
     * is played. If in this album directory a current title file is found this
     * title is played.
     * After a title has been played the playback controller automatically
     * starts playing the next title. If the end is reached the automatic
     * playback stops.
     * @param albumsPath The directory containing one sub-directory for each
     *                   album. The album directories contain the mp3 files.
     * @param mp3Player The Mp3Player instance that is controlled to play
     *                  the titles.
     */
    PlaybackController (const Path& albumsPath, Mp3Player& mp3Player);
    /**
     * Start the playback of the current album title and frame if at least one
     * valid MP3 album has been found in the albums path that has been given to
     * the playback controller in the constructor.
     * @return True if there is at least one directory in the albums path where
     *         at least one .mp3 file has been found. Else if there is no
     *         MP3 album at the MP3 albums path given in the constructor. If
     *         true is returned the MP3 player starts playing a title, if false
     *         no MP3 title is played. In case false is returned the
     *         methods next and back have no effect. */
    bool resume();
    /**
     * If a title is currently played the play back is stopped. If the
     * play back is already stopped the title is continued.
     */
    void pause();
    /**
     * Play the next title.
     * @param wrapAround If true the first title will be played when the current
     *                   title is the last title. If false the playback stops
     *                   at the last title.
     * @return True if the command could be performed. False if not (e.g because
     *         resume() has not been called at least one time or if there is
     *         no next title).
     */
    bool next (bool wrapAround);
    /**
     * If 30 seconds of the title have been already played the play back of
     * this title is started at the beginning. If less than 30 seconds have
     * been played the play back of the previous title is started (in case of
     * the first title of the album just the first title is restarted).
     * @return True if the command could be performed. False if not (
     *         e.g because resume() has not been called at least one time).
     */
    bool back();
    /**
     * Start the fast-play action in forward direction and return to the caller.
     * The fast-play action is executed as long as either the last frame of the
     * last album is reached or any other public method is called.
     * From the current position play fragments of the following sound faster
     * than the normal player. The speed is not always the same but is
     * increased. It starts with factor 2 for 3 seconds, continues with
     * factor 4 for the next 3 seconds, continues with factor 8 for the next
     * 3 seconds, etc. until factor 8192 is reached.
     * Note that if the fast-play action has already been started the factor
     * is set back to 2 if this method is called.
     */
    void fastForward ();
    /**
     * Start the fast-play action in backward direction and return to the
     * caller.
     * The fast-play action is executed as long as either the first frame of the
     * first album is reached or any other public method is called.
     * From the current position play fragments of the following sound but
     * play fragments in previous positions of the title. The speed is not
     * always the same but is increased. It starts with factor 2 for 3 seconds,
     * continues with factor 4 for the next 3 seconds, continues with
     * factor 8 for the next 3 seconds, etc. until factor 8192 is reached.
     * Note that if the fast-play action has already been started the factor
     * is set back to 2 if this method is called.
     */
    void fastBackwards ();
    /**
     * Stop playing the current album and resume playing album number n.
     * This means the album n is started at the same position where it has been
     * stopped the last time is was playing (considering title and frame).
     * If there is no album at the given position the album before or after the
     * not existing album number are stared. In the worst case the playback of
     * the current album just continues.
     * @param n The number of the album to be played relative to the first
     *          album.
     */
    void jumpToAlbum (int n);
    /**
     * Play the first title of the next album. Select the next album to be the
     * current album.
     */
    void presentNextAlbum();
    /**
     * Play the stored current title of the selected album (see
     * presentNextAlbum) or the first title of the selected album if there
     * is not current title file.
     */
    void resumeAlbum();
    /**
     * @see Mp3Player#IListener#mpg123Version
     */
    void mpg123Version (const std::string& message) override;
    /**
     * @see Mp3Player#IListener#titleLoaded
     */
    void titleLoaded (const Mp3Title& title) override;
    /**
     * @see Mp3Player#IListener#playStatus
     */
    void playStatus (int framecount, int framesLeft, float seconds,
            float secondsLeft) override;
    /**
     * @see Mp3Player#IListener#playingStopped
     */
    void playingStopped (bool endOfSongReached) override;
    /**
     * @see Mp3Player#IListener#playingPaused
     */
    void playingPaused() override;
    /**
     * @see Mp3Player#IListener#playingUnpaused
     */
    void playingUnpaused() override;
    /**
     * @see Mp3Player#IListener#playingErrorOccurred
     */
    void playingErrorOccurred (const std::string& errorMessage) override;
    /**
     * @see Mp3Player#IListener#mpg123CommunicationProblem
     */
    void mpg123CommunicationProblem (
            const boost::system::error_code& error) override;
    /**
     * @see Mp3Player#IListener#mpg123Terminated
     */
    void mpg123Terminated (int waitpidStatus) override;

protected:
    /**
     * Typedef DirectoryList for better readability of vector<path>.
     */
    typedef std::vector<Path> DirectoryList;
    /**
     * Listener for the Mp3Player. Note the its methods are called in the
     * Mp3Player's io_service.
     */
    class Mp3PlayerListener : public virtual Mp3Player::IListener {
    public:
        /**
         * Constructor.
         * @param playbackController The playback controller that is the context
         *                           of this listener.
         * @param titlePositionUpdateCycle The minimum number of frames that
         *                                 have to be played since the last time
         *                                 to update the current title position.
         *                                 Note that the title position is
         *                                 stored in a file and therefore its
         *                                 a relatively expensive operation.
         */
        Mp3PlayerListener (PlaybackController& playbackController,
                           int titlePositionUpdateCycle);
    private:
        PlaybackController& _playbackController;
    };
    /**
     * Class that represents a title and the position within this title.
     * The position within the title is specified with a frame-count.
     */
    class TitlePosition {
    public:
        /**
         * Constructor.
         * @param title The title represented by this object.
         * @param frameCount The frame-count that represents the position
         *                   within the given title.
         */
        TitlePosition (const Path& title, int frameCount);
        /**
         * Constructor with move semantics for the tile but which sets a new
         * position within the title.
         * @param titlePosition The title-position to be moved.
         * @param frameCount The frame-count that represents the new position
         *                   within the given title.
         */
        TitlePosition (TitlePosition&& titlePosition, int frameCount);
        /**
         * Get the title represented by this object.
         * @return The path to the represented title.
         */
        Path getTitle() const;
        /**
         * Get the position within the title represented by this object.
         * @return The frame-count that specifies the position within the title.
         */
        int getFrameCount() const;
    private:
        Path _title;
        int _frameCount;
    };
    /**
     * Set the given title position to be the title currently played and
     * remember this permanently in the current title file.
     * @param titlePosition The new title position being played.
     */
    void setCurrentTitlePosition (const TitlePosition& titlePosition);
    /**
     * Set the given title position to be the title currently played and
     * remember this permanently in the current title file. Only the position
     * within the current title is changed in the title file.
     * @param frameCount The frame-count the represents the current position
     *                   within the currently played title.
     */
    void setCurrentTitlePosition (int frameCount);
    /**
     * Helper method to get the path of the current mp3 title and the position
     * within this title for the given album directory.
     * @param album The path to the album directory.
     * @return The path of the current mp3 title file. None if there is not
     *         album at the given path.
     */
    boost::optional<TitlePosition> getCurrentTitlePosition (const Path& album);
    /**
     * Update the content of the current title file with the current title
     * position.
     */
    void updateCurrentTitleFile ();
    /**
     * Get the title first title of the current album.  If there is no title at
     * all return none. 
     * @return The first title or none.
     */
    boost::optional<Path> getFirstTitle () const;
    /**
     * Get the title after the current title in the current album. If it is
     * the last title in the album and wrapAround is false or if there is no
     * title at all return none. 
     * @param wrapAround The first title returned if the current title
     *                   is already the last title.
     * @return The next title or none if there is no next title.
     */
    boost::optional<Path> getNextTitle (bool wrapAround) const;
    /**
     * Get the title before the current title in the current album. If it is
     * the first title in the album or if there is no title at all return none. 
     * @return The previous title or none if there is no previous title.
     */
    boost::optional<Path> getPreviousTitle () const;
    /**
     * Get if the current title is the last title in the current album.
     * @return True if it is the last title, false if not.
     */
    bool isLastTitle () const;
    /**
     * Recursively go through the given albums path and return all valid
     * album directories mapped to the list of their mp3 files. A valid album
     * directory contains at least one mp3 file.
     * @param albums The path to look for valid mp3 files or album
     *               sub-directories. This means the albums path itself may
     *               also be an album path.
     * @return List of valid album directories.
     */
    static std::map<Path, DirectoryList> getAlbumMap(const Path& albums);
    /**
     * Start the fast-play action with the given factor of how much the title
     * is played faster than normal.
     * From the current position play fragments of the following sound faster
     * than the normal player. The speed is not always the same but is
     * increased. It starts with the given factor for 3 seconds, continues with
     * the doubled factor 4 for the next 3 seconds, continues with the doubled
     * factor 3 seconds, etc. while the factor is below 8192.
     * @param The factor to be used by the fast-play action. Positive to play
     *        forward, negative number to play backwards. If 0 no fast-play
     *        action is performed.
     */
    void startFastPlay (int factor);
    /**
     * Stop the fast-play action. This is just for convenience and in principle
     * the same as calling startFastPlay(0).
     */
    void stopFastPlay ();
private:
    const Path& _albumsPath;
    Mp3Player& _mp3Player;
    DirectoryList _albums;
    std::map<Path, DirectoryList> _albumMap;
    Path _currentAlbum;
    boost::optional<TitlePosition> _currentTitlePosition;
    int _titlePositionUpdateCycle;
    int _frameCountOfLastUpdateCycle;
    int _frameCountPlayed;
    int _frameCountTotal;
    float _secondsPlayed;
    int _fastPlayFactor;
    bool _fastForwardWaitsForLoadCompleted;
    bool _fastBackwardsWaitsForLoadCompleted;
    bool _fastBackwardsWaitsForJumpCompleted;
    boost::posix_time::ptime _fastPlayFactorUpdateTime;
    bool _presentingAlbums;
    RebootSafeString _currentAlbumInfo;
    RebootSafeString _currentTitleInfo;
    static const std::string CURRENT_ALBUM_FILENAME;
    static const std::string CURRENT_TITLE_FILENAME;
    static const boost::posix_time::time_duration FPFI_DURATION;
};

#endif	/* PLAYBACK_CONTROLLER_HPP */
