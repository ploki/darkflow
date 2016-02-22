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

### Debian packages

The current binary distribution needs ImageMagick 6.9.3 from experimental.

``` bash
$ echo deb http://darkflow.org/debian/ experimental main | sudo tee /etc/apt/sources.list.d/darkflow.list
$ apt-get update
$ apt-get install darkflow
```

### Ubuntu packages

``` bash
$ echo deb http://darkflow.org/ubuntu/ wily main | sudo tee /etc/apt/sources.list.d/darkflow.list
$ apt-get update
$ apt-get install darkflow
```

### Windows packages

Windows (x86 and x64) packages are available at http://darkflow.org/download/
