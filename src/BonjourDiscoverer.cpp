//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <string.h>

#include "BonjourDiscoverer.hpp"
#include "ILogger.hpp"
#include "MagellanCore.hpp"
#include "MagellanDataModel.hpp"

namespace Magellan
{
    static const char *TAG = "BonjourDiscoverer";

    static bool getTxtRecordItem(uint16_t txtLen, const unsigned char *txt, const char *keyName, std::string *result)
    {
        bool rc = false;

        if (TXTRecordContainsKey(txtLen, txt, keyName) == 1)
        {
            uint8_t vl;
            const void *vp = TXTRecordGetValuePtr(txtLen, txt, keyName, &vl);

            if (vl > 0 && vp != nullptr)
            {
                result->assign((const char*)vp, vl);
                rc = true;
            }
        }

        if (!rc)
        {
            result->clear();
        }

        return rc;
    }

    BonjourDiscoverer::BonjourDiscoverer()
    {
        setImplementation("Bonjour");

        _running = false;
        _clientConnection = nullptr;
    }

    BonjourDiscoverer::~BonjourDiscoverer()
    {
    }

    void BonjourDiscoverer::deleteThis()
    {
        stop();
        ReferenceCountedObject::deleteThis();
    }

    bool BonjourDiscoverer::configure(DataModel::JsonObjectBase& configuration)
    {
        _configuration = (DataModel::Mdns&)configuration;
        return true;
    }
        
    bool BonjourDiscoverer::start()
    {
        bool rc = false;

        Core::getLogger()->d(TAG, "{%p} started", (void*) this);

        if(_running)
        {
            return true;
        }

        _running = true;

        _workerThreadHandle = std::thread(&BonjourDiscoverer::workerThread, this);

        return rc;
    }

    void BonjourDiscoverer::stop()
    {
        Core::getLogger()->d(TAG, "{%p} stopped", (void*) this);

        _running = false;

        if(_workerThreadHandle.joinable())
        {
            _workerThreadHandle.join();
        }
    }

    void BonjourDiscoverer::pause()
    {
        Core::getLogger()->d(TAG, "{%p} paused", (void*) this);
    }

    void BonjourDiscoverer::resume()
    {
        Core::getLogger()->d(TAG, "{%p} resumed", (void*) this);
    }

    /*static*/ void DNSSD_API BonjourDiscoverer::cb_ResolveReply(DNSServiceRef sdref, 
                                                const DNSServiceFlags flags,
                                                uint32_t ifIndex,
                                                DNSServiceErrorType errorCode,
                                                const char *fullname, 
                                                const char *hosttarget, 
                                                uint16_t opaqueport, 
                                                uint16_t txtLen, 
                                                const unsigned char *txt, 
                                                void *context)    
    {
        ((BonjourDiscoverer*)context)->resolveReply(sdref,
                                                    flags, 
                                                    ifIndex, 
                                                    errorCode, 
                                                    fullname, 
                                                    hosttarget, 
                                                    opaqueport, 
                                                    txtLen, 
                                                    txt);
    }

    static void removeTralingPeriod(std::string& s)
    {
        if(!s.empty())
        {
            if(s.c_str()[s.length() - 1] == '.')
            {
                s = s.substr(0, s.length() - 1);
            }
        }
    }

    void BonjourDiscoverer::resolveReply(DNSServiceRef sdref, 
									  const DNSServiceFlags flags,
									  uint32_t ifIndex,
									  DNSServiceErrorType errorCode,
									  const char *fullname, 
									  const char *hosttarget, 
									  uint16_t opaqueport, 
									  uint16_t txtLen, 
									  const unsigned char *txt)
    {
        union 
        { 
            uint16_t s; 
            u_char b[2]; 
        } port = { opaqueport };

        uint16_t portAsNumber = ((uint16_t)port.b[0]) << 8 | port.b[1];

        std::string name;
        std::string host;
        std::string id;
        std::string cv;

        name.assign(fullname);
        removeTralingPeriod(name);

        host.assign(hosttarget);
        removeTralingPeriod(host);

        getTxtRecordItem(txtLen, txt, "id", &id);
        getTxtRecordItem(txtLen, txt, "cv", &cv);
        
        DNSServiceRefDeallocate(sdref);

        std::string json;
        int filterResponse;

        json.append("{");
            json.append("\"serviceType\":\""); json.append(_configuration.serviceType.c_str()); json.append("\"");
            json.append(",\"implementation\":\""); json.append(getImplementation()); json.append("\"");
            json.append(",\"name\":\""); json.append(name); json.append("\"");
            json.append(",\"hostName\":\""); json.append(host); json.append("\"");
        json.append("}");

        filterResponse = callFilterHook(json.c_str());

        if(filterResponse == MAGELLAN_FILTER_PROCEED)
        {
            DataModel::DiscoveredDevice    *dd = new DataModel::DiscoveredDevice();

            char buff[1024];

            sprintf(buff, "%s/%s/%s", getImplementation(), _configuration.serviceType.c_str(), name.c_str());
            dd->discovererKey.assign(buff);
            dd->id.assign(id);
            dd->configVersion = atoi(cv.c_str());

            // Note, HTTPS is assumed
            if(portAsNumber > 0)
            {
                sprintf(buff, "https://%s:%d/config", host.c_str(), portAsNumber);
            }
            else
            {
                sprintf(buff, "https://%s/config", host.c_str());
            }
            
            dd->rootUrl.assign(buff);

            Core::processDiscoveredDevice(dd);
        }
    }

