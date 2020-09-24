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

    BonjourDiscoverer::BonjourDiscoverer()
    {
        setImplementation("Bonjour");
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

        return rc;
    }

    void BonjourDiscoverer::stop()
    {
        Core::getLogger()->d(TAG, "{%p} stopped", (void*) this);
    }

    void BonjourDiscoverer::pause()
    {
        Core::getLogger()->d(TAG, "{%p} paused", (void*) this);
    }

    void BonjourDiscoverer::resume()
    {
        Core::getLogger()->d(TAG, "{%p} resumed", (void*) this);
    }
}
