#!/bin/bash

set -e

cd "$(dirname $0)/.."

REPO=apt

rm -rf $REPO

function Debian()
{
    echo stretch jessie
}

function Ubuntu()
{
    echo yakkety xenial wily
}

DISTS="Debian Ubuntu"

COMPONENT=main

for dist in $DISTS ; do
    for release in $($dist) ; do
        for arch in i386 amd64 ; do
            if [ -d "pkg/$release.$arch" ] ; then
                mkdir -p $REPO/$dist.$arch/dists/$release/$COMPONENT/binary-{amd64,i386} \
                      $REPO/$dist.$arch/pool/$release/$COMPONENT
                cp pkg/$release.$arch/*.deb $REPO/$dist.$arch/pool/$release/$COMPONENT
                for apt_arch in i386 amd64 ; do
                    ( cd $REPO/$dist.$arch ; apt-ftparchive --arch $apt_arch \
                                                            packages pool/$release \
                                                            /dev/null > dists/$release/$COMPONENT/binary-$apt_arch/Packages )
                    ( cd $REPO/$dist.$arch/dists/$release ; apt-ftparchive release . > ../../dists/$release/Release )
                done
            fi
        done
    done
done
