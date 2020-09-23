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
#include "MagellanDataModel.hpp"

const size_t MAX_CMD_BUFF_SIZE = 4096;
const char *LOG_TAG = "mth";

std::map<std::string, Magellan::DataModel::Talkgroup>   m_talkgroups;

void loggingHook(int level, const char * _Nonnull tag, const char *msg);
int discoveryFilterHook(const char * _Nonnull detailJson, const void * _Nullable userData);

void onNewTalkgroups(const char * _Nonnull newTalkgroupsJson, const void * _Nullable userData);
void onModifiedTalkgroups(const char * _Nonnull modifiedTalkgroupsJson, const void * _Nullable userData);
void onRemovedTalkgroups(const char * _Nonnull removedTalkgroupsJson, const void * _Nullable userData);

void showUsage();
void runTest1();
void runTest2();
void runTest3();
void runTest4();
std::string loadConfiguration(const char *fn);
void showTalkgroups();
void showHelp();

int m_testLoops = 0;

int main(int argc, char **argv)
{
    printf("---------------------------------------------------------------------------\n");
    printf("Magellan Test Harness (mth) version 0.1\n");
    printf("\n");
    printf("Copyright (c) 2020 Rally Tactical Systems, Inc.\n");
    printf("Build time: %s @ %s\n", __DATE__, __TIME__);
    printf("---------------------------------------------------------------------------\n");

    const char *cfgFile = nullptr;

    for(int x = 1; x < argc; x++)
    {
        if(strncmp(argv[x], "-cfg:", 5) == 0)
        {
            cfgFile = (argv[x] + 5);
        }
        else if(strncmp(argv[x], "-tl:", 4) == 0)
        {
            m_testLoops = atoi(argv[x] + 4);
        }
        else
        {
            printf("ERROR: unknown option '%s'\n", argv[x]);
            showUsage();
            return 1;
        }        
    }

    std::string configJson;
    
    if( cfgFile != nullptr)
    {
        configJson = loadConfiguration(cfgFile);
        if(configJson.empty())
        {
            printf("ERROR: cannot load configuration from '%s'\n", cfgFile);
            return 1;
        }
    }
    else
    {
        printf("WARNING: no configuration provided' - using defaults\n");
        configJson.assign("{}");
    }

    // Set the logging hook before anything so that we can log initialization
    magellanSetLoggingHook(&loggingHook);

    // Fire up the library
    magellanInitialize(configJson.c_str());

    // Register our talkgroup notification functions
    magellanSetTalkgroupCallbacks(onNewTalkgroups, onModifiedTalkgroups, onRemovedTalkgroups, nullptr);

    // Run this test
    runTest3();    

    // Shut it down
    magellanShutdown();

    return 0;
}

std::string loadConfiguration(const char *fn)
{
    std::string rc;
    FILE *fp = nullptr;

    #ifndef WIN32
        fp = fopen(fn, "rb");
    #else
        if(fopen_s(&fp, fn, "rb") != 0)
        {
            fp = nullptr;
        }
    #endif

    if(fp != nullptr)
    {
        fseek(fp, 0, SEEK_END);
        size_t sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        char *buff = new char[sz + 1];
        if( fread(buff, 1, sz, fp) == sz )
        {
            buff[sz] = 0;
            rc.assign(buff, sz);
        }
        delete[] buff;

        fclose(fp);
    }

    return rc;
}

void showUsage()
{
    printf("usage: mth [-cfg:configuration_json_file]\n");
}

void showHelp()
{
    printf("\n");
    printf("=====HELP=====\n");

    printf("q      .................... quit\n");
    printf("?      .................... help\n");
    printf("clear  .................... clear the screen\n");
    printf("sg     .................... show talkgroups\n");

    printf("\n");
}

void showTalkgroups()
{
    printf("\n");
    printf("=====TALKGROUPS=====\n");

    printf("ID                                                               Name                                                             Device\n");
    printf("---------------------------------------------------------------- ---------------------------------------------------------------- ------------------------------------------------------------------------\n");
    for(std::map<std::string, Magellan::DataModel::Talkgroup>::iterator itr = m_talkgroups.begin();
        itr != m_talkgroups.end();
        itr++)
    {
        printf("%-64s %-64s %-64s\n", 
                itr->second.id.c_str(),
                itr->second.name.c_str(),
                itr->second.deviceKey.c_str());
    }

    printf("\n");
}

