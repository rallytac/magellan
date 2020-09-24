//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <string.h>

#include "AvahiDiscoverer.hpp"
#include "ILogger.hpp"
#include "MagellanCore.hpp"
#include "MagellanDataModel.hpp"

namespace Magellan
{
    static const char *TAG = "AvahiDiscoverer";

    AvahiDiscoverer::AvahiDiscoverer()
    {
        setImplementation("Avahi-Linux");
        _poller = nullptr;
        _client = nullptr;
        _serviceBrowser = nullptr;
    }

    AvahiDiscoverer::~AvahiDiscoverer()
    {
    }

    void AvahiDiscoverer::deleteThis()
    {
        stop();
        ReferenceCountedObject::deleteThis();
    }

    bool AvahiDiscoverer::configure(DataModel::JsonObjectBase& configuration)
    {
        _configuration = (DataModel::Mdns&)configuration;
        return true;
    }
        
    bool AvahiDiscoverer::start()
    {
        bool rc = false;

        if(_poller == nullptr)
        {
            try
            {
                int err;

                _poller = avahi_threaded_poll_new();
                if(_poller == nullptr)
                {
                    Core::getLogger()->e(TAG, "{%p} avahi_threaded_poll_new failed()", (void*) this);
                    throw "";
                }

                _client = avahi_client_new(avahi_threaded_poll_get(_poller), AVAHI_CLIENT_NO_FAIL, clientCallbackHelper, (void*)this, &err);
                if(_client == nullptr)
                {
                    Core::getLogger()->e(TAG, "{%p} avahi_client_new failed() - %s", (void*) this, avahi_strerror(err));
                    throw "";
                }

                _serviceBrowser = avahi_service_browser_new(_client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, _configuration.serviceType.c_str(), nullptr, (AvahiLookupFlags)0, AvahiDiscoverer::browseCallbackHelper, (void*)this);
                if(_serviceBrowser == nullptr)
                {
                    Core::getLogger()->e(TAG, "{%p} avahi_service_browser_new failed('%s') - %s", (void*) this, _configuration.serviceType.c_str(), avahi_strerror(avahi_client_errno(_client)));
                    throw "";
                }

                if(avahi_threaded_poll_start(_poller) < 0)
                {
                    Core::getLogger()->e(TAG, "{%p} avahi_threaded_poll_start failed('%s') - %s", (void*) this, _configuration.serviceType.c_str(), avahi_strerror(avahi_client_errno(_client)));
                    throw "";
                }

                Core::getLogger()->d(TAG, "{%p} started for '%s'", (void*) this, _configuration.serviceType.c_str());
            }
            catch(...)
            {
                rc = false;
                stop();
            }
        }
        else
        {
            rc = true;
        }

        return rc;
    }

    void AvahiDiscoverer::stop()
    {
        if(_poller != nullptr)
        {
            avahi_threaded_poll_stop(_poller);

            if(_serviceBrowser != nullptr)
            {
                avahi_service_browser_free(_serviceBrowser);
                _serviceBrowser = nullptr;
            }

            if(_client != nullptr)
            {
                avahi_client_free(_client);
                _client = nullptr;
            }

            if(_poller != nullptr)
            {
                avahi_threaded_poll_free(_poller);
                _poller = nullptr;
            }

            Core::getLogger()->d(TAG, "{%p} stopped", (void*) this);
        }
    }

    void AvahiDiscoverer::pause()
    {
        Core::getLogger()->d(TAG, "{%p} paused", (void*) this);
    }

    void AvahiDiscoverer::resume()
    {
        Core::getLogger()->d(TAG, "{%p} resumed", (void*) this);
    }

    /*static*/ void AvahiDiscoverer::clientCallbackHelper(AvahiClient *c, 
                                    AvahiClientState state, 
                                    void *userData)
    {
        ((AvahiDiscoverer*)userData)->clientCallback(c, state);
    }

    void AvahiDiscoverer::clientCallback(AvahiClient *c,
                        AvahiClientState state)
    {      
        if (state == AVAHI_CLIENT_FAILURE) 
        {
            avahi_threaded_poll_quit(_poller);
        }
    }                        

    /*static*/ void AvahiDiscoverer::browseCallbackHelper(AvahiServiceBrowser *b, 
                                        AvahiIfIndex interface,
                                        AvahiProtocol protocol,
                                        AvahiBrowserEvent event,
                                        const char *name,
                                        const char *type,
                                        const char *domain,
                                        AvahiLookupResultFlags flags,
                                        void* userData)
    {
        
        ((AvahiDiscoverer*)userData)->browseCallback(b, interface, protocol, event, name, type, domain, flags);
    }                                        

