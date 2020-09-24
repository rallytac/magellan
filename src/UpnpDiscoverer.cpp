//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <string.h>

#include "UpnpDiscoverer.hpp"
#include "ILogger.hpp"
#include "MagellanCore.hpp"
#include "MagellanDataModel.hpp"

namespace Magellan
{
    static const char *TAG = "UpnpDiscoverer";

    UpnpDiscoverer::UpnpDiscoverer()
    {
        setImplementation("UPnP");
    }

    UpnpDiscoverer::~UpnpDiscoverer()
    {
    }

    void UpnpDiscoverer::deleteThis()
    {
        stop();
        ReferenceCountedObject::deleteThis();
    }

    bool UpnpDiscoverer::configure(DataModel::JsonObjectBase& configuration)
    {
        _configuration = (DataModel::Mdns&)configuration;
        return true;
    }
        
    bool UpnpDiscoverer::start()
    {
        bool rc = false;

        Core::getLogger()->d(TAG, "{%p} started", (void*) this);

        return rc;
    }

    void UpnpDiscoverer::stop()
    {
        Core::getLogger()->d(TAG, "{%p} stopped", (void*) this);
    }

    void UpnpDiscoverer::pause()
    {
        Core::getLogger()->d(TAG, "{%p} paused", (void*) this);
    }

    void UpnpDiscoverer::resume()
    {
        Core::getLogger()->d(TAG, "{%p} resumed", (void*) this);
    }
}
