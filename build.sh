#!/bin/bash

#
#  Copyright (c) 2020 Rally Tactical Systems, Inc.
#  All rights reserved.
#

PROJ_ROOT=`pwd`
SRC_ROOT=${PROJ_ROOT}/src
BUILD_ROOT=${PROJ_ROOT}/.build

mkdir -p "${BUILD_ROOT}"
cd "${BUILD_ROOT}"

if [ ! -d CMakeFiles ]; then
    cmake "${SRC_ROOT}"
fi

cmake --build .
