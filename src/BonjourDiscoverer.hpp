//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef BONJOURDISCOVERER_HPP
#define BONJOURDISCOVERER_HPP

#include <thread>

#include "Discoverer.hpp"
#include "Sem.hpp"

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
        DataModel::Mdns             _configuration;
    };
}

#endif
