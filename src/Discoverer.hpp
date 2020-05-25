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

        /** @brief Set the implementation **/
        inline virtual void setImplementation(const char *implementation)
        {
            _implementation = implementation;
        }

        /** @brief Get the implementation **/
        inline virtual const char *getImplementation()
        {
            return _implementation.c_str();
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

        /** @brief Set the callback hook **/
        inline virtual void setHook(PFN_MAGELLAN_ASSET_DISCOVERY_HOOK hook)
        {
            _hook = hook;
        }

        /** @brief Get the callback hook **/
        inline virtual PFN_MAGELLAN_ASSET_DISCOVERY_HOOK getHook()
        {
            return _hook;
        }

        /** @brief Set the user data **/
        inline virtual void setUserData(const void *userData)
        {
            _userData = userData;
        }

        /** @brief Get the user data **/
        inline virtual const void *getUserData()
        {
            return _userData;
        }

        /** @brief Call the hook **/
        virtual void callHook(const char *detailJson)
        {
            if(_hook != nullptr)
            {
                _hook(detailJson, _userData);
            }
        }

    private:
        /** @brief The service type for this discoverer **/
        std::string                         _serviceType;

        /** @brief The implementation of this discoverer **/
        std::string                         _implementation;

        /** @brief Pointer to the work queue **/
        WorkQueue                           *_wq;

        /** @brief User data **/
        const void                          *_userData;

        /** @brief callback hook **/
        PFN_MAGELLAN_ASSET_DISCOVERY_HOOK   _hook;
    };
}

#endif
