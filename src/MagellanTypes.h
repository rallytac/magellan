/*
 *  Copyright (c) 2020 Rally Tactical Systems, Inc.
 *  All rights reserved.
 */

#ifndef MAGELLANTYPES_H
#define MAGELLANTYPES_H

#if defined(WIN32)
    #ifdef MAGELLAN_EXPORTS
        #define MAGELLAN_API  __declspec(dllexport) extern
    #else
        #define MAGELLAN_API  extern
    #endif
#else
    #define MAGELLAN_API
#endif

#if !defined(__clang__)
    #define _Nullable
    #define _Nonnull
#endif

/** @brief A Magellan token **/
typedef void * MagellanToken_t;

/** @brief Prototype for the logging callback hook **/
typedef void (* _Nullable PFN_MAGELLAN_LOGGING_HOOK)(int level, const char * _Nonnull tag, const char *msg);

/** @brief Prototype for the asset discovery filter hook **/
typedef int (* _Nullable PFN_MAGELLAN_DISCOVERY_FILTER_HOOK)(const char * _Nonnull detailJson, const void * _Nullable userData);

/** @brief Prototype for notification of newly discovered talkgroups **/
typedef void (* _Nullable PFN_MAGELLAN_ON_NEW_TALKGROUPS)(const char * _Nonnull newTalkgroupsJson, const void * _Nullable userData);

/** @brief Prototype for notification of modified talkgroups **/
typedef void (* _Nullable PFN_MAGELLAN_ON_MODIFIED_TALKGROUPS)(const char * _Nonnull modifiedTalkgroupsJson, const void * _Nullable userData);

/** @brief Prototype for notification of removed talkgroups **/
typedef void (* _Nullable PFN_MAGELLAN_ON_REMOVED_TALKGROUPS)(const char * _Nonnull removedTalkgroupsJson, const void * _Nullable userData);

#endif
