//
//  Copyright (c) 2020 Rally Tactical Systems, Inc.
//  All rights reserved.
//

#ifndef ILOGGER_h
#define ILOGGER_h

namespace Magellan
{
    /** @brief Interface definition for a logger **/
    class ILogger
    {
    public:
        /** @brief Logging levels **/
        typedef enum 
        {
            /** @brief Error occurred, operation cannot continue **/
            fatal = 0, 

            /** @brief Error occurred but operation can continue **/
            error = 1, 

            /** @brief Warning that should be attended to soon **/
            warning = 2, 

            /** @brief Informational only **/
            info = 3, 

            /** @brief Debugging for development and troubleshooting **/
            debug = 4
        } Level;

        /** @brief Constructor **/
        ILogger()
        {
            _maxLevel = debug;
            _enableSyslog = false;
        }

        /** @brief Destructor **/
        virtual ~ILogger()
        {
        }

        /** 
         * @brief Returns true if the logging level is value 
         * 
         * @param level Level to test
         * 
         * @see ILogger::Level 
         **/
        inline virtual bool isValidLevel(int level)
        {
            return (level >= (int) fatal && level <= (int) debug);
        }

        /** 
         * @brief Set the highest logging level 
         * 
         * @param maxLevel Maximum level to log out
         * 
         * @see ILogger::Level 
         **/
        inline virtual void setMaxLevel(Level maxLevel)
        {
            if(maxLevel >= fatal && maxLevel <= debug)
            {
                _maxLevel = maxLevel;
            }
        }

        /** @brief Return the current logging level **/
        inline virtual Level getMaxLevel()
        {
            return _maxLevel;
        }

        /** @brief Enable/disable logging to OS syslog (or equivalent) subsystem **/
        inline virtual void setSyslogEnabled(bool enabled)
        {
            _enableSyslog = enabled;
        }

        /** @brief Indicate whether logging to OS syslog subsystem is enabled **/
        inline virtual bool isSyslogEnabled()
        {
            return _enableSyslog;
        }

    #ifdef WIN32
        /** @brief Log a debug message **/
        virtual void d(const char *pszTag, _Printf_format_string_ const char *pszFmt, ...) = 0;

        /** @brief Log an informational message **/
        virtual void i(const char *pszTag, _Printf_format_string_ const char *pszFmt, ...) = 0;

        /** @brief Log a warning message **/
        virtual void w(const char *pszTag, _Printf_format_string_ const char *pszFmt, ...) = 0;

        /** @brief Log a debug message **/
        virtual void e(const char *pszTag, _Printf_format_string_ const char *pszFmt, ...) = 0;

        /** @brief Log a fatal message **/
        virtual void f(const char *pszTag, _Printf_format_string_ const char *pszFmt, ...) = 0;
    #else
        /** @brief Log a debug message **/
        virtual void d(const char *pszTag, const char *pszFmt, ...) __attribute__((__format__(__printf__, 3, 4))) = 0;

        /** @brief Log an informational message **/
        virtual void i(const char *pszTag, const char *pszFmt, ...) __attribute__((__format__(__printf__, 3, 4))) = 0;

        /** @brief Log a warning message **/
        virtual void w(const char *pszTag, const char *pszFmt, ...) __attribute__((__format__(__printf__, 3, 4))) = 0;

        /** @brief Log a debug message **/
        virtual void e(const char *pszTag, const char *pszFmt, ...) __attribute__((__format__(__printf__, 3, 4))) = 0;

        /** @brief Log a fatal message **/
        virtual void f(const char *pszTag, const char *pszFmt, ...) __attribute__((__format__(__printf__, 3, 4))) = 0;
    #endif

    protected:
        /** @brief Maximum logging level **/
        Level   _maxLevel;

        /** @brief Syslog enabled/disabled **/
        bool    _enableSyslog;
    };
}

#endif
