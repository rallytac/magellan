//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef IDISCOVERER_HPP
#define IDISCOVERER_HPP

#include "MagellanTypes.h"
#include "MagellanConstants.h"
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
            _filterHook = nullptr;
            _userData = nullptr;
        }

        /** @brief Destructor **/
        virtual ~Discoverer()      {}

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

        /** @brief Configure the discoverer **/
        virtual bool configure(DataModel::JsonObjectBase& configuration) = 0;

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

        /** @brief Set the filter hook **/
        inline virtual void setHook(PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hook)
        {
            _filterHook = hook;
        }

        /** @brief Get the filter hook **/
        inline virtual PFN_MAGELLAN_DISCOVERY_FILTER_HOOK getFilterHook()
        {
            return _filterHook;
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
        virtual int callFilterHook(const char *detailJson)
        {
            if(_filterHook != nullptr)
            {
                return _filterHook(detailJson, _userData);
            }
            else
            {
                return MAGELLAN_FILTER_PROCEED;
            }
        }

    private:
        /** @brief The implementation of this discoverer **/
        std::string                         _implementation;

        /** @brief Pointer to the work queue **/
        WorkQueue                           *_wq;

        /** @brief User data **/
        const void                          *_userData;

        /** @brief fiter hook **/
        PFN_MAGELLAN_DISCOVERY_FILTER_HOOK  _filterHook;
    };
}

#endif
