//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef UPNPDISCOVERER_HPP
#define UPNPDISCOVERER_HPP

#include <thread>

#include "Discoverer.hpp"
#include "Sem.hpp"

namespace Magellan
{
    /** @brief Provides discovery services on non-Linux systems using UPnP **/
    class UpnpDiscoverer : public Discoverer
    {
    public:
        UpnpDiscoverer();
        virtual ~UpnpDiscoverer();

        // Override from ReferenceCountedObject to stop before deletion
        virtual void deleteThis();
        
        virtual bool configure(DataModel::JsonObjectBase& configuration);
        virtual bool start();
        virtual void stop();
        virtual void pause();
        virtual void resume();

    private:
        DataModel::Mdns             _configuration;
    };
}

#endif
