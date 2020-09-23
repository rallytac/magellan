/*
 *  Copyright (c) 2020 Rally Tactical Systems, Inc.
 *  All rights reserved.
 */

/**
 *  @file MagellanApi.h
 *  @brief The C-callable API for the Magellan library.
 */

#ifndef MAGELLANAPI_H
#define MAGELLANAPI_H

#include "MagellanTypes.h"
#include "MagellanConstants.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * @brief [SYNC] Initializes the Magellan library.
 *
 * Call this function to initialize the library prior starting main operation.
 *
 * @param configuration An optional JSON object containing configuration
 *
 * @return MAGELLAN_RESULT_OK if successful
 * @see magellanShutdown()
 */
MAGELLAN_API int magellanInitialize(const char * _Nullable configuration);


/**
 * @brief [SYNC] Shuts down the Magellan library.
 *
 * Calling this function will carry out the inverse of magellanInitialize().  Typically this 
 * function is called as part of the application shutdown procedure or when a full restart of the 
 * library is required.
 *
 * Furthermore, as this is a synchronous function call, the application should take care
 * to call it from a thread that will not be impacted by a delay in processing.
 *
 * @return MAGELLAN_RESULT_OK if successful
 * @see magellanInitialize()
*/
MAGELLAN_API int magellanShutdown();


/**
 * @brief [SYNC] Installs a callback function to be called for logging
 *
 * @return MAGELLAN_RESULT_OK if successful
 * @see magellanLogMessage()
*/
MAGELLAN_API int magellanSetLoggingHook(PFN_MAGELLAN_LOGGING_HOOK hookFn);


/**
 * @brief [SYNC] Sets the logging level (0-4)
 *
 * @return MAGELLAN_RESULT_OK if successful
 * @see magellanLogMessage(), logLevels
*/
MAGELLAN_API int magellanSetLoggingLevel(int level);


/**
 * @brief [SYNC] Logs a message using the currenbtly active logger.
 *
 * @return None
 * @see magellanSetLoggingHook()
*/
MAGELLAN_API void magellanLogMessage(int level, const char * _Nonnull tag, const char * _Nonnull msg);


/**
 * @brief [SYNC] Begins discovery for a particular discovery type.
 *
 * Once a discoverer has been created (or reused) for the request, a handle
 * to it is returned into the token pointed to by pToken.
 * 
 * @param discoveryType The discovery method to use.
 * @param pToken Pointer to a token buffer where the handle for the discoverer is returned.
 * @param hookFn Optional callback function which allows the application to filter discoveries.
 * @param userData Optional user data passed to the application on callbacks for the token.
 * 
 * @return MAGELLAN_RESULT_OK if successful.
 * @see discoveryTypes, magellanEndDiscovery(), magellanPauseDiscovery(), magellanResumeDiscovery()
*/
MAGELLAN_API int magellanBeginDiscovery(const char * _Nullable discoveryType, 
                                        MagellanToken_t * _Nonnull pToken,
                                        PFN_MAGELLAN_DISCOVERY_FILTER_HOOK hookFn,
                                        const void * _Nullable userData);


/**
 * @brief [SYNC] End discovery for a token.
 *
 * @param token The token on which discovery is to cease.
 * 
 * @return MAGELLAN_RESULT_OK if successful.
 * @see magellanBeginDiscovery(), magellanPauseDiscovery(), magellanResumeDiscovery()
*/
MAGELLAN_API int magellanEndDiscovery(MagellanToken_t token);


/**
 * @brief [SYNC] Pause discovery for a token.
 *
 * @param token The token on which discovery is to be paused.
 * 
 * @return MAGELLAN_RESULT_OK if successful.
 * @see magellanBeginDiscovery(), magellanEndDiscovery(), magellanResumeDiscovery()
*/
MAGELLAN_API int magellanPauseDiscovery(MagellanToken_t token);


/**
 * @brief [SYNC] Resumes discovery for a token.
 *
 * @param token The token on which discovery is to resume.
 * 
 * @return MAGELLAN_RESULT_OK if successful.
 * @see magellanBeginDiscovery(), magellanEndDiscovery(), magellanPauseDiscovery()
*/
MAGELLAN_API int magellanResumeDiscovery(MagellanToken_t token);


/**
 * @brief [SYNC] Set callbacks for discovery.
 *
 * @param pfnOnNewTalkgroups The function to call when new talkgroups are discovered.
 * @param pfnOnModifiedTalkgroups The function to call when configuration has changed for previously discovered talkgroups.
 * @param pfnOnRemovedTalkgroups The function to call when previously discovered talkgroups have been removed.
 * @param userData Application-defined user data to pass when calling the callbacks.
 * 
 * @return MAGELLAN_RESULT_OK if successful.
*/
MAGELLAN_API void magellanSetTalkgroupCallbacks(PFN_MAGELLAN_ON_NEW_TALKGROUPS pfnOnNewTalkgroups,
                            PFN_MAGELLAN_ON_MODIFIED_TALKGROUPS pfnOnModifiedTalkgroups,
                            PFN_MAGELLAN_ON_REMOVED_TALKGROUPS pfnOnRemovedTalkgroups,
                            const void *userData);



#ifdef __cplusplus
}
#endif
#endif
