#
#  Copyright (c) 2020 Rally Tactical Systems, Inc.
#  All rights reserved.
#

cmake_minimum_required(VERSION 3.4.1)

project(rtsmagellan)

set(CMAKE_CXX_STANDARD 11)
set(SANITIZE FALSE)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
    set(LINUX true)
endif()

if(${CMAKE_SYSTEM_NAME} MATCHES "Windows")
    set(WINDOWS true)
endif()

if(LINUX)
    add_compile_options(-Wall 
                        -Werror
                        -fPIC
                        -Wno-strict-aliasing
                        -Wno-unused-value
                        -fvisibility=hidden
                        -O0 
                        -ggdb 
                        -pedantic)

    if(SANITIZE)
        add_compile_options(-fsanitize=address
                            -fno-omit-frame-pointer)                    
    endif()
endif()

#------------------------------------------------
# magellan-static library
#------------------------------------------------
set(SOURCES MagellanCore.cpp
            MagellanApi.cpp
            WorkQueue.cpp
            SimpleLogger.cpp
            ReferenceCountedObject.cpp
            AppDiscoverer.cpp            
            TimerManager.cpp
            SsdpDiscoverer.cpp)

if(LINUX)
    set(SOURCES ${SOURCES} AvahiDiscoverer.cpp)
else()
    set(SOURCES ${SOURCES} BonjourDiscoverer.cpp)
endif()    

add_library(magellan-static STATIC ${SOURCES})

target_include_directories(magellan-static PRIVATE ${CMAKE_SOURCE_DIR})

if(WINDOWS)
    target_include_directories(magellan-static PRIVATE 
                                ${CMAKE_SOURCE_DIR}/bonjour/Win/Include
                                ${CMAKE_SOURCE_DIR}/curl/win
                                )
endif()

target_precompile_headers(magellan-static PUBLIC MagellanDataModel.hpp)

#------------------------------------------------
# mth executable
#------------------------------------------------
add_executable(mth mth.cpp)

target_precompile_headers(mth PUBLIC MagellanDataModel.hpp)

target_include_directories(mth PRIVATE ${CMAKE_SOURCE_DIR})

if(LINUX)
    if(SANITIZE)
        target_link_libraries(mth PRIVATE asan)
    endif()

    target_link_libraries(mth PRIVATE magellan-static avahi-client avahi-common pthread curl ssl crypto)
endif()

if(WINDOWS)
    target_link_libraries(mth PRIVATE 
                                magellan-static 
                                ws2_32 
                                crypt32 
                                wldap32 
                                ${CMAKE_SOURCE_DIR}/curl/win/lib/x86/libcurl.lib
                                ${CMAKE_SOURCE_DIR}/bonjour/Win/Lib/Win32/dnssd.lib)
endif()
