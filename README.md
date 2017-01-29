# DarkFlow, a node based image processor for astronomers


This software is dedicated to astronomical images processing. It gives you the opportunity to track the processing of your images using a non destructive approach. So you no longer need to use your memory to replay treatments you have taken some much time to implement.


## Screenshot

![DarkFlow in action](http://darkflow.org/images/df-2016-02-01bis.jpg)

## Availability

DarkFlow is always available from https://github.com/ploki/darkflow

All the code I wrote in this package is distributed under the 3-clause BSD License.

By the way, it includes in some places code from other projects which may taint binary distribution. At the time of writing, binary distribution falls under the LGPL 2.1.

### Dependencies
 - Qt 5.4
 - ImageMagick (Magick++) 6.9 (6.8 suffers from an annoying bug with locking)
 - ffmpeg (libavformat, libavcodec, libavutil)
 
### Code borrowed
 - libdc1394 (bayer.c and bayer.h)

## Installation

### Run in a docker container on any GNU/Linux

Make sure docker is installed and your user is a sudoer or type the following commands, then logout and login in your graphical environment.

``` bash
# apt-get install docker.io
# adduser $(id -un) sudo
```

When logged in again, checkout and build once.


``` bash
$ git clone https://github.com/ploki/darkflow.git
$ cd darkflow
$ setup/build.sh
```

And run it (many times) from a terminal within your graphical environment.
DarkFlow expects your photos to be in the directory ```DARKFLOW``` in your home directory.

``` bash
$ setup/run.sh
```


### Build from source


You'll need a complete Qt5 development environment, and all development packages for the dependencies. The simpler is to use qtcreator and to click on the big green triangle on the lower left corner of the screen.
``` bash
$ git clone https://github.com/ploki/darkflow.git
$ mkdir darkflow-build
$ cd darkflow-build/
$ qmake ../darkflow CONFIG+=release
$ make
$ ./darkflow
```

### Debian and Ubuntu packages

Currently supported distributions
 - Debian stretch amd64 and i386
 - Debian jessie amd64 and i386
 - Ubuntu yakkety amd64 and i386
 - Ubuntu xenial amd64 and i386
 - Ubuntu wily amd64

``` bash
$ sudo apt-get install lsb-release
$ echo deb http://darkflow.org/$(lsb_release -si).$(dpkg --print-architecture)/ $(lsb_release -sc) main | sudo tee /etc/apt/sources.list.d/darkflow.list
$ sudo apt-get update
$ sudo apt-get install darkflow
```

### Windows packages

Windows (x86 and x64) packages are available at http://darkflow.org/download/windows/

### Macintosh OS X app

OS X package is available at http://darkflow.org/download/osx/
