//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <mutex>
#include <map>
#include <string>
#include <string.h>
#include <atomic>
#include <curl/curl.h>

#include "MagellanCore.hpp"
#include "MagellanJson.hpp"
#include "WorkQueue.hpp"
#include "TimerManager.hpp"
#include "AppDiscoverer.hpp"
#include "AvahiDiscoverer.hpp"

namespace Magellan
{
    namespace Core
    {
        class DeviceTracker
        {
            public:
                typedef enum
                {
                    psRequired,
                    psInProgress,
                    psComplete
                } ProcessingState_t;

                DeviceTracker()
                {          
                    _ps = psRequired;
                    _nextCheckTs = 0;
                    _consecutiveErrors = 0;
                }

                ~DeviceTracker()
                {                
                }

                std::string             _key;
                std::string             _url;
                ProcessingState_t       _ps;
                DeviceConfiguration     _cfg;
                uint64_t                _nextCheckTs;
                uint64_t                _consecutiveErrors;
        };

        typedef std::map<std::string, Discoverer*> DiscoMap_t;
        typedef std::map<std::string, DeviceTracker> DeviceMap_t;

        static const char *TAG = "MagellanCore";

        static const uint64_t HOUSEKEEPER_INTERVAL_MS = 5000;
        static const uint64_t URLCHECKER_INTERVAL_MS = 2500;
        static const uint64_t URL_RETRY_INTERVAL_MS = 5000;
        static const uint64_t MAX_CONSECUTIVE_PROCESSING_ERRORS = 50;
        
        static WorkQueue                                m_mainWorkQueue;
        static WorkQueue                                m_downloadWorkQueue;
        static std::atomic<bool>                        m_initialized(false);
        static SimpleLogger                             m_simpleLogger;
        static DiscoMap_t                               m_discos;
        static DeviceMap_t                              m_devices;
        static TimerManager                             m_timerManager;


        static PFN_MAGELLAN_ON_NEW_TALKGROUPS           m_pfnOnNewTalkgroups = nullptr;
        static PFN_MAGELLAN_ON_MODIFIED_TALKGROUPS      m_pfnOnModifiedTalkgroups = nullptr;             
        static PFN_MAGELLAN_ON_REMOVED_TALKGROUPS       m_pfnOnRemovedTalkgroups = nullptr;
        static const void                               *m_pfOnTgUserData = nullptr;

        static uint64_t                                 m_tmrHouseKeeper = 0;
        static uint64_t                                 m_tmrUrlChecker = 0;

        void doUrlDownload(const char *url, const char *discovererKey);

