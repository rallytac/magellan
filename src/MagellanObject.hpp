//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef MAGELLANOBJECT_HPP
#define MAGELLANOBJECT_HPP

#include "ILogger.hpp"

namespace Magellan
{
    /**
     * @brief Base class for all Magellan objects
     **/
    class MagellanObject
    {
    public:
        /** @brief Constructor **/
        MagellanObject()               {}

        /** @brief Destructor **/
        virtual ~MagellanObject()      {}
        
        /** @brief Set the logger **/
        inline virtual void setLogger(ILogger *logger)
        {
            _logger = logger;
        }

        /** @brief Get the logger **/
        inline virtual ILogger *getLogger()
        {
            return _logger;
        }

    private:
        /** @brief Pointer to the logger **/
        ILogger         *_logger;
    };
}

#endif
