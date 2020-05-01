//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef WORKQUEUE_HPP
#define WORKQUEUE_HPP

#include <thread>
#include <functional>
#include <list>
#include <forward_list>
#include <map>

#include "MagellanObject.hpp"
#include "Sem.hpp"

namespace Magellan
{
    /** @brief A simple worker queue **/
    class WorkQueue
    {
    public:   
        /** @brief The default maximum number of lamdas queued before new submissions are denied **/
        static const size_t DEFAULT_MAX_DEPTH = 512;
        
        /** @brief Constructor **/
        WorkQueue();

        /** @brief Destructor **/
        virtual ~WorkQueue();
        
        /** @brief Starts the worker queue **/
        void start();

        /** @brief Stops the worker queue and abandons outstanding lanbdas. **/
        void stop();

        /** @brief Restarts the worker queue.  Existing lambdas are abandoned. **/
        void restart();

        /** @brief Submit a lambda for asynchronous processing **/
        bool submit(std::function<void()> op);

        /** @brief Submit a lambda for processsing and return when it has completed **/
        bool submitAndWait(std::function<void()> op);
        
        /** @brief Sets the maximum queue depth **/
        inline void setMaxDepth(size_t d)
        {
            _maxDepth = d;
        }
        
        /** @brief Returns the maximum depth level **/
        inline size_t getMaxDepth()
        {
            return _maxDepth;
        }
        
        /** @brief Enable submission of lambdas **/
        inline void enableSubmissions()
        {
            _executorLock.lock();
            _allowSubmissions = true;
            _executorLock.unlock();
        }

        /** @brief Deny new lambda submissions **/
        inline void disableSubmissions()
        {
            _executorLock.lock();
            _allowSubmissions = false;
            _executorLock.unlock();
        }

        /** @brief Resets all queues and abandons queued lambdas **/
        void reset();
        
    private:
        /** @brief Represents a lambda held in the queue **/
        class Lambda
        {
        public:
            /** @brief Constructor **/
            Lambda(std::function<void()> r)
            {
                _r = r;
                _blockSemToSignal = nullptr;
            }

            /** @brief The actual lambda **/
            std::function<void()>   _r;

            /** @brief An optional signal to ping when the lamda has executed **/
            Sem                     *_blockSemToSignal;
        };

        /** @brief Indicates if the dispatcher is running **/
        bool                            _running;

        /** @brief Indicates if a fatal error has occurred **/
        bool                            _fatalError;

        /** @brief Locks the executor **/
        std::mutex                      _executorLock;

        /** @brief The queue of lambdas **/
        std::list<Lambda*>              _queue;

        /** @brief Pool of reusable lambdas **/
        std::forward_list<Lambda*>      _pool;

        /** @brief Size of the pool **/
        size_t                          _poolSize;

        /** @brief Thread handle of the dispatcher **/
        std::thread                     _dispatchThread;

        /** @brief Signalled to wake up the dispatcher **/
        Sem                             _semSignalAction;

        /** @brief Indicates that the dispatcher is operational **/
        Sem                             _semReady;

        /** @brief Inidicates that the dispatcher has completed **/
        Sem                             _semDone;

        /** @brief Tracks the maximum lambda queue depth **/
        size_t                          _maxDepth;

        /** @brief Indicates whether submissions are allowed **/
        bool                            _allowSubmissions;

        /** @brief Creates a lambda [internal] **/
        Lambda *createLambda(std::function<void()> r);

        /** @brief The dispatcher loop which runs on its own thread **/
        void dispatcher();

        /** @brief Clears queues [internal] **/
        void clear();

        /** @brief Returns a lambda to the pool [internal] **/
        void returnToPool(Lambda *l);
    };    
}

#endif
