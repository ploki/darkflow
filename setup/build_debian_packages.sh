#!/bin/bash

set -e

cd "$(dirname $0)/.."

find setup/Dockerfile -name '*~' -delete

for TARGETP in setup/Dockerfile/* ; do
    TARGET="$(basename "$TARGETP")"
    mkdir -p "pkg/$TARGET"
    docker build -f "setup/Dockerfile/$TARGET" -t "darkflow/$TARGET" .
    docker run -v "$PWD/pkg/$TARGET:/artifacts"  "darkflow/$TARGET"
done
