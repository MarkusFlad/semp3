#ifndef PLAYBACK_CONTROLLER_HPP
#define	PLAYBACK_CONTROLLER_HPP

#include "Mp3Player.hpp"
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
class PlaybackController {
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
     * If a title is currently played the play back is stopped. If the
     * play back is already stopped the title is continued.
     */
    void pause();
    /**
     * Play the next title.
     */
    void next();
    /**
     * If 30 seconds of the title have been already played the play back of
     * this title is started at the beginning. If less than 30 seconds have
     * been played the play back of the previous title is started (in case of
     * the first title of the album just the first title is restarted).
     */
    void back();
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
        void mpg123Version (const std::string& message) override;
        void titleLoaded (const Mp3Title& title) override;
        void playStatus (int framecount, int framesLeft, float seconds,
                float secondsLeft) override;
        void playingStopped (bool endOfSongReached) override;
        void playingPaused() override;
        void playingUnpaused() override;
        void playingErrorOccurred (const std::string& errorMessage) override;
        void mpg123CommunicationProblem (
                const boost::system::error_code& error) override;
        void mpg123Terminated (int waitpidStatus) override;
    private:
        PlaybackController& _playbackController;
        int _titlePositionUpdateCycle;
        int _frameCountOfLastUpdateCycle;
        // Performance improvement - by direct access to playback controllers
        // _secondsPlayed data member.
        float& _secondsPlayed;
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
     * Update the content of the current title file with the current title
     * position.
     */
    void updateCurrentTitleFile ();
    /**
     * Helper method to get the path of the current album file for the given
     * albums directory.
     * @param albums The path to the directory with all album sub-directories.
     * @return The path of the current album file.
     */
    static Path getCurrentAlbumFile (
            const Path& albums);
    /**
     * Helper method to get the path of the current title file for the given
     * album directory.
     * @param album The path to the album directory.
     * @return The path of the current title file.
     */
    static Path getCurrentTitleFile (
            const Path& album);
    /**
     * Helper method to get the path of the current mp3 title and the position
     * within this title for the given album directory.
     * @param album The path to the album directory.
     * @param mp3Files The content of the album directory.
     * @return The path of the current mp3 title file.
     */
    static TitlePosition getCurrentTitlePosition (
            const Path& album,
            const DirectoryList& mp3Files);
private:
    const Path& _albumsPath;
    Mp3Player& _mp3Player;
    Mp3PlayerListener _mp3PlayerListener;
    DirectoryList _albums;
    std::map<Path, DirectoryList> _albumMap;
    Path _currentAlbum;
    TitlePosition _currentTitlePosition;
    float _secondsPlayed;
    bool _presentingAlbums;
};

#endif	/* PLAYBACK_CONTROLLER_HPP */
