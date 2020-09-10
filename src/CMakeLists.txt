#
#  Copyright (c) 2020 Rally Tactical Systems, Inc.
#  All rights reserved.
#

cmake_minimum_required(VERSION 3.4.1)

project(rtsmagellan)

set(CMAKE_CXX_STANDARD 11)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX true)
endif()

add_compile_options(-Wall 
                    -Werror
                    -fPIC
                    -Wno-strict-aliasing
                    -Wno-unused-value
                    -fvisibility=hidden
                    -O0 
                    -ggdb 
                    -pedantic)

#------------------------------------------------
# magellan-static library
#------------------------------------------------
set(SOURCES MagellanCore.cpp
            MagellanApi.cpp
            WorkQueue.cpp
            SimpleLogger.cpp
            ReferenceCountedObject.cpp
            AppDiscoverer.cpp)

if(LINUX)
    set(SOURCES ${SOURCES} AvahiDiscoverer.cpp)
endif()    

add_library(magellan-static STATIC ${SOURCES})

target_include_directories(magellan-static PRIVATE ./)

target_precompile_headers(magellan-static PUBLIC MagellanJson.hpp)

#------------------------------------------------
# mth executable
#------------------------------------------------
add_executable(mth mth.cpp)

target_include_directories(mth PRIVATE ./)

if(LINUX)
    target_link_libraries(mth PRIVATE magellan-static avahi-client avahi-common pthread curl)
endif()
