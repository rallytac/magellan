//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef SSDPDISCOVERER_HPP
#define SSDPDISCOVERER_HPP

#include <thread>
#include <atomic>
#include <map>
#include <string>

#include "Discoverer.hpp"
#include "Sem.hpp"


namespace Magellan
{
    /** @brief Provides discovery services on POSIX systems using SSDP **/
    class SsdpDiscoverer : public Discoverer
    {
    public:
        SsdpDiscoverer();
        virtual ~SsdpDiscoverer();

        // Override from ReferenceCountedObject to stop before deletion
        virtual void deleteThis();
        
        virtual bool configure(DataModel::JsonObjectBase& configuration);
        virtual bool start();
        virtual void stop();
        virtual void pause();
        virtual void resume();

    private:        
        DataModel::Ssdp                 _configuration;
        std::atomic<bool>               _running;
        std::thread                     _workerThreadHandle;

        typedef struct _NeighborData_t
        {
            uint64_t        _expiresAt;
            unsigned long   _version;
        } NeighborData_t;

        typedef std::map<std::string, NeighborData_t> NeighborMap_t;

        NeighborMap_t                   _neighbors;

        void workerThread();

        DataModel::DiscoveredDevice *parseMessage(char *msg);
        void checkNeighbors();

        uint64_t                        _lastNeighborCheck;
    };
}

#endif
