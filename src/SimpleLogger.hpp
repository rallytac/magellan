//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef SIMPLER_LOGGER_HPP
#define SIMPLER_LOGGER_HPP

#include <mutex>

#include "ILogger.hpp"
#include "MagellanApi.h"

namespace Magellan
{
    /** @brief A simple logger **/
    class SimpleLogger : public ILogger
    {
    public:   
        /** @brief Constructor **/
        SimpleLogger();

        /** @brief Destructor **/
        virtual ~SimpleLogger();


        /** @brief Set the callback to be invoked to output log messages
         * 
         * The logging subsystem in the library guarantees that the output hook function is
         * called under lock so there is no need for hook to perform locking of it's own other
         * than for it's own purposes.
         * 
         * Also, this function call should return as soon as possible as the library will be
         * held up while logging is underway.
         **/
        inline void setOutputHook(PFN_MAGELLAN_LOGGING_HOOK outputHook)
        {
            _outputHook = outputHook;
        }

        virtual void f(const char *pszTag, const char *pszFmt, ...);
        virtual void e(const char *pszTag, const char *pszFmt, ...);
        virtual void w(const char *pszTag, const char *pszFmt, ...);
        virtual void i(const char *pszTag, const char *pszFmt, ...);
        virtual void d(const char *pszTag, const char *pszFmt, ...);
        
    private:
        std::mutex                  _lock;
        char                        *_pszBuffer;
        PFN_MAGELLAN_LOGGING_HOOK   _outputHook;
    };    
}

#endif