    void AvahiDiscoverer::browseCallback(AvahiServiceBrowser *b,
                            AvahiIfIndex interface,
                            AvahiProtocol protocol,
                            AvahiBrowserEvent event,
                            const char *name,
                            const char *type,
                            const char *domain,
                            AvahiLookupResultFlags flags)
    {   
        switch(event) 
        {
            case AVAHI_BROWSER_FAILURE:
                Core::getLogger()->e(TAG, "{%p} %s", (void*) this, avahi_strerror(avahi_client_errno(_client)));
                avahi_threaded_poll_quit(_poller);
                break;

            case AVAHI_BROWSER_NEW:
                AvahiServiceResolver *asr;
                
                asr = avahi_service_resolver_new(_client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolveCallbackHelper, (void*) this);
                if(asr != nullptr)
                {
                    Core::getLogger()->d(TAG, "{%p} resolving new service '%s' of type '%s' in domain '%s'", (void*) this, name, type, domain);
                }
                else
                {
                    Core::getLogger()->e(TAG, "{%p} failed to initiate resolving new service '%s' of type '%s' in domain '%s' - %s", (void*) this, name, type, domain, avahi_strerror(avahi_client_errno(_client)));
                }
                break;
                
            case AVAHI_BROWSER_REMOVE:
                Core::getLogger()->d(TAG, "{%p} removed service '%s' of type '%s' in domain '%s'", (void*) this, name, type, domain);

                char buff[1024];
                sprintf(buff, "%s/%s/%s/%s", getImplementation(), _configuration.serviceType.c_str(), domain, name);
                Core::processUndiscoveredDevice(buff);
                break;

            case AVAHI_BROWSER_ALL_FOR_NOW:
            case AVAHI_BROWSER_CACHE_EXHAUSTED:
                //Core::getLogger()->d(TAG, "{%p} %s", (void*) this, event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
                break;
        }
    }

    /*static*/ void AvahiDiscoverer::resolveCallbackHelper(AvahiServiceResolver *r,
                                        AvahiIfIndex interface,
                                        AvahiProtocol protocol,
                                        AvahiResolverEvent event,
                                        const char *name,
                                        const char *type,
                                        const char *domain,
                                        const char *hostName,
                                        const AvahiAddress *address,
                                        uint16_t port,
                                        AvahiStringList *txt,
                                        AvahiLookupResultFlags flags,
                                        void* userData)
    {
        ((AvahiDiscoverer*)userData)->resolveCallback(r, interface, protocol, event, name, type, domain, hostName, address, port, txt, flags);
    }                                          

    void AvahiDiscoverer::resolveCallback(AvahiServiceResolver *r,
                                        AvahiIfIndex interface,
                                        AvahiProtocol protocol,
                                        AvahiResolverEvent event,
                                        const char *name,
                                        const char *type,
                                        const char *domain,
                                        const char *hostName,
                                        const AvahiAddress *address,
                                        uint16_t port,
                                        AvahiStringList *txt,
                                        AvahiLookupResultFlags flags)
    {        
        switch (event) 
        {
            case AVAHI_RESOLVER_FAILURE:
                Core::getLogger()->e(TAG, "{%p} failed to resolve service '%s' of type '%s' in domain '%s': %s", (void*) this, name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
                break;

            case AVAHI_RESOLVER_FOUND: 
            {
                char a[AVAHI_ADDRESS_STR_MAX];
                char *t;
                int filterResponse;

                avahi_address_snprint(a, sizeof(a), address);
                t = avahi_string_list_to_string(txt);

                std::string json;

                json.append("{");
                    json.append("\"serviceType\":\""); json.append(_configuration.serviceType.c_str()); json.append("\"");
                    json.append(",\"implementation\":\""); json.append(getImplementation()); json.append("\"");
                    json.append(",\"name\":\""); json.append(name); json.append("\"");
                    json.append(",\"hostName\":\""); json.append(hostName); json.append("\"");
                json.append("}");

                filterResponse = callFilterHook(json.c_str());

                if(filterResponse == MAGELLAN_FILTER_PROCEED)
                {
                    DataModel::DiscoveredDevice    *dd = new DataModel::DiscoveredDevice();

                    AvahiStringList *curr = txt;
                    char buff[1024];

                    sprintf(buff, "%s/%s/%s/%s", getImplementation(), _configuration.serviceType.c_str(), domain, name);
                    dd->discovererKey.assign(buff);

                    while( curr != nullptr )
                    {
                        if(curr->size > 0 && curr->size < sizeof(buff))
                        {
                            memcpy(buff, curr->text, curr->size);
                            buff[curr->size] = 0;

                            if(strncmp(buff, "id=", 3) == 0)
                            {
                                dd->id.assign(buff + 3);
                            }
                            else if(strncmp(buff, "cv=", 3) == 0)
                            {
                                dd->configVersion = atoi(buff + 3);
                            }
                        }
                        
                        curr = curr->next;
                    }

                    // Note, HTTPS is assumed
                    if(port > 0)
                    {
                        sprintf(buff, "https://%s:%d/config", hostName, port);
                    }
                    else
                    {
                        sprintf(buff, "https://%s/config", hostName);
                    }
                    
                    dd->rootUrl.assign(buff);

                    Core::processDiscoveredDevice(dd);
                }
                                    
                avahi_free(t);
            }
        }

        avahi_service_resolver_free(r);
    }
}
