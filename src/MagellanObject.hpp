//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef MAGELLANOBJECT_HPP
#define MAGELLANOBJECT_HPP

#include "ILogger.hpp"
#include "ReferenceCountedObject.hpp"

namespace Magellan
{   
    /**
     * @brief Base class for all Magellan objects
     **/
    class MagellanObject : public ReferenceCountedObject
    {
    public:
        /** @brief Constructor **/
        MagellanObject()               {}

        /** @brief Destructor **/
        virtual ~MagellanObject()      {}
    };
}
#endif
