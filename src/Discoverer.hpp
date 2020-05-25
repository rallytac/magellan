//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef IDISCOVERER_HPP
#define IDISCOVERER_HPP

#include "MagellanTypes.h"
#include "MagellanObject.hpp"
#include "WorkQueue.hpp"

namespace Magellan
{
    /**
     * @brief Base class for service and entity discovers
     **/
    class Discoverer : public MagellanObject
    {
    public:
        /** @brief Constructor **/
        Discoverer()               
        {
            _wq = nullptr;
            _hook = nullptr;
            _userData = nullptr;
        }

        /** @brief Destructor **/
        virtual ~Discoverer()      {}

        /** @brief Set the service type **/
        inline virtual void setServiceType(const char *serviceType)
        {
            _serviceType = serviceType;
        }

        /** @brief Get the service type **/
        inline virtual const char *getServiceType()
        {
            return _serviceType.c_str();
        }
        
        /** @brief Start the discoverer **/
        virtual bool start() = 0;

        /** @brief Stop the discoverer **/
        virtual void stop() = 0;

        /** @brief Pause the discoverer **/
        virtual void pause() = 0;

        /** @brief Resume the discoverer **/
        virtual void resume() = 0;

        /** @brief Set the work queue **/
        inline virtual void setWorkQueue(WorkQueue *wq)
        {
            _wq = wq;
        }

        /** @brief Get the work queue **/
        inline virtual WorkQueue *getWorkQueue()
        {
            return _wq;
        }

    private:
        /** @brief The service type for this discoverer **/
        std::string                         _serviceType;

        /** @brief Pointer to the work queue **/
        WorkQueue                           *_wq;

        /** @brief User data **/
        const void                          *_userData;

        /** @brief callback hook **/
        PFN_MAGELLAN_ASSET_DISCOVERY_HOOK   _hook;
    };
}

#endif
