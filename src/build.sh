#!/bin/bash

SRC_ROOT=`pwd`
BUILD_ROOT=${SRC_ROOT}/.build

mkdir -p "${BUILD_ROOT}"
cd "${BUILD_ROOT}"

if [ ! -d CMakeFiles ]; then
    cmake "${SRC_ROOT}"
fi

make
