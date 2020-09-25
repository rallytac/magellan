@echo off

rem
rem Copyright (c) 2020 Rally Tactical Systems, Inc.
rem All rights reserved.
rem

setlocal enableextensions

set PROJ_ROOT=%~dp0
set SRC_ROOT=%PROJ_ROOT%\src
set BUILD_ROOT=%PROJ_ROOT%\.build

md "%BUILD_ROOT%"
cd "%BUILD_ROOT%"

if not exist "CMakeFiles" (
    cmake "%SRC_ROOT%"
)

cmake --build .
