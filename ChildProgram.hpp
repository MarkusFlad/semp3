#ifndef CHILDPROGRAM_HPP
#define	CHILDPROGRAM_HPP

#include <string>
#include <vector>
#include <exception>
#include <boost/asio/posix/stream_descriptor.hpp>

namespace boost {
    namespace asio {
        class io_service;
    }
}

class ChildProgram {
public:
    class CreationException : public std::exception {
        public:
            CreationException (const std::string& message);
            CreationException (const std::string& message, int errorNumber);
            ~CreationException () throw ();
            virtual const char* what() const throw();
        private:
            std::string _message;
    };
    typedef boost::asio::posix::stream_descriptor StreamDescriptor;
    ChildProgram (const std::string& executable,
            const std::vector<std::string>& commandLine,
            boost::asio::io_service& _ioService) throw (CreationException);
    StreamDescriptor in() const;
    StreamDescriptor out() const;
    StreamDescriptor err() const;
    int pid() const;
protected:
    /**
     * Call dup2 system call and repeat this until it works - to prevent from
     * the case being interrupted by a signal.
     * @parm sourceFD The source file descriptor. This remains open after the
     *                call to dup2.
     * @parm destFD The destination file descriptor. This file descriptor will
     *              point to the same file as filedes after this call returns.
     * @exception Thrown for other errors than EINTR - interrupted function
     *            call. */
    static void safeDup2 (int sourceFD, int destFD) throw (CreationException);
private:
    int _pid;
    int _inFileDescriptor;
    int _outFileDescriptor;
    int _errFileDescriptor;
    boost::asio::io_service& _ioService;
};

#endif	/* CHILDPROGRAM_HPP */

