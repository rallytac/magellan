//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef IDISCOVERER_HPP
#define IDISCOVERER_HPP

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
        Discoverer()               {}

        /** @brief Destructor **/
        virtual ~Discoverer()      {}
        
        /** 
         * @brief Start the discoverer
         * 
         * @param serviceType String describing the service type to be discovered. If empty or null, the default Magellan service type is used.
         * **/
        virtual bool start(const char *serviceType) = 0;

        /** @brief Stop the discoverer **/
        virtual void stop() = 0;

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
        /** @brief Pointer to the logger **/
        WorkQueue       *_wq;            
    };
}

#endif