void loggingHook(int level, const char * tag, const char *msg)
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

int DiscoveryHook(const char * _Nonnull detailJson, const void * _Nullable userData)
{
    magellanLogMessage(MAGELLAN_LOG_LEVEL_INFORMATIONAL, LOG_TAG, detailJson);

    // We will always proceed
    return MAGELLAN_FILTER_PROCEED;
}

void onNewTalkgroups(const char * _Nonnull newTalkgroupsJson, const void * _Nullable userData)
{    
    try
    {
        std::vector<Magellan::DataModel::Talkgroup> tga = nlohmann::json::parse(newTalkgroupsJson);

        for(std::vector<Magellan::DataModel::Talkgroup>::iterator itr = tga.begin();
            itr != tga.end();
            itr++)
        {
            char buff[1024];

            std::map<std::string, Magellan::DataModel::Talkgroup>::iterator itrFnd = m_talkgroups.find(itr->id);
            if(itrFnd == m_talkgroups.end())
            {
                m_talkgroups[itr->id] = *itr;

                sprintf(buff, "New TG: '%s' - '%s'",
                        itr->id.c_str(),
                        itr->name.c_str());

                magellanLogMessage(MAGELLAN_LOG_LEVEL_INFORMATIONAL, LOG_TAG, buff);
            }
            else
            {
                sprintf(buff, "New TG: '%s' - '%s' - ALREADY EXISTS",
                        itr->id.c_str(),
                        itr->name.c_str());

                magellanLogMessage(MAGELLAN_LOG_LEVEL_FATAL, LOG_TAG, buff);
            }
        }
    }
    catch(...)
    {
        magellanLogMessage(MAGELLAN_LOG_LEVEL_FATAL, LOG_TAG, "exception while processing new talkgroup json");
    }
}

void onModifiedTalkgroups(const char * _Nonnull modifiedTalkgroupsJson, const void * _Nullable userData)
{
    try
    {
        std::vector<Magellan::DataModel::Talkgroup> tga = nlohmann::json::parse(modifiedTalkgroupsJson);

        for(std::vector<Magellan::DataModel::Talkgroup>::iterator itr = tga.begin();
            itr != tga.end();
            itr++)
        {
            char buff[1024];

            std::map<std::string, Magellan::DataModel::Talkgroup>::iterator itrFnd = m_talkgroups.find(itr->id);
            if(itrFnd == m_talkgroups.end())
            {
                sprintf(buff, "Updated TG: '%s' - '%s' - DOES NOT EXIST",
                        itr->id.c_str(),
                        itr->name.c_str());

                magellanLogMessage(MAGELLAN_LOG_LEVEL_FATAL, LOG_TAG, buff);
            }
            else
            {
                sprintf(buff, "Updated TG: '%s' - '%s'",
                        itr->id.c_str(),
                        itr->name.c_str());

                magellanLogMessage(MAGELLAN_LOG_LEVEL_INFORMATIONAL, LOG_TAG, buff);
            }

            // Always update anyway
            m_talkgroups[itr->id] = *itr;
        }
    }
    catch(...)
    {
        magellanLogMessage(MAGELLAN_LOG_LEVEL_FATAL, LOG_TAG, "exception while processing modified talkgroup json");
    }    
}

