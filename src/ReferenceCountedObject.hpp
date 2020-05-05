//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef REFERENCECOUNTEDOBJECT_HPP
#define REFERENCECOUNTEDOBJECT_HPP

#include <atomic>
#include <mutex>
#include <list>

namespace Magellan
{
    /**
     * @brief Class that allows for reference tracking of objects
     **/
    class ReferenceCountedObject
    {
    public:
        /** @brief Constructor - sets the referemce count to 1 **/
        ReferenceCountedObject();

        /** @brief Destructor **/
        virtual ~ReferenceCountedObject();

        /** @brief Adds a reference **/
        virtual void addReference();

        /** @brief Releases a reference.  When the reference count drops to zero, the object is deleted. **/
        virtual void releaseReference();

        virtual inline int getRefCount()
        {
            return _refCount;
        }

    protected:
        /** @brief Deletes the object **/
        virtual void deleteThis();

    private:
        /** @brief Tracks the references to the object **/
        std::atomic<int>            _refCount;

    };
}
#endif
