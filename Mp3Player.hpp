#ifndef MP3PLAYER_HPP
#define	MP3PLAYER_HPP

#include "ChildProgram.hpp"
#include "Id3TagParser.hpp"
#include "Mp3Title.hpp"
#include <boost/asio/streambuf.hpp>
#include <boost/asio/deadline_timer.hpp>

namespace boost {
    namespace asio {
        class io_service;
    }
    namespace filesystem {
        class path;
    }
    namespace system {
        class error_code;
    }
}

class Mp3Player {
public:
   class IListener {
    public:
        virtual ~IListener() {}
        virtual void mpg123Version (const std::string& message) = 0;
        virtual void titleLoaded (const Mp3Title& title) = 0;
        virtual void playStatus (int framecount, int framesLeft,
                                 float seconds, float secondsLeft) = 0;
        virtual void playingStopped (bool endOfSongReached) = 0;
        virtual void playingPaused() = 0;
        virtual void playingUnpaused() = 0;
        virtual void playingErrorOccurred (const std::string& errorMessage) = 0;
        virtual void mpg123CommunicationProblem (
            const boost::system::error_code& error) = 0;
        virtual void mpg123Terminated (int waitpidStatus) = 0;
    };
    Mp3Player (const std::string& executable,
        boost::asio::io_service& ioService);
    void addListener (IListener* listener);
    void removeListener (IListener* listener);
    void load (const boost::filesystem::path& mp3File);
    bool isLoadCompleted() const;
    void pause();
    void jumpToBegin();
    void jumpTo(int frameCount);
    bool isJumpToCompleted() const;
    void jumpBackward (int frames);
    void jumpForward (int frames);

protected:
    void bindHandleInputMethod();
    void handleReadInput(const boost::system::error_code& error, size_t length);
    void handleStatusMessages(const boost::system::error_code& error);
    
private:
    boost::asio::deadline_timer _waitForId3TagsTimer;
    const ChildProgram _mpg123Program;
    boost::asio::posix::stream_descriptor _in;
    boost::asio::posix::stream_descriptor _out;
    boost::asio::posix::stream_descriptor _err;
    boost::asio::streambuf _inputBuffer;
    std::vector<IListener*> _listeners;
    Id3TagParser _id3TagParser;
    bool _loadCompleted;
    int _jumpToFrameCount;
    bool _jumpToCompleted;
};

#endif	/* MP3PLAYER_HPP */

