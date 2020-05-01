//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <mutex>

#include "MagellanApi.h"
#include "WorkQueue.hpp"
#include "SimpleLogger.hpp"
#include "AvahiDiscoverer.hpp"

static const char *TAG = "MagellanApi";

static Magellan::WorkQueue          g_wq;
static Magellan::SimpleLogger       g_logger;

MAGELLAN_API int magellanSetLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn)
{
    g_logger.setOutputHook(hookFn);    
    return MAGELLAN_RESULT_OK;
}

MAGELLAN_API int magellanInitialize(const char * _Nullable configuration)
{
    int rc = MAGELLAN_RESULT_OK;

    g_logger.d(TAG, "magellanInitialize %s", (configuration == nullptr ? "" : configuration));

    return rc;
}

MAGELLAN_API int magellanShutdown()
{
    int rc = MAGELLAN_RESULT_OK;

    g_logger.d(TAG, "magellanShutdown");
    
    return rc;
}

MAGELLAN_API int magellanBeginDiscovery(const char * _Nullable configuration, MagellanToken_t *pToken, const void * _Nullable userData)
{
    int rc = MAGELLAN_RESULT_OK;

    Magellan::Discoverer    *disco = new Magellan::AvahiDiscoverer();

    *pToken = (MagellanToken_t)disco;

    g_logger.d(TAG, "magellanBeginDiscovery returns %p", (void*) (*pToken));

    return rc;
}

MAGELLAN_API int magellanEndDiscovery(MagellanToken_t token)
{
    int rc = MAGELLAN_RESULT_OK;

    g_logger.d(TAG, "magellanEndDiscovery %p", (void*) token);
    
    return rc;
}

MAGELLAN_API int magellanPauseDiscovery(MagellanToken_t token)
{
    int rc = MAGELLAN_RESULT_OK;

    g_logger.d(TAG, "magellanPauseDiscovery %p", (void*) token);

    Magellan::Discoverer    *disco = (Magellan::Discoverer*)token;
    delete disco;
    
    return rc;
}

MAGELLAN_API int magellanResumeDiscovery(MagellanToken_t token)
{
    int rc = MAGELLAN_RESULT_OK;

    g_logger.d(TAG, "magellanResumeDiscovery %p", (void*) token);
    
    return rc;
}
