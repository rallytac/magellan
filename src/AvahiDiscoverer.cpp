//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include "AvahiDiscoverer.hpp"

namespace Magellan
{
    static const char *TAG = "AvahiDiscoverer";

    AvahiDiscoverer::AvahiDiscoverer()
    {
        _pollerLoop = nullptr;
        _client = nullptr;
        _serviceBrowser = nullptr;
    }

    AvahiDiscoverer::~AvahiDiscoverer()
    {
        stop();
    }
        
    bool AvahiDiscoverer::start(const char *serviceType)
    {
        bool rc = false;

        try
        {
            int err;

            _pollerLoop = avahi_simple_poll_new();
            if(_pollerLoop == nullptr)
            {
                throw "";
            }

            _client = avahi_client_new(avahi_simple_poll_get(_pollerLoop), AVAHI_CLIENT_NO_FAIL, clientCallbackHelper, (void*)this, &err);
            if(_client == nullptr)
            {
                throw "";
            }

            _serviceBrowser = avahi_service_browser_new(_client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, serviceType, nullptr, (AvahiLookupFlags)0, browseCallbackHelper, _client);
            if(_serviceBrowser == nullptr)
            {
                throw "";
            }

            avahi_simple_poll_loop(_pollerLoop);
        }
        catch(...)
        {
            rc = false;
            stop();
        }

        return rc;
    }

    void AvahiDiscoverer::stop()
    {
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

        if(_pollerLoop != nullptr)
        {
            avahi_simple_poll_free(_pollerLoop);
            _pollerLoop = nullptr;
        }
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
            avahi_simple_poll_quit(_pollerLoop);
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
                getLogger()->e(TAG, "(Browser) %s", avahi_strerror(avahi_client_errno(_client)));
                avahi_simple_poll_quit(_pollerLoop);
                break;

            case AVAHI_BROWSER_NEW:
                getLogger()->d(TAG, "(Browser) NEW: service '%s' of type '%s' in domain '%s'", name, type, domain);
                break;
                
            case AVAHI_BROWSER_REMOVE:
                getLogger()->d(TAG, "(Browser) REMOVE: service '%s' of type '%s' in domain '%s'", name, type, domain);
                break;

            case AVAHI_BROWSER_ALL_FOR_NOW:
            case AVAHI_BROWSER_CACHE_EXHAUSTED:
                getLogger()->d(TAG, "(Browser) %s", event == AVAHI_BROWSER_CACHE_EXHAUSTED ? "CACHE_EXHAUSTED" : "ALL_FOR_NOW");
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
                getLogger()->e(TAG, "(Resolver) Failed to resolve service '%s' of type '%s' in domain '%s': %s", name, type, domain, avahi_strerror(avahi_client_errno(avahi_service_resolver_get_client(r))));
                break;

            case AVAHI_RESOLVER_FOUND: 
            {
                char a[AVAHI_ADDRESS_STR_MAX];
                char *t;

                getLogger()->d(TAG, "Service '%s' of type '%s' in domain '%s':", name, type, domain);
                avahi_address_snprint(a, sizeof(a), address);
                t = avahi_string_list_to_string(txt);

                getLogger()->d(TAG, 
                                    "\t%s:%u (%s)"
                                    ", TXT=%s"
                                    ", cookie is %u"
                                    ", is_local: %i"
                                    ", our_own: %i"
                                    ", wide_area: %i"
                                    ", multicast: %i"
                                    ", cached: %i",
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
