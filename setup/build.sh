#!/bin/bash

set -ex

cd "$(dirname $0)/.."

DIST="stretch.amd64"

sudo docker build -f "setup/live/$DIST" \
     -t "darkflow/$DIST" \
     .
