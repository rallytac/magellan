//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include "ReferenceCountedObject.hpp"

namespace Magellan
{
    ReferenceCountedObject::ReferenceCountedObject()
    {
        _refCount = 1;
    }

    ReferenceCountedObject::~ReferenceCountedObject()
    {
    }

    void ReferenceCountedObject::addReference()
    {
        _refCount++;
    }

    void ReferenceCountedObject::releaseReference()
    {
        if(_refCount.fetch_sub(1) == 1)
        {
            deleteThis();
        }
    }

    void ReferenceCountedObject::deleteThis()
    {
        delete this;
    }
}
