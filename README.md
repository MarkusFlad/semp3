# semp3
Simple MP3 player running on the Raspberry Pi. Uses mpg123 via its generic
remote interface. Implemented in C++11.

Besides the Raspberry Pi the program can also be compiled and run on a Desktop
PC and be controlled with a keyboard. Note that this is forseen for testing
purposes.

Development can be done on the command line. However the Netbeans IDE is
recommended. Corresponding Netbeans project files are part of the project
(under folder nbproject). Note that the Makefiles have been automatically
created by Netbeans and they should be not edited manually.

This source code and application is released under the Apache 2.0 license.
See the attached LICENSE file.

Prerequisites for Raspberry Pi:
1. Operating System: Raspbian Jessie
   (see https://www.raspberrypi.org/downloads/raspbian/)
2. C++ compiler (can be installed with "sudo apt-get install g++")
3. Boost libraries (can be installed with "sudo apt-get libboost-dev")
4. mpg123 - Fast console MPEG Audio Player
   (can be installed with "sudo apt-get install mpg123")

The prerequisites for Desktop PC are in principle the same. Since the makefiles
are for gcc Linux is the highly recommended operating system. The distribution
has to be not to old and support the C++11 standard as well as a new boost
library.

To build and run on the command line on the Raspberry Pi clone the repository
from GitHub and switch to the corresponding branch (e.g. Master branch for the
latest version). Therefore Type:
git clone -b master https://github.com/MarkusFlad/semp3.git

Now build the executable application. There is a dedicated makefile for building
the Raspberry Pi artifacts. Just type:
make -f nbproject/Makefile-WiringPi.mk

