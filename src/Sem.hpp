//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef SEM_HPP
#define SEM_HPP

#include <mutex>
#include <condition_variable>
#include <chrono>

namespace Magellan
{
    /** @brief A simple semaphore class **/
    class Sem
    {
    public:
        /** @brief Constructor **/
        Sem()
        {
            _sig = false;
        }
        
        /** @brief Destructor **/
        virtual ~Sem()
        {
        }
        
        /** @brief Signal the sempahore **/
        inline void notify()
        {
            {
                std::lock_guard<std::mutex> lck(_mutex);
                _sig = true;
            }
            
            _condVar.notify_one();
        }
        
        /** @brief Waits indefinitely for notification **/
        inline void wait()
        {
            std::unique_lock<std::mutex> lck(_mutex);
            _condVar.wait(lck, [this]{ return _sig; });
            _sig = false;
        }
        
        /** @brief Waits for a period of time to be signalled
         * 
         * @param ms Number of milliseconds to wait **/
        inline void waitFor(int ms)
        {
            std::unique_lock<std::mutex> lck(_mutex);
            _condVar.wait_for(lck, std::chrono::milliseconds(ms), [this]{ return _sig; });
            _sig = false;
        }
        
        /** @brief Reset the signalled state **/
        inline void reset()
        {
            _sig = false;
        }
        
    private:
        /** @brief Mutex used to lock the object **/
        std::mutex              _mutex;

        /** @brief Condition variable used for actual operation **/
        std::condition_variable _condVar;

        /** @brief The current signal state **/
        bool                    _sig;
    };
}

#endif
