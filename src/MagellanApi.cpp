//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <mutex>
#include <map>
#include <string>
#include <string.h>
#include <atomic>

#include "MagellanApi.h"
#include "MagellanCore.hpp"

// Exposed C API
MAGELLAN_API int magellanDevTest()
{
    return 0;
}

MAGELLAN_API int magellanSetLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn)
{
    return Magellan::Core::setLoggingHook(hookFn);
}

MAGELLAN_API int magellanInitialize(const char * _Nullable configuration)
{
    return Magellan::Core::initialize(configuration);
}

MAGELLAN_API int magellanShutdown()
{
    return Magellan::Core::shutdown();
}

MAGELLAN_API void magellanLogMessage(int level, const char * _Nonnull tag, const char * _Nonnull msg)
{
    switch(level)
    {
        case MAGELLAN_LOG_LEVEL_FATAL:
            Magellan::Core::getLogger()->f(tag, "%s", msg);
            break;

        case MAGELLAN_LOG_LEVEL_ERROR:
            Magellan::Core::getLogger()->e(tag, "%s", msg);
            break;

        case MAGELLAN_LOG_LEVEL_WARNING:
            Magellan::Core::getLogger()->w(tag, "%s", msg);
            break;

        case MAGELLAN_LOG_LEVEL_INFORMATIONAL:
            Magellan::Core::getLogger()->i(tag, "%s", msg);
            break;

        case MAGELLAN_LOG_LEVEL_DEBUG:
        default:
            Magellan::Core::getLogger()->d(tag, "%s", msg);
            break;
    }
}

MAGELLAN_API int magellanBeginDiscovery(const char * _Nullable serviceType, MagellanToken_t *pToken, PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hookFn, const void * _Nullable userData)
{
    return Magellan::Core::beginDiscovery(serviceType, pToken, hookFn, userData);
}

MAGELLAN_API int magellanEndDiscovery(MagellanToken_t token)
{
    return Magellan::Core::endDiscovery(token);
}

MAGELLAN_API int magellanPauseDiscovery(MagellanToken_t token)
{
    return Magellan::Core::pauseDiscovery(token);
}

MAGELLAN_API int magellanResumeDiscovery(MagellanToken_t token)
{
    return Magellan::Core::resumeDiscovery(token);
}

MAGELLAN_API void magellanSetTalkgroupCallbacks(PFN_MAGELLAN_ON_NEW_TALKGROUPS pfnOnNewTalkgroups,
                            PFN_MAGELLAN_ON_MODIFIED_TALKGROUPS pfnOnModifiedTalkgroups,
                            PFN_MAGELLAN_ON_REMOVED_TALKGROUPS pfnOnRemovedTalkgroups,
                            const void *userData)
{
    Magellan::Core::setTalkgroupCallbacks(pfnOnNewTalkgroups,
                                          pfnOnModifiedTalkgroups,
                                          pfnOnRemovedTalkgroups,
                                          userData);
}
