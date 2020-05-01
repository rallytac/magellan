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
