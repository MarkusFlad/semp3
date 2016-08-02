/* 
 * File:   main.cpp
 * Author: markus
 *
 * Created on 28. Dezember 2015, 21:24
 */

#include "Mp3Player.hpp"
#include "OneButtonPlaybackController.hpp"
#include "Frontend.hpp"
#include <boost/asio/io_service.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/operations.hpp>
#include <iostream>

using std::cerr;
using std::endl;
using std::shared_ptr;
using boost::asio::io_service;
using boost::filesystem::path;
using boost::filesystem::exists;
using boost::filesystem::is_directory;

int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "Usage: semp3 <albums-directory>" << endl;
        return EXIT_FAILURE;
    }
    path albums = argv[1];
    if (!exists (albums)) {
        cerr << "Given albums-directory '" << argv[1]
             << "' does not exist." << endl;
        return EXIT_FAILURE;
    } else if (!is_directory (albums)) {
        cerr << "Given file '" << argv[1]
             << "' is not a directory." << endl;
        return EXIT_FAILURE;
    }
    boost::asio::io_service ioService;
    Mp3Player mp3Player ("/usr/bin/mpg123", ioService);
    OneButtonPlaybackController playbackController (albums, mp3Player,
                                                    ioService);
    if (!playbackController.resume()) {
        cerr << "Given albums-directory contains no valid album-directory."
             << endl;
        return EXIT_FAILURE;
    }
    shared_ptr<Frontend> frontend = Frontend::create (playbackController);
    ioService.run();
    return EXIT_SUCCESS;
}

