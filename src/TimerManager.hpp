//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef TimerManager_hpp
#define TimerManager_hpp

#include <cstdint>
#include <map>
#include <thread>
#include <mutex>
#include <vector>

#include "Sem.hpp"

namespace Magellan
{
    class TimerManager
    {
    public:
        typedef bool (*CALLBACK_FN)(uint64_t hnd, const void *ctx);
        
        TimerManager();
        virtual ~TimerManager();
        
        void start();
        void stop();
        
        uint64_t setTimer(CALLBACK_FN fn, const void *ctx, uint64_t ms, bool repeat);
        void cancelTimer(uint64_t hnd);
        void restartTimer(uint64_t hnd);
        
    private:
        class TimerEvent
        {
        public:
            TimerEvent()
            {
                id = 0;
                fn = nullptr;
                ctx = nullptr;
                ms = 0;
                expiresAt = 0;
                repeat = false;
            }
            
            ~TimerEvent()
            {
            }
            
            uint64_t    id;
            CALLBACK_FN fn;
            const void  *ctx;
            uint64_t    ms;
            uint64_t    expiresAt;
            bool        repeat;
        };
        
        bool                                _running;
        std::thread                         _threadHandle;
        std::map<uint64_t, TimerEvent*>     _timers;
        std::mutex                          _lock;
        uint64_t                            _id;
        Sem                                 _wakeUpSem;
        uint64_t                            _currentSleepTime;
        std::vector<TimerEvent*>            _trash;
        
        void timerThread();
        uint64_t getNowMs();
        void determineCurrentSleepTime();
        void clearAll();
        void clearTimers();
        void clearTrash();
    };
}
#endif /* TimerManager_hpp */