        uint64_t getNowMs()
        {
            return static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())
                                        .time_since_epoch()
                                        .count());
        }        

        ILogger *getLogger()
        {
            return &m_simpleLogger;
        }

        int setLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn)
        {
            m_simpleLogger.setOutputHook(hookFn);    
            return MAGELLAN_RESULT_OK;
        }

        void performHousekeeping()
        {
            getLogger()->d(TAG, "performHousekeeping");
        }

        void performUrlChecking()
        {
            getLogger()->d(TAG, "performUrlChecking");

            uint64_t now = getNowMs();

            for(DeviceMap_t::iterator itr = m_devices.begin();
                itr != m_devices.end();
                itr++)
            {
                DeviceTracker *dt = &itr->second;
                if(dt->_ps == DeviceTracker::psRequired)
                {
                    if(dt->_nextCheckTs > 0 && dt->_nextCheckTs <= now)
                    {
                        dt->_ps = DeviceTracker::psInProgress;
                        dt->_nextCheckTs = 0;

                        std::string l_url = dt->_url.c_str();
                        std::string l_key = dt->_key.c_str();
                        m_downloadWorkQueue.submit(([l_url, l_key]()
                        {            
                            doUrlDownload(l_url.c_str(), l_key.c_str());
                        }));
                    }
                }
            }
        }

        bool tmrCbHouseKeeper(uint64_t hnd, const void *ctx)
        {
            m_mainWorkQueue.submit(([]()
            {
                performHousekeeping();
            }));

            return true;
        }

        bool tmrCbUrlChecker(uint64_t hnd, const void *ctx)
        {
            m_mainWorkQueue.submit(([]()
            {
                performUrlChecking();
            }));

            return true;
        }

        int initialize(const char *configuration)
        {
            int rc = MAGELLAN_RESULT_OK;

            if(m_initialized)
            {
                return rc;
            }

            m_initialized = true;
            getLogger()->d(TAG, "magellanInitialize %s", (configuration == nullptr ? "" : configuration));

            m_mainWorkQueue.start();
            m_downloadWorkQueue.start();
            m_timerManager.start();
            curl_global_init(CURL_GLOBAL_ALL);

            m_tmrHouseKeeper = m_timerManager.setTimer(tmrCbHouseKeeper, nullptr, HOUSEKEEPER_INTERVAL_MS, true);
            m_tmrUrlChecker = m_timerManager.setTimer(tmrCbUrlChecker, nullptr, URLCHECKER_INTERVAL_MS, true);

            return rc;
        }

        int shutdown()
        {
            int rc = MAGELLAN_RESULT_OK;

            if(!m_initialized)
            {
                return rc;
            }

            getLogger()->d(TAG, "magellanShutdown");

            m_timerManager.cancelTimer(m_tmrHouseKeeper);
            m_tmrHouseKeeper = 0;

            m_timerManager.cancelTimer(m_tmrUrlChecker);
            m_tmrUrlChecker = 0;

            m_timerManager.stop();

            curl_global_cleanup();
            m_downloadWorkQueue.stop();
            m_mainWorkQueue.stop();

            m_initialized = false;
            
            return rc;
        }

        Talkgroup *getTalkgroup(const char *id, std::vector<Talkgroup>& v)
        {
            for(std::vector<Talkgroup>::iterator itrSearch = v.begin();
                itrSearch != v.end();
                itrSearch++)
            {
                if(itrSearch->id.compare(id) == 0)
                {
                    return &(*itrSearch);
                }
            }

            return nullptr;
        }

        void notifyOfLostDevice(DeviceTracker *dt)
        {
            if(!dt->_cfg.talkgroups.empty() && m_pfnOnRemovedTalkgroups != nullptr)
            {
                nlohmann::json idArray = nlohmann::json::array();

                for( std::vector<Talkgroup>::iterator itrTg = dt->_cfg.talkgroups.begin();
                    itrTg != dt->_cfg.talkgroups.end();
                    itrTg++)
                {
                    getLogger()->d(TAG, "notify tg '%s' has gone", itrTg->id.c_str());
                    idArray.push_back(itrTg->id.c_str());
                }

                m_pfnOnRemovedTalkgroups(idArray.dump().c_str(), m_pfOnTgUserData);
            }
        }


        void processDeviceConfiguration(const char *discovererKey, DeviceTracker *dt, DeviceConfiguration *dc, bool encounteredError)
        {
            // Did we encounter an error?
            if(encounteredError)
            {
                dt->_consecutiveErrors++;

                if(dt->_consecutiveErrors >= MAX_CONSECUTIVE_PROCESSING_ERRORS)
                {
                    getLogger()->e(TAG, "too many consecutive errors on %s - abandoning", discovererKey);
                    notifyOfLostDevice(dt);
                    m_devices.erase(discovererKey);
                }
                else
                {
                    uint64_t now = getNowMs();
                    uint64_t rndAmount = (rand() % (dt->_consecutiveErrors * URL_RETRY_INTERVAL_MS));

                    dt->_nextCheckTs = ((now + URL_RETRY_INTERVAL_MS) + rndAmount);
                    dt->_ps = DeviceTracker::psRequired;
                    getLogger()->e(TAG, "scheduled next check of %s in %" PRIu64 " milliseconds", discovererKey, (dt->_nextCheckTs - now));
                }
                
                return;
            }

            // Reset these
            dt->_consecutiveErrors = 0;
            dt->_nextCheckTs = 0;
            

            std::vector<std::string>     newTalkGroups;
            std::vector<std::string>     modifiedTalkGroups;
            std::vector<std::string>     removedTalkGroups;

            // Look for new or modified
            for(std::vector<Talkgroup>::iterator itrIncoming = dc->talkgroups.begin();
                itrIncoming != dc->talkgroups.end();
                itrIncoming++)
            {
                Talkgroup *tg = getTalkgroup(itrIncoming->id.c_str(), dt->_cfg.talkgroups);
                if(tg != nullptr)
                {
                    if(!itrIncoming->matches(*tg))
                    {
                        modifiedTalkGroups.push_back(itrIncoming->id);
                    }
                }
                else
                {
                    newTalkGroups.push_back(itrIncoming->id);
                }
            }

            // Look for removed
            for(std::vector<Talkgroup>::iterator itrExisting = dt->_cfg.talkgroups.begin();
                itrExisting != dt->_cfg.talkgroups.end();
                itrExisting++)
            {
                Talkgroup *tg = getTalkgroup(itrExisting->id.c_str(), dc->talkgroups);
                if(tg == nullptr)
                {
                    removedTalkGroups.push_back(itrExisting->id);
                }
            }
            
            // Notify for removals
            if(!removedTalkGroups.empty() && m_pfnOnRemovedTalkgroups != nullptr)
            {
                nlohmann::json idArray = nlohmann::json::array();

                for(std::vector<std::string>::iterator itrNotify = removedTalkGroups.begin();
                    itrNotify != removedTalkGroups.end();
                    itrNotify++)
                {
                    Talkgroup *tg = getTalkgroup(itrNotify->c_str(), dt->_cfg.talkgroups);
                    if(tg != nullptr)
                    {
                        getLogger()->d(TAG, "notify of removed tg '%s'", itrNotify->c_str());

                        idArray.push_back(itrNotify->c_str());
                    }
                    else
                    {
                        getLogger()->e(TAG, "cannot find tg '%s' in existing configuration", itrNotify->c_str());
                    }
                }

                m_pfnOnRemovedTalkgroups(idArray.dump().c_str(), m_pfOnTgUserData);
            }

            // Notify for updates
            if(!modifiedTalkGroups.empty() && m_pfnOnModifiedTalkgroups != nullptr)
            {
                nlohmann::json tgArray = nlohmann::json::array();

                for(std::vector<std::string>::iterator itrNotify = modifiedTalkGroups.begin();
                    itrNotify != modifiedTalkGroups.end();
                    itrNotify++)
                {
                    Talkgroup *tg = getTalkgroup(itrNotify->c_str(), dc->talkgroups);
                    if(tg != nullptr)
                    {
                        getLogger()->d(TAG, "notify of modified tg '%s'", itrNotify->c_str());

                        tgArray.push_back((*tg));
                    }
                    else
                    {
                        getLogger()->e(TAG, "cannot find modified tg '%s' in new configuration", itrNotify->c_str());
                    }
                }

                m_pfnOnModifiedTalkgroups(tgArray.dump().c_str(), m_pfOnTgUserData);
            }

            // Notify for additions
            if(!newTalkGroups.empty() && m_pfnOnModifiedTalkgroups != nullptr)
            {
                nlohmann::json tgArray = nlohmann::json::array();

                for(std::vector<std::string>::iterator itrNotify = newTalkGroups.begin();
                    itrNotify != newTalkGroups.end();
                    itrNotify++)
                {
                    Talkgroup *tg = getTalkgroup(itrNotify->c_str(), dc->talkgroups);
                    if(tg != nullptr)
                    {
                        getLogger()->d(TAG, "notify of new tg '%s'", itrNotify->c_str());

                        tgArray.push_back((*tg));
                    }
                    else
                    {
                        getLogger()->e(TAG, "cannot find new tg '%s' in new configuration", itrNotify->c_str());
                    }
                }

                m_pfnOnModifiedTalkgroups(tgArray.dump().c_str(), m_pfOnTgUserData);
            }

            // Update our cached configuration
            dt->_ps = DeviceTracker::psComplete;
            dt->_cfg = (*dc);
        }

        class DeviceConfigurationDownloadCtx
        {
            public:
                DeviceConfigurationDownloadCtx()
                {
                    _ok = false;
                }

                bool                    _ok;
                DeviceConfiguration     _dc;
        };

        static size_t curlCbDataToDeviceConfiguration(void *ptr, size_t size, size_t nmemb, void *userData)
        {
            DeviceConfigurationDownloadCtx *ctx = (DeviceConfigurationDownloadCtx*)userData;
            //getLogger()->d(TAG, "curlCbDataToDeviceConfiguration: ptr=%p, size=%zu, nmemb=%zu, ctx=%p", ptr, size, nmemb, (void*)ctx);

            std::string tmp;

            tmp.assign((char*)ptr, size*nmemb);
            ctx->_ok = ctx->_dc.deserialize(tmp.c_str());

            return (size * nmemb);
        }

        void doUrlDownload(const char *url, const char *discovererKey)
        {
            getLogger()->d(TAG, "doUrlDownload from %s for %s", url, discovererKey);

            CURL *curl_handle;
            CURLcode cc;

            DeviceConfigurationDownloadCtx *dcctx = new DeviceConfigurationDownloadCtx();

            curl_handle = curl_easy_init();

            curl_easy_setopt(curl_handle, CURLOPT_URL, url);
            curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
            curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, dcctx);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlCbDataToDeviceConfiguration);

            cc = curl_easy_perform(curl_handle);

            curl_easy_cleanup(curl_handle);

            std::string l_discovererKey = discovererKey;

            dcctx->_dc.discovererKey = discovererKey;

            m_mainWorkQueue.submit(([cc, dcctx, l_discovererKey]()
            {            
                DeviceMap_t::iterator itr = m_devices.find(l_discovererKey);
                if(itr != m_devices.end())
                {
                    DeviceTracker *dt = &itr->second;

                    if(cc != CURLE_OK)
                    {
                        getLogger()->e(TAG, "curl error %d (%s) for device %s", (int)cc, curl_easy_strerror(cc), l_discovererKey.c_str());
                    }

                    processDeviceConfiguration(l_discovererKey.c_str(), dt, &dcctx->_dc, (cc == CURLE_OK) ? false : true);
                }
                else
                {
                    getLogger()->e(TAG, "did not find device '%s' after configuration download", l_discovererKey.c_str());
                }                

                delete dcctx;
            }));
        }

        Discoverer *addDiscoverer(const char *serviceType, PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hookFn, const void *userData)
        {
            Discoverer *disco = nullptr;

            DiscoMap_t::iterator itr = m_discos.find(serviceType);
            if(itr == m_discos.end())
            {
                
                if(strcmp(serviceType, MAGELLAN_APP_SERVICE_TYPE) == 0 )
                {

                }
                else
                {
                    disco = new AvahiDiscoverer();
                }

                disco->setServiceType(serviceType);
                disco->setHook(hookFn);
                disco->setUserData(userData);
            }
            else
            {
                disco->addReference();
            }

            return disco;
        }

        void processDiscoveredDevice(DiscoveredDevice *dd)
        {
            m_mainWorkQueue.submit(([dd]()
            {
                bool needsProcessing = false;
                DeviceMap_t::iterator itr = m_devices.find(dd->discovererKey);

                if(itr == m_devices.end())
                {
                    DeviceTracker   dt;

                    getLogger()->d(TAG, "processDiscoveredDevice %s - not found, querying", dd->serialize().c_str());

                    needsProcessing = true;
                    dt._key = dd->discovererKey;
                    dt._url = dd->rootUrl;
                    dt._ps = DeviceTracker::psInProgress;
                    m_devices[dd->discovererKey] = dt;
                }
                else
                {
                    if(itr->second._cfg.version != dd->configVersion)
                    {
                        if(itr->second._ps != DeviceTracker::psInProgress && itr->second._ps != DeviceTracker::psComplete)
                        {
                            needsProcessing = true;
                            itr->second._ps = DeviceTracker::psInProgress;
                            getLogger()->d(TAG, "processDiscoveredDevice %s - new version, querying", dd->serialize().c_str());
                        }
                        else
                        {
                            getLogger()->d(TAG, "processDiscoveredDevice %s - query already in progress or completed", dd->serialize().c_str());
                        }                        
                    }
                    else
                    {
                        getLogger()->d(TAG, "processDiscoveredDevice %s - cached version", dd->serialize().c_str());
                    }
                }

                if(needsProcessing)
                {
                    m_downloadWorkQueue.submit(([dd]()
                    {            
                        doUrlDownload(dd->rootUrl.c_str(), dd->discovererKey.c_str());
                        delete dd;
                    }));
                }
                else
                {
                    delete dd;
                }                
            }));
        }

        void processUndiscoveredDevice(const char *discovererKey)
        {
            getLogger()->d(TAG, "processUndiscoveredDevice %s", discovererKey);

            std::string l_discovererKey = discovererKey;

            m_mainWorkQueue.submit(([l_discovererKey]()
            {
                DeviceMap_t::iterator itrDev = m_devices.find(l_discovererKey);
                if(itrDev != m_devices.end())
                {
                    notifyOfLostDevice(&itrDev->second);
                    m_devices.erase(itrDev);
                }
            }));
        }

        int beginDiscovery(const char *serviceType, MagellanToken_t *pToken, PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hookFn, const void *userData)
        {
            int rc = MAGELLAN_RESULT_OK;

            // Use the default Magellan service type if not specified
            std::string l_serviceType = ((serviceType != nullptr && serviceType[0] != 0) ? serviceType : MAGELLAN_DEFAULT_SERVICE_TYPE);

            m_mainWorkQueue.submitAndWait(([l_serviceType, pToken, hookFn, userData]()
            {            
                Discoverer    *disco = addDiscoverer(l_serviceType.c_str(), hookFn, userData);
                *pToken = (MagellanToken_t)disco;
                disco->start();
            }));


            getLogger()->d(TAG, "magellanBeginDiscovery returns %p", (void*) (*pToken));

            return rc;
        }

        int endDiscovery(MagellanToken_t token)
        {
            int rc = MAGELLAN_RESULT_OK;

            getLogger()->d(TAG, "magellanEndDiscovery %p", (void*) token);

            m_mainWorkQueue.submit(([token]()
            {
                ((Discoverer*)token)->releaseReference();
            }));
            
            return rc;
        }

        int pauseDiscovery(MagellanToken_t token)
        {
            int rc = MAGELLAN_RESULT_OK;

            getLogger()->d(TAG, "magellanPauseDiscovery %p", (void*) token);

            m_mainWorkQueue.submit(([token]()
            {
                ((Discoverer*)token)->pause();
            }));

            return rc;
        }

        int resumeDiscovery(MagellanToken_t token)
        {
            int rc = MAGELLAN_RESULT_OK;

            getLogger()->d(TAG, "magellanResumeDiscovery %p", (void*) token);

            m_mainWorkQueue.submit(([token]()
            {
                ((Discoverer*)token)->resume();
            }));
            
            return rc;
        }

        void setTalkgroupCallbacks(PFN_MAGELLAN_ON_NEW_TALKGROUPS pfnOnNewTalkgroups,
                                   PFN_MAGELLAN_ON_MODIFIED_TALKGROUPS pfnOnModifiedTalkgroups,
                                   PFN_MAGELLAN_ON_REMOVED_TALKGROUPS pfnOnRemovedTalkgroups,
                                   const void *userData)
        {
            m_mainWorkQueue.submit(([pfnOnNewTalkgroups,
                                     pfnOnModifiedTalkgroups,
                                     pfnOnRemovedTalkgroups,
                                     userData]()
            {
                m_pfnOnNewTalkgroups = pfnOnNewTalkgroups;
                m_pfnOnModifiedTalkgroups = pfnOnModifiedTalkgroups;
                m_pfnOnRemovedTalkgroups = pfnOnRemovedTalkgroups;
                m_pfOnTgUserData = userData;
            }));
        }
    }
}
