//
//  Copyright (c) 2019 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include "TimerManager.hpp"

#include <chrono>
#include <inttypes.h>

namespace Magellan
{
    static const uint64_t DOZING_MS = (1000 * 60 * 10);

    TimerManager::TimerManager()
    {
        _id = 0;
        _running = false;
        _currentSleepTime = 0;
    }

    TimerManager::~TimerManager()
    {
        stop();
    }

    void TimerManager::start()
    {
        if(!_running)
        {
            _running = true;
            
            _lock.lock();
            {
                clearTimers();
                
                determineCurrentSleepTime();
                
                _threadHandle = std::thread(&TimerManager::timerThread, this);
            }
            _lock.unlock();
        }
    }

    void TimerManager::stop()
    {
        if(_running)
        {
            _running = false;
            _wakeUpSem.notify();
            
            if(_threadHandle.joinable())
            {
                _threadHandle.join();
            }
        }
        
        _lock.lock();
        {
            clearTimers();
            clearTrash();
        }
        _lock.unlock();
    }

    void TimerManager::clearTimers()
    {
        for(std::map<uint64_t, TimerEvent*>::iterator itr = _timers.begin();
            itr != _timers.end();
            itr++)
        {
            delete itr->second;
        }
        
        _timers.clear();
    }

    void TimerManager::clearTrash()
    {
        for(std::vector<TimerEvent*>::iterator itr = _trash.begin();
            itr != _trash.end();
            itr++)
        {
            delete (*itr);
        }
        
        _trash.clear();
    }

    uint64_t TimerManager::setTimer(CALLBACK_FN fn, const void *ctx, uint64_t ms, bool repeat)
    {
        uint64_t rc;
        
        _lock.lock();
        {
            _id++;
            
            TimerEvent *te = new TimerEvent();
            
            te->id = _id;
            te->fn = fn;
            te->ctx = ctx;
            te->ms = ms;
            te->expiresAt = (getNowMs() + ms);
            te->repeat = repeat;
            
            _timers[_id] = te;
            
            determineCurrentSleepTime();
            _wakeUpSem.notify();
            
            rc = _id;
        }
        _lock.unlock();
        
        return rc;
    }

    void TimerManager::cancelTimer(uint64_t hnd)
    {
        _lock.lock();
        {
            std::map<uint64_t, TimerEvent*>::iterator itr = _timers.find(hnd);
            if(itr != _timers.end())
            {
                _trash.push_back(itr->second);
                _timers.erase(itr);
            }
            
            determineCurrentSleepTime();
            _wakeUpSem.notify();
        }
        _lock.unlock();
    }

    void TimerManager::restartTimer(uint64_t hnd)
    {    
        _lock.lock();
        {
            std::map<uint64_t, TimerEvent*>::iterator itr = _timers.find(hnd);
            if(itr != _timers.end())
            {
                itr->second->expiresAt = (getNowMs() + itr->second->ms);
            }
            
            determineCurrentSleepTime();
            _wakeUpSem.notify();
        }
        _lock.unlock();
    }

    uint64_t TimerManager::getNowMs()
    {
        return static_cast<uint64_t>(std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now())
                                    .time_since_epoch()
                                    .count());
    }

    void TimerManager::determineCurrentSleepTime()
    {
        if(_timers.empty())
        {
            _currentSleepTime = DOZING_MS;
        }
        else
        {
            uint64_t now = getNowMs();
            _currentSleepTime = UINT64_MAX;
            
            for(std::map<uint64_t, TimerEvent*>::iterator itr = _timers.begin();
                itr != _timers.end();
                itr++)
            {
                if(itr->second->expiresAt >= now)
                {
                    uint64_t delta = (itr->second->expiresAt - now);
                    if(delta < _currentSleepTime)
                    {
                        _currentSleepTime = delta;
                    }
                }
            }
            
            if(_currentSleepTime > 0)
            {
                _currentSleepTime /= 4;
            }
            
            if(_currentSleepTime == 0 || _currentSleepTime == UINT64_MAX)
            {
                _currentSleepTime = 1;
            }
        }
    }

    void TimerManager::timerThread()
    {
        uint64_t                    now;
        uint64_t                    sleepFor;
        std::vector<TimerEvent*>    woken;

        while(_running)
        {
            _lock.lock();
            {
                sleepFor = _currentSleepTime;
            }
            _lock.unlock();
            
            _wakeUpSem.waitFor((int)sleepFor);
            
            if(!_running)
            {
                break;
            }
            
            _lock.lock();
            {
                if(!_timers.empty())
                {
                    now = getNowMs();
                    
                    woken.clear();
                    
                    for(std::map<uint64_t, TimerEvent*>::iterator itr = _timers.begin();
                        itr != _timers.end();
                        itr++)
                    {
                        if(itr->second->expiresAt <= now)
                        {
                            woken.push_back(itr->second);
                        }
                    }
                    
                    for(std::vector<TimerEvent*>::iterator itr = woken.begin();
                        itr != woken.end();
                        itr++)
                    {
                        if(!(*itr)->repeat)
                        {
                            _trash.push_back((*itr));
                            _timers.erase((*itr)->id);
                        }
                    }
                }

                determineCurrentSleepTime();   
                clearTrash();
            }
            _lock.unlock();

            if(!woken.empty())
            {
                for(std::vector<TimerEvent*>::iterator itr = woken.begin();
                    itr != woken.end();
                    itr++)
                {
                    ((*itr)->fn)((*itr)->id, (*itr)->ctx);

                    if((*itr)->repeat)
                    {
                        (*itr)->expiresAt = (getNowMs() + (*itr)->ms);
                    }
                }

                woken.clear();
            }
        }

        woken.clear();
    }
}

