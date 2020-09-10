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
                }

                ~DeviceTracker()
                {                
                }

                ProcessingState_t       _ps;
                DeviceConfiguration     _cfg;
        };

        typedef std::map<std::string, Discoverer*> DiscoMap_t;
        typedef std::map<std::string, DeviceTracker> DeviceMap_t;

        static const char *TAG = "MagellanCore";

        static WorkQueue                              m_mainWorkQueue;
        static WorkQueue                              m_downloadWorkQueue;
        static std::atomic<bool>                      m_initialized(false);
        static SimpleLogger                           m_simpleLogger;
        static DiscoMap_t                             m_discos;
        static DeviceMap_t                            m_devices;

        ILogger                                       *logger = &m_simpleLogger;

        ILogger *getLogger()
        {
            return logger;
        }

        int setLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn)
        {
            m_simpleLogger.setOutputHook(hookFn);    
            return MAGELLAN_RESULT_OK;
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
            curl_global_init(CURL_GLOBAL_ALL);

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

            curl_global_cleanup();
            m_downloadWorkQueue.stop();
            m_mainWorkQueue.stop();

            m_initialized = false;
            
            return rc;
        }

        void wq_processDeviceConfiguration(DeviceConfiguration *dc)
        {
            DeviceMap_t::iterator itr = m_devices.find(dc->discovererKey);

            if(itr == m_devices.end())
            {
                getLogger()->e(TAG, "wq_processDeviceConfiguration did not find device '%s'", dc->discovererKey.c_str());
            }
            else
            {
                // TODO: Notify callback that groups are updated
                //DeviceConfiguration previous;

                //previous = itr->second._cfg;


                itr->second._ps = DeviceTracker::psComplete;
                itr->second._cfg = (*dc);
            }            
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
            getLogger()->d(TAG, "write_data: ptr=%p, size=%zu, nmemb=%zu, ctx=%p", ptr, size, nmemb, (void*)ctx);

            std::string tmp;

            tmp.assign((char*)ptr, size*nmemb);
            printf("[%s]\n", tmp.c_str());
            ctx->_ok = ctx->_dc.deserialize(tmp.c_str());

            return (size * nmemb);
        }

        void doUrlDownload(const char *url, const char *discovererKey)
        {
            CURL *curl_handle;
            DeviceConfigurationDownloadCtx *dcctx = new DeviceConfigurationDownloadCtx();

            curl_handle = curl_easy_init();

            curl_easy_setopt(curl_handle, CURLOPT_URL, url);
            curl_easy_setopt(curl_handle, CURLOPT_VERBOSE, 0L);
            curl_easy_setopt(curl_handle, CURLOPT_NOPROGRESS, 1L);

            curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, dcctx);
            curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, curlCbDataToDeviceConfiguration);

            curl_easy_perform(curl_handle);

            curl_easy_cleanup(curl_handle);

            std::string l_discovererKey = discovererKey;

            dcctx->_dc.discovererKey = discovererKey;

            m_mainWorkQueue.submit(([dcctx, l_discovererKey]()
            {            
                if(dcctx->_ok)
                {
                    wq_processDeviceConfiguration(&dcctx->_dc);
                }
                else
                {
                    m_devices.erase(l_discovererKey);
                }
                
                delete dcctx;
            }));
        }

        Discoverer *wq_addDiscoverer(const char *serviceType, PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hookFn, const void *userData)
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

                    getLogger()->i(TAG, "processDiscoveredDevice %s - not found, querying", dd->serialize().c_str());

                    needsProcessing = true;
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
                            getLogger()->i(TAG, "processDiscoveredDevice %s - new version, querying", dd->serialize().c_str());
                        }
                        else
                        {
                            getLogger()->i(TAG, "processDiscoveredDevice %s - query already in progress or completed", dd->serialize().c_str());
                        }                        
                    }
                    else
                    {
                        getLogger()->i(TAG, "processDiscoveredDevice %s - cached version", dd->serialize().c_str());
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
            }));
        }

        void processUndiscoveredDevice(const char *discovererKey)
        {
            getLogger()->i(TAG, "processUndiscoveredDevice %s", discovererKey);

            std::string l_discovererKey = discovererKey;

            m_mainWorkQueue.submit(([l_discovererKey]()
            {
                // TODO: Notify callback that groups are gone

                DeviceMap_t::iterator itrDev = m_devices.find(l_discovererKey);
                if(itrDev != m_devices.end())
                {
                    for( std::vector<Talkgroup>::iterator itrTg = itrDev->second._cfg.talkgroups.begin();
                         itrTg != itrDev->second._cfg.talkgroups.end();
                         itrTg++)
                    {
                        getLogger()->i(TAG, "notify tg '%s' has gone", itrTg->id.c_str());
                    }

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
                Discoverer    *disco = wq_addDiscoverer(l_serviceType.c_str(), hookFn, userData);
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
    }
}
