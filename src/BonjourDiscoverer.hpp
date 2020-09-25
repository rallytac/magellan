//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef BONJOURDISCOVERER_HPP
#define BONJOURDISCOVERER_HPP

#include <thread>

#include "Discoverer.hpp"
#include "Sem.hpp"

#include "dns_sd.h"

namespace Magellan
{
    /** @brief Provides discovery services on non-Linux systems using Bonjour **/
    class BonjourDiscoverer : public Discoverer
    {
    public:
        BonjourDiscoverer();
        virtual ~BonjourDiscoverer();

        // Override from ReferenceCountedObject to stop before deletion
        virtual void deleteThis();
        
        virtual bool configure(DataModel::JsonObjectBase& configuration);
        virtual bool start();
        virtual void stop();
        virtual void pause();
        virtual void resume();

    private:
        DataModel::Mdns                 _configuration;
        std::atomic<bool>               _running;
        std::thread                     _workerThreadHandle;
        DNSServiceRef                   _clientConnection;

        void workerThread();

        static void DNSSD_API cb_BrowseReply(DNSServiceRef sdref, 
                                    const DNSServiceFlags flags, 
                                    uint32_t ifIndex, 
                                    DNSServiceErrorType errorCode,
                                    const char *replyName, 
                                    const char *replyType, 
                                    const char *replyDomain,
                                    void *context);

        void browseReply(DNSServiceRef sdref, 
                                const DNSServiceFlags flags, 
                                uint32_t ifIndex, 
                                DNSServiceErrorType errorCode,
                                const char *replyName, 
                                const char *replyType, 
                                const char *replyDomain);

        static void DNSSD_API cb_ResolveReply(DNSServiceRef sdref, 
									  const DNSServiceFlags flags,
									  uint32_t ifIndex,
									  DNSServiceErrorType errorCode,
									  const char *fullname, 
									  const char *hosttarget, 
									  uint16_t opaqueport, 
									  uint16_t txtLen, 
									  const unsigned char *txt, 
									  void *context); 

        void resolveReply(DNSServiceRef sdref, 
									  const DNSServiceFlags flags,
									  uint32_t ifIndex,
									  DNSServiceErrorType errorCode,
									  const char *fullname, 
									  const char *hosttarget, 
									  uint16_t opaqueport, 
									  uint16_t txtLen, 
									  const unsigned char *txt);                                                                     
    };
}

#endif
