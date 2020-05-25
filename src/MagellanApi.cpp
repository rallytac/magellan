//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <mutex>
#include <map>
#include <string>

#include "MagellanApi.h"
#include "WorkQueue.hpp"
#include "SimpleLogger.hpp"
#include "AvahiDiscoverer.hpp"

namespace Magellan
{
    typedef std::map<std::string, Magellan::Discoverer*> DiscoMap_t;

    static const char *TAG = "MagellanApi";

    static Magellan::WorkQueue                              m_wq;
    static Magellan::SimpleLogger                           m_simpleLogger;
    static std::mutex                                       m_discosLock;
    static DiscoMap_t                                       m_discos;

    ILogger                                                 *logger = &m_simpleLogger;

    int setLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn)
    {
        m_simpleLogger.setOutputHook(hookFn);    
        return MAGELLAN_RESULT_OK;
    }

    Magellan::Discoverer *addDiscoverer(const char *serviceType, PFN_MAGELLAN_ASSET_DISCOVERY_HOOK hookFn, const void * _Nullable userData)
    {
        Magellan::Discoverer *disco = nullptr;

        m_discosLock.lock();
        {
            DiscoMap_t::iterator itr = m_discos.find(serviceType);
            if(itr == m_discos.end())
            {
                disco = new AvahiDiscoverer();
                disco->setServiceType(serviceType);
            }
            else
            {
                disco->addReference();
            }
        }
        m_discosLock.unlock();

        return disco;
    }

    int initialize(const char * _Nullable configuration)
    {
        int rc = MAGELLAN_RESULT_OK;

        logger->d(TAG, "magellanInitialize %s", (configuration == nullptr ? "" : configuration));

        return rc;
    }

    int shutdown()
    {
        int rc = MAGELLAN_RESULT_OK;

        logger->d(TAG, "magellanShutdown");
        
        return rc;
    }

    int beginDiscovery(const char * _Nullable serviceType, MagellanToken_t *pToken, PFN_MAGELLAN_ASSET_DISCOVERY_HOOK hookFn, const void * _Nullable userData)
    {
        int rc = MAGELLAN_RESULT_OK;

        // Use the default Magellan service type if not specified
        if(serviceType == nullptr || serviceType[0] == 0)
        {
            serviceType = MAGELLAN_DEFAULT_SERVICE_TYPE;
        }

        Magellan::Discoverer    *disco = addDiscoverer(serviceType, hookFn, userData);
        *pToken = (MagellanToken_t)disco;
        disco->start();

        logger->d(TAG, "magellanBeginDiscovery returns %p", (void*) (*pToken));

        return rc;
    }

    int endDiscovery(MagellanToken_t token)
    {
        int rc = MAGELLAN_RESULT_OK;

        logger->d(TAG, "magellanEndDiscovery %p", (void*) token);

        Magellan::Discoverer    *disco = (Magellan::Discoverer*)token;
        disco->releaseReference();
        
        return rc;
    }

    int pauseDiscovery(MagellanToken_t token)
    {
        int rc = MAGELLAN_RESULT_OK;

        logger->d(TAG, "magellanPauseDiscovery %p", (void*) token);

        Magellan::Discoverer    *disco = (Magellan::Discoverer*)token;
        disco->pause();
        
        return rc;
    }

    int resumeDiscovery(MagellanToken_t token)
    {
        int rc = MAGELLAN_RESULT_OK;

        logger->d(TAG, "magellanResumeDiscovery %p", (void*) token);

        Magellan::Discoverer    *disco = (Magellan::Discoverer*)token;
        disco->resume();
        
        return rc;
    }
}

// Exposed C API
MAGELLAN_API int magellanSetLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn)
{
    return Magellan::setLoggingHook(hookFn);
}

MAGELLAN_API int magellanInitialize(const char * _Nullable configuration)
{
    return Magellan::initialize(configuration);
}

MAGELLAN_API int magellanShutdown()
{
    return Magellan::shutdown();
}

MAGELLAN_API int magellanBeginDiscovery(const char * _Nullable serviceType, MagellanToken_t *pToken, PFN_MAGELLAN_ASSET_DISCOVERY_HOOK hookFn, const void * _Nullable userData)
{
    return Magellan::beginDiscovery(serviceType, pToken, hookFn, userData);
}

MAGELLAN_API int magellanEndDiscovery(MagellanToken_t token)
{
    return Magellan::endDiscovery(token);
}

MAGELLAN_API int magellanPauseDiscovery(MagellanToken_t token)
{
    return Magellan::pauseDiscovery(token);
}

MAGELLAN_API int magellanResumeDiscovery(MagellanToken_t token)
{
    return Magellan::resumeDiscovery(token);
}