    /*static*/ void DNSSD_API BonjourDiscoverer::cb_BrowseReply(DNSServiceRef sdref, 
                                        const DNSServiceFlags flags, 
                                        uint32_t ifIndex, 
                                        DNSServiceErrorType errorCode,
                                        const char *replyName, 
                                        const char *replyType, 
                                        const char *replyDomain,
                                        void *context)
    {
        ((BonjourDiscoverer*)context)->browseReply(sdref, 
                                                   flags, 
                                                   ifIndex, 
                                                   errorCode, 
                                                   replyName, 
                                                   replyType, 
                                                   replyDomain);
    }

    void BonjourDiscoverer::browseReply(DNSServiceRef sdref, 
                                        const DNSServiceFlags flags, 
                                        uint32_t ifIndex, 
                                        DNSServiceErrorType errorCode,
                                        const char *replyName, 
                                        const char *replyType, 
                                        const char *replyDomain)
    {
        if(flags & kDNSServiceFlagsAdd)
        {
            DNSServiceErrorType err;
            DNSServiceRef resolveRef;

            resolveRef = _clientConnection;
            
            err = DNSServiceResolve(&resolveRef, 
                                    kDNSServiceFlagsShareConnection, 
                                    ifIndex, 
                                    replyName, 
                                    replyType, 
                                    replyDomain, 
                                    cb_ResolveReply, 
                                    this);

            if (err != kDNSServiceErr_NoError)
            {
                Core::getLogger()->e(TAG, "DNSServiceResolve() failed, err=%d", err);
            }
        }
        else
        {
            std::string name;
            name.assign(replyName);
            removeTralingPeriod(name);

            char buff[1024];
            sprintf(buff, "%s/%s/%s", getImplementation(), _configuration.serviceType.c_str(), name.c_str());
            Core::processUndiscoveredDevice(buff);
        }
    }

    void BonjourDiscoverer::workerThread()
    {
        DNSServiceErrorType err;
        DNSServiceRef browseRef = nullptr;

        err = DNSServiceCreateConnection(&_clientConnection);
        if (err != kDNSServiceErr_NoError)
        {
            Core::getLogger()->e(TAG, "DNSServiceCreateConnection(_clientConnection) failed");
            return;
        }

        browseRef = _clientConnection;

        err = DNSServiceBrowse(&browseRef, 
                               kDNSServiceFlagsShareConnection, 
                               0, 
                               _configuration.serviceType.c_str(), 
                               "", 
                               cb_BrowseReply, 
                               this);

        while(_running)
        {
            int fd = DNSServiceRefSockFD(_clientConnection);

            fd_set readfds;
            struct timeval tv;
            int result;
            int nfds;

            FD_ZERO(&readfds);
            FD_SET(fd, &readfds);

            tv.tv_sec = 1;
            tv.tv_usec = 0;

            #if defined(WIN32)
                nfds = 1;
            #else
                ndfs = fd + 1;
            #endif

            result = select(nfds, &readfds, (fd_set*)NULL, (fd_set*)NULL, &tv);
            if (result < 0)
            {
                Core::getLogger()->e(TAG, "select() failed");
                break;
            }

            if (result > 0)
            {
                if (FD_ISSET(fd, &readfds))
                {
                    err = DNSServiceProcessResult(_clientConnection);
                    if (err != kDNSServiceErr_NoError)
                    {
                        Core::getLogger()->e(TAG, "DNSServiceProcessResult(browseConnection) failed");
                        break;
                    }
                }
            }
        }

        if (browseRef != nullptr)
        {
            DNSServiceRefDeallocate(browseRef);
        }

        if(_clientConnection != nullptr)
        {
            DNSServiceRefDeallocate(_clientConnection);
            _clientConnection = nullptr;
        }
    }
}
