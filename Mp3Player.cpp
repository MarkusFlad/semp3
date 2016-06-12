#include "Mp3Player.hpp"
#include <wait.h>
#include <iostream>
#include <algorithm>
#include <boost/algorithm/string/trim.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <boost/date_time/posix_time/posix_time_duration.hpp>

using std::ostringstream;
using std::istringstream;
using std::string;
using std::endl;
using std::getline;
using std::vector;
using std::remove_if;
using std::cout;
using boost::bind;
using boost::algorithm::trim_copy;
using boost::optional;
using boost::asio::io_service;
using boost::asio::async_read_until;
using boost::asio::deadline_timer;
using boost::asio::buffer;
using boost::asio::placeholders::error;
using boost::asio::placeholders::bytes_transferred;
using boost::filesystem::path;
using boost::posix_time::milliseconds;
using boost::system::error_code;

Mp3Player::Mp3Player(const std::string& executable,
        io_service& ioService)
: _waitForId3TagsTimer (ioService)
, _mpg123Program (executable, vector<string>{"-R"}, ioService)
, _in (_mpg123Program.in())
, _out(_mpg123Program.out())
, _err(_mpg123Program.err()) {
    bindHandleInputMethod();
} 
void Mp3Player::addListener (IListener* listener) {
    _listeners.push_back(listener);
}
void Mp3Player::removeListener (IListener* listener) {
    remove_if (_listeners.begin(), _listeners.end(),
            [listener](IListener* l){return l==listener;});
}
void Mp3Player::load(const path& mp3File) {
    ostringstream oss;
    oss << "LOAD " << mp3File.string() << endl;
    string command = oss.str();
    _in.write_some (buffer(command.c_str(), command.size()));
}
void Mp3Player::pause() {
    static const string command = "PAUSE\n";
    _in.write_some (buffer(command.c_str(), command.size()));
}
void Mp3Player::jumpToBegin() {
    static const string command = "JUMP 0\n";
    _in.write_some (buffer(command.c_str(), command.size()));
}
void Mp3Player::jumpTo(int frameCount) {
    ostringstream command;
    command << "JUMP " << frameCount << endl;
    _in.write_some (buffer(command.str().c_str(), command.str().size()));
}
void Mp3Player::jumpBackward (int frames) {
    ostringstream command;
    command << "JUMP -" << frames << endl;
    _in.write_some (buffer(command.str().c_str(), command.str().size()));
}
void Mp3Player::jumpForward (int frames) {
    ostringstream command;
    command << "JUMP +" << frames << endl;
    _in.write_some (buffer(command.str().c_str(), command.str().size()));
}
void Mp3Player::bindHandleInputMethod() {
    async_read_until(_out, _inputBuffer, '\n',
            bind(&Mp3Player::handleReadInput, this, error,
            bytes_transferred));
}
void Mp3Player::handleReadInput(const error_code& error, size_t length) {
    if (!error) {
        boost::asio::streambuf::const_buffers_type bufs = _inputBuffer.data();
        string input (boost::asio::buffers_begin(bufs),
                boost::asio::buffers_end(bufs));
        _inputBuffer.consume(_inputBuffer.size());
        istringstream issInput (input);
        vector<string> messages;
        string line;
        while (getline (issInput, line, '\n')) {
            messages.push_back(line);
        }
        for (string message : messages) {
            size_t messagePos = string::npos;
            if (message.length() < 3) {
                continue;
            }
            if (messagePos = message.find(string ("@F")) != string::npos) {
                istringstream iss (message.substr(messagePos+2));
                int framecount;
                int framesLeft;
                float seconds;
                float secondsLeft;
                iss >> framecount;
                iss >> framesLeft;
                iss >> seconds;
                iss >> secondsLeft;
                for (auto l : _listeners) {
                    l->playStatus(framecount, framesLeft, seconds, secondsLeft);
                }
            } else if (messagePos = message.find(string ("@P")) != string::npos) {
                istringstream iss (message.substr(messagePos+2));
                int playingStatusCode = 0;
                string reason;
                iss >> playingStatusCode;
                iss >> reason;
                switch (playingStatusCode) {
                    case 0:
                        if (reason == "EOF") {
                            for (auto l : _listeners) {
                                l->playingStopped(true);
                            }
                        } else {
                            for (auto l : _listeners) {
                                l->playingStopped(false);
                            }
                        }
                        break;
                    case 1:
                        for (auto l : _listeners) {
                            l->playingPaused();
                        }
                        break;
                    case 2:
                        for (auto l : _listeners) {
                            l->playingUnpaused();
                        }
                        break;
                }
            } else if (messagePos = message.find(string ("@I")) != string::npos) {
                if (!_id3TagParser.isParsingStarted()) {
                    // After the first status message has been received give
                    // mpg123 200 milliseconds to send the rest of ID3 tags...
                    _waitForId3TagsTimer.expires_from_now (milliseconds(200));
                    _waitForId3TagsTimer.async_wait(bind(
                            &Mp3Player::handleStatusMessages, this, error));
                }
                _id3TagParser.parse(message.substr(messagePos+2));
            } else if (messagePos = message.find(string ("@R")) != string::npos) {
                istringstream iss (message.substr(messagePos+2));
                string mpg123;
                string version;
                iss >> mpg123;
                iss >> version;
                for (auto l : _listeners) {
                    l->mpg123Version(version);
                }
            } else if (messagePos = message.find(string ("@E")) != string::npos) {
                istringstream iss (message.substr(messagePos+2));
                string errorMessage;
                iss >> errorMessage;
                for (auto l : _listeners) {
                    l->playingErrorOccurred(errorMessage);
                }
            } else if (messagePos = message.find(string ("@S")) != string::npos) {
                istringstream iss (message.substr(messagePos+2));
            }
        }
        bindHandleInputMethod();
    } else {
        if (error == boost::asio::error::misc_errors::eof) {
            int returnStatus;    
            waitpid(_mpg123Program.pid(), &returnStatus, 0);
            for (auto l : _listeners) {
                l->mpg123Terminated(returnStatus);
            }
        } else {
            for (auto l : _listeners) {
                l->mpg123CommunicationProblem(error);
            }
        }
    }
}
void Mp3Player::handleStatusMessages(const error_code& error) {
    if (!error) {
        Mp3Title mp3Title = _id3TagParser.getMp3Title();
        for (auto l : _listeners) {
            l->titleLoaded(mp3Title);
        }
        // Reset the tag parser by a clean one.
        _id3TagParser = Id3TagParser();
    }
}