void onRemovedTalkgroups(const char * _Nonnull removedTalkgroupsJson, const void * _Nullable userData)
{
    try
    {
        std::vector<std::string> ida = nlohmann::json::parse(removedTalkgroupsJson);

        for(std::vector<std::string>::iterator itr = ida.begin();
            itr != ida.end();
            itr++)
        {
            char buff[1024];

            std::map<std::string, Magellan::DataModel::Talkgroup>::iterator itrFnd = m_talkgroups.find(*itr);
            if(itrFnd == m_talkgroups.end())
            {
                sprintf(buff, "Removed TG: '%s' - DOES NOT EXIST",
                        itr->c_str());

                magellanLogMessage(MAGELLAN_LOG_LEVEL_FATAL, LOG_TAG, buff);
            }
            else
            {                
                sprintf(buff, "Removed TG: '%s' - '%s'",
                    itr->c_str(),
                    itrFnd->second.name.c_str());

                magellanLogMessage(MAGELLAN_LOG_LEVEL_INFORMATIONAL, LOG_TAG, buff);     

                m_talkgroups.erase(itrFnd);
            }
        }
    }
    catch(...)
    {
        magellanLogMessage(MAGELLAN_LOG_LEVEL_FATAL, LOG_TAG, "exception while processing removed talkgroup json");
    }    
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

    else if(strcmp(buff, "?") == 0)
    {
        showHelp();
    }    

    else if(strcmp(buff, "clear") == 0)
    {
        system("clear");
    }    

    else if(strcmp(buff, "sg") == 0)
    {
        showTalkgroups();
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

void runTest1()
{
    printf("starting test1\n");

    MagellanToken_t                 t;
    std::vector<MagellanToken_t>    tokens;

    // Default discoverer (MDNS)
    t = MAGELLAN_NULL_TOKEN;
    magellanBeginDiscovery(MAGELLAN_DEFAULT_DISCOVERY_TYPE, &t, DiscoveryHook, nullptr);
    tokens.push_back(t);
    
    commandInputLoop("test1");

    for(std::vector<MagellanToken_t>::iterator itr = tokens.begin();
        itr != tokens.end();
        itr++)
    {
        magellanEndDiscovery(*itr);
    }

    printf("ended test1\n");
}

void runTest2()
{
    printf("starting test2\n");

    MagellanToken_t                 t;
    std::vector<MagellanToken_t>    tokens;

    t = MAGELLAN_NULL_TOKEN;
    magellanBeginDiscovery(MAGELLAN_SSDP_DISCOVERY_TYPE, &t, DiscoveryHook, nullptr);
    tokens.push_back(t);
    
    commandInputLoop("test2");

    for(std::vector<MagellanToken_t>::iterator itr = tokens.begin();
        itr != tokens.end();
        itr++)
    {
        magellanEndDiscovery(*itr);
    }

    printf("ended test2\n");
}

void runTest3()
{
    printf("starting test3\n");

    MagellanToken_t                 t;
    std::vector<MagellanToken_t>    tokens;

    // MDNS
    {
        t = MAGELLAN_NULL_TOKEN;
        magellanBeginDiscovery(MAGELLAN_MDNS_DISCOVERY_TYPE, &t, DiscoveryHook, nullptr);
        tokens.push_back(t);
    }

    // SSDP
    {
        t = MAGELLAN_NULL_TOKEN;
        magellanBeginDiscovery(MAGELLAN_SSDP_DISCOVERY_TYPE, &t, DiscoveryHook, nullptr);
        tokens.push_back(t);
    }
    
    commandInputLoop("test3");

    for(std::vector<MagellanToken_t>::iterator itr = tokens.begin();
        itr != tokens.end();
        itr++)
    {
        magellanEndDiscovery(*itr);
    }

    printf("ended test3\n");
}


void runTest4()
{
    if(m_testLoops <= 0)
    {
        m_testLoops = 100;
    }

    printf("starting test4 for %d test loops\n", m_testLoops);

    for(int x = 0; x < m_testLoops; x++)
    {
        printf("*********** LOOP %d *************\n", x);

        MagellanToken_t                 t;
        std::vector<MagellanToken_t>    tokens;

        // MDNS
        {
            t = MAGELLAN_NULL_TOKEN;
            magellanBeginDiscovery(MAGELLAN_MDNS_DISCOVERY_TYPE, &t, DiscoveryHook, nullptr);
            tokens.push_back(t);
        }

        // SSDP
        {
            t = MAGELLAN_NULL_TOKEN;
            magellanBeginDiscovery(MAGELLAN_SSDP_DISCOVERY_TYPE, &t, DiscoveryHook, nullptr);
            tokens.push_back(t);
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        for(std::vector<MagellanToken_t>::iterator itr = tokens.begin();
            itr != tokens.end();
            itr++)
        {
            magellanEndDiscovery(*itr);
        }
    }

    printf("ended test4\n");
}