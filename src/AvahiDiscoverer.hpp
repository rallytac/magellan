//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef AVAHIDISCOVERER_HPP
#define AVAHIDISCOVERER_HPP

#include <thread>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>
#include <avahi-common/thread-watch.h>
#include <avahi-common/malloc.h>
#include <avahi-common/error.h>

#include "Discoverer.hpp"
#include "Sem.hpp"

namespace Magellan
{
    /** @brief Provides discovery services on Linux systems using Avahi **/
    class AvahiDiscoverer : public Discoverer
    {
    public:
        AvahiDiscoverer();
        virtual ~AvahiDiscoverer();

        // Override from ReferenceCountedObject to stop before deletion
        virtual void deleteThis();
        
        virtual bool start();
        virtual void stop();
        virtual void pause();
        virtual void resume();

    private:
        AvahiThreadedPoll           *_poller;
        AvahiClient                 *_client;
        AvahiServiceBrowser         *_serviceBrowser;

        static void clientCallbackHelper(AvahiClient *c, 
                                         AvahiClientState state,
                                         void *userData);

        void clientCallback(AvahiClient *c,
                            AvahiClientState state);

        static void browseCallbackHelper(AvahiServiceBrowser *b, 
                                         AvahiIfIndex interface,
                                         AvahiProtocol protocol,
                                         AvahiBrowserEvent event,
                                         const char *name,
                                         const char *type,
                                         const char *domain,
                                         AvahiLookupResultFlags flags,
                                         void* userData);

        void browseCallback(AvahiServiceBrowser *b,
                            AvahiIfIndex interface,
                            AvahiProtocol protocol,
                            AvahiBrowserEvent event,
                            const char *name,
                            const char *type,
                            const char *domain,
                            AvahiLookupResultFlags flags);

        static void resolveCallbackHelper(AvahiServiceResolver *r,
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
                                          void* userData);

        void resolveCallback(AvahiServiceResolver *r,
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
                                          AvahiLookupResultFlags flags);
    };
}

#endif
