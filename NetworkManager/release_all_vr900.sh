#!/bin/bash

#################################################################################################
# This script builds all the projects from parent folder.
##################################################################################################


cd "$CRTDIR"

cd ../NetworkEngine
CRTDIR=`pwd`
echo "Building project NetworkEngine from: ${CRTDIR}"
make -s TOOLCHAIN=m68k-unknown-linux-uclibc -j4


cd ../NetworkManager
CRTDIR=`pwd`

echo "Building project NetworkManager from: ${CRTDIR}"
make -s TOOLCHAIN=m68k-unknown-linux-uclibc -j4 compile
