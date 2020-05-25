//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include "AvahiDiscoverer.hpp"
#include "ILogger.hpp"

namespace Magellan
{
    static const char *TAG = "AvahiDiscoverer";
    extern ILogger *logger;

    AvahiDiscoverer::AvahiDiscoverer()
    {
        logger->i(TAG, "{%p} ctor()", (void*) this);
        _poller = nullptr;
        _client = nullptr;
        _serviceBrowser = nullptr;
    }

    AvahiDiscoverer::~AvahiDiscoverer()
    {
        logger->i(TAG, "{%p} ~dtor()", (void*) this);
    }

    void AvahiDiscoverer::deleteThis()
    {
        stop();
        ReferenceCountedObject::deleteThis();
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
                    logger->e(TAG, "{%p} avahi_threaded_poll_new failed()", (void*) this);
                    throw "";
                }

                _client = avahi_client_new(avahi_threaded_poll_get(_poller), AVAHI_CLIENT_NO_FAIL, clientCallbackHelper, (void*)this, &err);
                if(_client == nullptr)
                {
                    logger->e(TAG, "{%p} avahi_client_new failed() - %s", (void*) this, avahi_strerror(err));
                    throw "";
                }

                _serviceBrowser = avahi_service_browser_new(_client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, getServiceType(), nullptr, (AvahiLookupFlags)0, AvahiDiscoverer::browseCallbackHelper, (void*)this);
                if(_serviceBrowser == nullptr)
                {
                    logger->e(TAG, "{%p} avahi_service_browser_new failed('%s') - %s", (void*) this, getServiceType(), avahi_strerror(avahi_client_errno(_client)));
                    throw "";
                }

                if(avahi_threaded_poll_start(_poller) < 0)
                {
                    logger->e(TAG, "{%p} avahi_threaded_poll_start failed('%s') - %s", (void*) this, getServiceType(), avahi_strerror(avahi_client_errno(_client)));
                    throw "";
                }

                logger->d(TAG, "{%p} started for '%s'", (void*) this, getServiceType());
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

            logger->d(TAG, "{%p} stopped", (void*) this);
        }
    }

    void AvahiDiscoverer::pause()
    {
        logger->d(TAG, "{%p} paused", (void*) this);
    }

    void AvahiDiscoverer::resume()
    {
        logger->d(TAG, "{%p} resumed", (void*) this);
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
                logger->e(TAG, "{%p} %s", (void*) this, avahi_strerror(avahi_client_errno(_client)));
                avahi_threaded_poll_quit(_poller);
                break;

            case AVAHI_BROWSER_NEW:
                AvahiServiceResolver *asr;
                
                asr = avahi_service_resolver_new(_client, interface, protocol, name, type, domain, AVAHI_PROTO_UNSPEC, (AvahiLookupFlags)0, resolveCallbackHelper, (void*) this);
                if(asr != nullptr)
                {
                    logger->d(TAG, "{%p} resolving new service '%s' of type '%s' in domain '%s'", (void*) this, name, type, domain);
                }
                else
                {
                    logger->e(TAG, "{%p} failed to initiate resolving new service '%s' of type '%s' in domain '%s' - %s", (void*) this, name, type, domain, avahi_strerror(avahi_client_errno(_client)));
                }
                break;
                
            case AVAHI_BROWSER_REMOVE:
                logger->d(TAG, "{%p} removed service '%s' of type '%s' in domain '%s'", (void*) this, name, type, domain);
                break;

            case AVAHI_BROWSER_ALL_FOR_NOW:
            case AVAHI_BROWSER_CACHE_EXHAUSTED:
                //logger->d(TAG, "{%p} %s", (void*) this, event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
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
                logger->e(TAG, "{%p} failed to resolve service '%s' of type '%s' in domain '%s': %s", (void*) this, name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
                break;

            case AVAHI_RESOLVER_FOUND: 
            {
                char a[AVAHI_ADDRESS_STR_MAX];
                char *t;

                avahi_address_snprint(a, sizeof(a), address);
                t = avahi_string_list_to_string(txt);

                logger->d(TAG, "{%p} resolved service '%s' of type '%s' in domain '%s': "
                                    "%s:%u (%s)"
                                    ", TXT=%s"
                                    ", cookie is %u"
                                    ", is_local: %i"
                                    ", our_own: %i"
                                    ", wide_area: %i"
                                    ", multicast: %i"
                                    ", cached: %i",
                                (void*) this, name, type, domain,
                                hostName, port, a,
                                t,
                                avahi_string_list_get_service_cookie(txt),
                                !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
                                !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
                                !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
                                !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
                                !!(flags & AVAHI_LOOKUP_RESULT_CACHED));
                    
                avahi_free(t);
            }
        }

        avahi_service_resolver_free(r);
    }
}
