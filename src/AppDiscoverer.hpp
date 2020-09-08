//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef APPDISCOVERER_HPP
#define APPDISCOVERER_HPP

#include "Discoverer.hpp"
#include "Sem.hpp"

namespace Magellan
{
    /** @brief Provides discovery services using the app-provided discoverer interface **/
    class AppDiscoverer : public Discoverer
    {
    public:
        AppDiscoverer();
        virtual ~AppDiscoverer();

        virtual bool start();
        virtual void stop();
        virtual void pause();
        virtual void resume();
    };
}

#endif
