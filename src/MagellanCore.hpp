//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef MAGELLANCORE_HPP
#define MAGELLANCORE_HPP

#include "MagellanTypes.h"
#include "MagellanConstants.h"
#include "MagellanDataModel.hpp"
#include "SimpleLogger.hpp"
#include "Discoverer.hpp"

namespace Magellan
{       
    namespace Core
    {
        ILogger *getLogger();
        int setLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn);
        uint64_t getNowMs();

        int initialize(const char *configuration);
        int shutdown();

        int beginDiscovery(const char *serviceType, MagellanToken_t *pToken, PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hookFn, const void *userData);
        int endDiscovery(MagellanToken_t token);
        int pauseDiscovery(MagellanToken_t token);
        int resumeDiscovery(MagellanToken_t token);

        void processDiscoveredDevice(DataModel::DiscoveredDevice *dd);
        void processUndiscoveredDevice(const char *discovererKey);

        void setTalkgroupCallbacks(PFN_MAGELLAN_ON_NEW_TALKGROUPS pfnOnNewTalkgroups,
                                   PFN_MAGELLAN_ON_MODIFIED_TALKGROUPS pfnOnModifiedTalkgroups,
                                   PFN_MAGELLAN_ON_REMOVED_TALKGROUPS pfnOnRemovedTalkgroups,
                                   const void *userData);
    }
}
#endif
