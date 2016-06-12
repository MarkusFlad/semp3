#include "ChildProgram.hpp"
#include <unistd.h>
#include <sstream>
#include <errno.h>
#include <string.h>
#include <thread>
#include <chrono>
#include <iostream>
#include <boost/asio/io_service.hpp>

using std::cerr;
using std::endl;
using std::string;
using std::vector;
using std::ostringstream;
using boost::asio::io_service;
using boost::asio::posix::stream_descriptor;

//==============================================================================
//----------------------------- ChildProgram -----------------------------------
//==============================================================================
ChildProgram::ChildProgram (const string& executable,
        const vector<string>& args,
        io_service& ioService) throw (CreationException)
: _pid (0)
, _inFileDescriptor(0)
, _outFileDescriptor(0)
, _errFileDescriptor(0)
, _ioService (ioService) {
    int fdChildStdIn[2];
    int fdChildStdOut[2];
    int fdChildStdErr[2];

    if (pipe(fdChildStdIn) < 0) {
        throw CreationException("Unable to create pipe for child's stdin",
                errno);
    } else if (pipe(fdChildStdOut) < 0) {
        throw CreationException("Unable to create pipe for child's stdout",
                errno);
    } else if (pipe(fdChildStdErr) < 0) {
        throw CreationException("Unable to create pipe for child's stderr",
                errno);
    }
    ioService.notify_fork (io_service::fork_prepare);
    pid_t pid = fork();
    if (pid < 0) {
        throw CreationException("Error in creating the child process.");
    } else if (pid == 0) {
        ioService.notify_fork (io_service::fork_child);
        // Child process
        close(fdChildStdIn[1]);
        close(fdChildStdOut[0]);
        close(fdChildStdErr[0]);
        safeDup2(fdChildStdIn[0], STDIN_FILENO);
        safeDup2(fdChildStdOut[1], STDOUT_FILENO);
        safeDup2(fdChildStdErr[1], STDERR_FILENO);
        close(fdChildStdIn[0]);
        close(fdChildStdOut[1]);
        close(fdChildStdErr[1]);
        char** argsArray = new char*[args.size() + 2];
        argsArray[0] = new char[executable.length()+1];
        strcpy(argsArray[0], executable.c_str());
        argsArray[args.size()+ 1] = nullptr;
        for (int i=0; i<args.size(); i++) {
            const string& arg = args[i];
            argsArray[i+1] = new char[arg.length()+1];
            strcpy(argsArray[i+1], arg.c_str());
        }
        if (execv (executable.c_str(), argsArray) == -1) {
            cerr << "Unable to start child program." << strerror(errno);
        }
        for (int i=0; i<args.size()+1; i++) {
            delete[] argsArray[i];
        }
        delete[] argsArray;
    } else {
        ioService.notify_fork (io_service::fork_parent);
        // Parent process
        close(fdChildStdIn[0]);
        close(fdChildStdOut[1]);
        close(fdChildStdErr[1]);
        _inFileDescriptor = fdChildStdIn[1];
        _outFileDescriptor = fdChildStdOut[0];
        _errFileDescriptor = fdChildStdErr[0];
    }
}
stream_descriptor ChildProgram::in() const {
    return stream_descriptor (_ioService, _inFileDescriptor);
}
stream_descriptor ChildProgram::out() const {
    return stream_descriptor (_ioService, _outFileDescriptor);
}
stream_descriptor ChildProgram::err() const {
    return stream_descriptor (_ioService, _errFileDescriptor);
}
int ChildProgram::pid() const {
    return _pid;
}
void ChildProgram::safeDup2(int sourceFD, int destFD) throw (CreationException) {
    int dup2ReturnCode = 0;
    for (;;) {
        dup2ReturnCode = dup2(sourceFD, destFD);
        if (dup2ReturnCode >= 0 || errno != EINTR) {
            break;
        }
    }
    if (dup2ReturnCode < 0) {
        throw CreationException("Unable to perform dup2 for file-descriptor",
                dup2ReturnCode);
    }
}
//==============================================================================
//-------------------- ChildProgram::CreationException -------------------------
//==============================================================================
ChildProgram::CreationException::CreationException(const string& message)
: _message(message)
{
}
ChildProgram::CreationException::CreationException(const std::string& message,
        int errorNumber) {
    ostringstream oss;
    oss << message << ": errno=" << errorNumber << "(" << strerror(errno) <<
            ")";
    _message = oss.str();
}
ChildProgram::CreationException::~CreationException() throw () {
}
const char* ChildProgram::CreationException::what() const throw () {
    return _message.c_str();
}
