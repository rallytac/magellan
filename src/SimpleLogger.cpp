//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#include <stdarg.h>

#include "SimpleLogger.hpp"

namespace Magellan
{
    static const size_t MAX_LOG_BUFFER_SIZE = (8192 * 4);

    static void defaultLoggingHook(int level, const char * _Nonnull tag, const char *msg)
    {
        printf("%d/%s - %s\n", level, tag, msg);
        fflush(stdout);
    }

    SimpleLogger::SimpleLogger()
    {
        _pszBuffer = new char[MAX_LOG_BUFFER_SIZE + 1];
        _outputHook = defaultLoggingHook;
    }

    SimpleLogger::~SimpleLogger()
    {
        delete[] _pszBuffer;
    }

    void SimpleLogger::f(const char *pszTag, const char *pszFmt, ...)
    {
        std::lock_guard<std::mutex>   scopedLock(_lock);

        va_list args;
        va_start(args, pszFmt);
        vsnprintf(_pszBuffer, MAX_LOG_BUFFER_SIZE, pszFmt, args);

        _outputHook(0, pszTag, _pszBuffer);

        va_end(args);
    }

    void SimpleLogger::e(const char *pszTag, const char *pszFmt, ...)
    {
        std::lock_guard<std::mutex>   scopedLock(_lock);

        va_list args;
        va_start(args, pszFmt);
        vsnprintf(_pszBuffer, MAX_LOG_BUFFER_SIZE, pszFmt, args);

        _outputHook(1, pszTag, _pszBuffer);

        va_end(args);
    }

    void SimpleLogger::w(const char *pszTag, const char *pszFmt, ...)
    {
        std::lock_guard<std::mutex>   scopedLock(_lock);

        va_list args;
        va_start(args, pszFmt);
        vsnprintf(_pszBuffer, MAX_LOG_BUFFER_SIZE, pszFmt, args);

        _outputHook(2, pszTag, _pszBuffer);

        va_end(args);
    }

    void SimpleLogger::i(const char *pszTag, const char *pszFmt, ...)
    {
        std::lock_guard<std::mutex>   scopedLock(_lock);

        va_list args;
        va_start(args, pszFmt);
        vsnprintf(_pszBuffer, MAX_LOG_BUFFER_SIZE, pszFmt, args);

        _outputHook(3, pszTag, _pszBuffer);

        va_end(args);
    }

    void SimpleLogger::d(const char *pszTag, const char *pszFmt, ...)
    {
        std::lock_guard<std::mutex>   scopedLock(_lock);

        va_list args;
        va_start(args, pszFmt);
        vsnprintf(_pszBuffer, MAX_LOG_BUFFER_SIZE, pszFmt, args);

        _outputHook(4, pszTag, _pszBuffer);

        va_end(args);
    }
}

