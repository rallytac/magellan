//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <chrono>
#include <string>
#include <thread>

#include "MagellanApi.h"

void LoggingHook(int level, const char * _Nonnull tag, const char *msg);
void showUsage();
void runTest1();

int main(int argc, char **argv)
{
    printf("---------------------------------------------------------------------------\n");
    printf("mth version 0.1\n");
    printf("\n");
    printf("Copyright (c) 2020 Rally Tactical Systems, Inc.\n");
    printf("Build time: %s @ %s\n", __DATE__, __TIME__);
    printf("---------------------------------------------------------------------------\n");

    //magellanSetLoggingHook(&LoggingHook);

    runTest1();

    return 0;
}

void showUsage()
{

}

void LoggingHook(int level, const char * tag, const char *msg)
{

}

void runTest1()
{
    printf("starting test1\n");

    int                 rc;
    MagellanToken_t     token = NULL_MAGELLAN_TOKEN;

    rc = magellanBeginDiscovery("", &token, (void*) 0xDEADBEEF);
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    rc = magellanEndDiscovery(token);

    printf("ended test1\n");
}