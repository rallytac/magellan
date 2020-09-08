//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include "AppDiscoverer.hpp"
#include "ILogger.hpp"

namespace Magellan
{
    static const char *TAG = "AppDiscoverer";
    extern ILogger *logger;

    AppDiscoverer::AppDiscoverer()
    {
        logger->i(TAG, "{%p} ctor()", (void*) this);
        setImplementation("Application");
    }

    AppDiscoverer::~AppDiscoverer()
    {
        logger->i(TAG, "{%p} ~dtor()", (void*) this);
    }

    bool AppDiscoverer::start()
    {
        logger->d(TAG, "{%p} start", (void*) this);
        return true;
    }

    void AppDiscoverer::stop()
    {
        logger->d(TAG, "{%p} stop", (void*) this);
    }

    void AppDiscoverer::pause()
    {
        logger->d(TAG, "{%p} paused", (void*) this);
    }

    void AppDiscoverer::resume()
    {
        logger->d(TAG, "{%p} resumed", (void*) this);
    }
}
