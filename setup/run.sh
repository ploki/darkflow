#!/bin/bash

set -e

cd "$(dirname $0)/.."

RHOME=/home/darkflow

DIST="stretch.amd64"

mkdir -p ~/DARKFLOW ~/.config/darkflow

sudo docker run -it -u $(id -u):$(id -g)\
     -v /tmp/.X11-unix:/tmp/.X11-unix \
     -v ~/DARKFLOW/:$RHOME/DARKFLOW/ \
     -v ~/.config/darkflow/:$RHOME/.config/darkflow/ \
     -v $XAUTHORITY:$RHOME/.Xauthority \
     -e XAUTHORITY=$RHOME/.Xauthority \
     -e DISPLAY=$DISPLAY \
     -e HOME=$RHOME \
     -e LIBGL_ALWAYS_SOFTWARE=1 \
     "darkflow/$DIST"
