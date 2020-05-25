//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

/**
 * @brief This is the Magellan Test Harness utility meant for developer-level testing
 * and debugging of the Magellan library.
 **/

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#ifdef WIN32
    #include <windows.h>
#else
    #include <sys/time.h>
#endif
#include <math.h>
#include <chrono>
#include <string>
#include <thread>
#include <vector>

#include "MagellanApi.h"

const size_t MAX_CMD_BUFF_SIZE = 4096;
const char *LOG_TAG = "mth";

void LoggingHook(int level, const char * _Nonnull tag, const char *msg);
void DiscoveryHook(const char * _Nonnull detailJson, const void * _Nullable userData);
void showUsage();
void runTest1(int argc, char **argv);

int main(int argc, char **argv)
{
    printf("---------------------------------------------------------------------------\n");
    printf("mth version 0.1\n");
    printf("\n");
    printf("Copyright (c) 2020 Rally Tactical Systems, Inc.\n");
    printf("Build time: %s @ %s\n", __DATE__, __TIME__);
    printf("---------------------------------------------------------------------------\n");

    magellanSetLoggingHook(&LoggingHook);

    runTest1(argc, argv);

    return 0;
}

void showUsage()
{
}

void LoggingHook(int level, const char * tag, const char *msg)
{
    static const char *COLOR_ERROR = "\033[31;1m";
    static const char *COLOR_DEBUG = "\033[37m";
    static const char *COLOR_INFO = "\033[32;1m";
    static const char *COLOR_WARNING = "\033[33;1m";
    static const char *COLOR_FATAL = "\033[37;1;41m";

    static const char *ANSI_RESET = "\033[0m";

    int millisec;
    struct tm tm_info;
    struct timeval tv;
    char timeBuff[64];
    char levelChar;
    const char *clr;

    if(level == 4)
    {
        levelChar = 'D';
        clr = COLOR_DEBUG;
    }
    else if(level == 3)
    {
        levelChar = 'I';
        clr = COLOR_INFO;
    }
    else if(level == 2)
    {
        levelChar = 'W';
        clr = COLOR_WARNING;
    }
    else if(level == 1)
    {
        levelChar = 'E';
        clr = COLOR_ERROR;
    }
    else if(level == 0)
    {
        levelChar = 'F';
        clr = COLOR_FATAL;
    }
    else
    {
        levelChar = 'D';
        clr = COLOR_DEBUG;
    }

    #if defined(WIN32)
        static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

        SYSTEMTIME  system_time;
        FILETIME    file_time;
        uint64_t    time;

        GetSystemTime(&system_time);
        SystemTimeToFileTime(&system_time, &file_time);
        time = ((uint64_t)file_time.dwLowDateTime);
        time += ((uint64_t)file_time.dwHighDateTime) << 32;

        tv.tv_sec = (long)((time - EPOCH) / 10000000L);
        tv.tv_usec = (long)(system_time.wMilliseconds * 1000);
    #else
        gettimeofday(&tv, NULL);
    #endif

    millisec = (int) lrint(tv.tv_usec/1000.0);
    if (millisec >= 1000)
    {
        millisec -=1000;
        tv.tv_sec++;
    }

	time_t t = tv.tv_sec;

    #ifdef WIN32
        localtime_s(&tm_info, &t);
    #else
        struct tm *p = localtime(&t);
        memcpy(&tm_info, p, sizeof(tm_info));
    #endif

    strftime(timeBuff, sizeof(timeBuff), "%Y:%m:%d %H:%M:%S", &tm_info);

    printf("%s%s - %c/%s %s%s\n", clr, timeBuff, levelChar, tag, msg, ANSI_RESET);
    fflush(stdout);
}

void DiscoveryHook(const char * _Nonnull detailJson, const void * _Nullable userData)
{
    magellanLogMessage(MAGELLAN_LOG_LEVEL_INFORMATIONAL, LOG_TAG, detailJson);
}

bool readInput(char *buff, size_t maxSize)
{
    if( !fgets(buff, maxSize, stdin) )
    {
        return false;
    }

    char *p = (buff + strlen(buff) - 1);
    while((p >= buff) && (*p == '\n'))
    {
        *p = 0;
        p--;
    }

    return true;
}

bool processCommandBuffer(char *buff)
{
    if(buff[0] == 0)
    {
        return true;
    }

    else if(strcmp(buff, "q") == 0)
    {
        return false;
    }

    return true;
}

void commandInputLoop(const char *prompt)
{
    while( true )
    {
        char buff[MAX_CMD_BUFF_SIZE];
        printf("............%s >", prompt);

        memset(buff, 0, sizeof(buff));
        if( !readInput(buff, sizeof(buff)) )
        {
            continue;
        }

        if( !processCommandBuffer(buff) )
        {
            break;
        }
    }    
}

void runTest1(int argc, char **argv)
{
    printf("starting test1\n");

    MagellanToken_t                 t;
    std::vector<MagellanToken_t>    tokens;

    // Add the default discoverer
    t = MAGELLAN_NULL_TOKEN;
    magellanBeginDiscovery(nullptr, &t, DiscoveryHook, nullptr);
    tokens.push_back(t);

    // Add additional discoverers
    for(int x = 1; x < argc; x++)
    {
        t = MAGELLAN_NULL_TOKEN;
        magellanBeginDiscovery(argv[x], &t, DiscoveryHook, nullptr);
        tokens.push_back(t);
    }
    
    commandInputLoop("test1");

    for(std::vector<MagellanToken_t>::iterator itr = tokens.begin();
        itr != tokens.end();
        itr++)
    {
        magellanEndDiscovery(*itr);
    }

    printf("ended test1\n");
}
