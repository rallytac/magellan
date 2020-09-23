/*
 *  Copyright (c) 2020 Rally Tactical Systems, Inc.
 *  All rights reserved.
 */

#ifndef MAGELLANCONSTANTS_H
#define MAGELLANCONSTANTS_H

/**
 *  @file MagellanConstants.h
 *  @brief Constants and other pre-defined values.
 */

/** @brief A NULL value for a Magellan token **/
#define MAGELLAN_NULL_TOKEN                 nullptr


/** @addtogroup resultCodes Magellan Result Codes
 *
 * Result codes are returned by calls to the API functions and most often are related to
 * the submission of a request to Magellan rather than the outcome of that submission.
 *
 *  @{
 */
/** @brief The request was succesful */
static const int MAGELLAN_RESULT_OK = 0;
/** @brief One or more parameters are invalid */
static const int MAGELLAN_RESULT_INVALID_PARAMETERS = -1;
/** @brief The library has not yet been initialized - magellanInitialize() should be called */
static const int MAGELLAN_RESULT_NOT_INITIALIZED = -2;
/** @brief The library has already been initialized */
static const int MAGELLAN_RESULT_ALREADY_INITIALIZED = -3;
/** @brief An unspecified error has occurred */
static const int MAGELLAN_RESULT_GENERAL_FAILURE = -4;
/** @} */


/** @addtogroup logLevels Magellan Logging Levels
 *
 * Result codes are returned by calls to the API functions and most often are related to
 * the submission of a request to Magellan rather than the outcome of that submission.
 *
 *  @{
 */
/** @brief Fatal */
static const int MAGELLAN_LOG_LEVEL_FATAL = 0;
/** @brief Error */
static const int MAGELLAN_LOG_LEVEL_ERROR = 1;
/** @brief Warning */
static const int MAGELLAN_LOG_LEVEL_WARNING = 2;
/** @brief Informational */
static const int MAGELLAN_LOG_LEVEL_INFORMATIONAL = 3;
/** @brief Debug */
static const int MAGELLAN_LOG_LEVEL_DEBUG = 4;
/** @} */


/** @addtogroup discoveryTypes Magellan Discovery Types
 *
 * Discovery types are unique strings that identify the method by which
 * discovery of devices is to occur.
 *
 *  @{
 */
/** @brief MDNS discovery type **/
#define MAGELLAN_MDNS_DISCOVERY_TYPE                "mdns"

/** @brief SSDP discovery type **/
#define MAGELLAN_SSDP_DISCOVERY_TYPE                "ssdp"

/** @brief Trellisware discovery type **/
#define MAGELLAN_TRELLISWAREDISCOVERY_TYPE          "trellisware"

/** @brief CISTECH discovery type **/
#define MAGELLAN_CISTECH_DISCOVERYTYPE              "cistech"

/** @brief CUBIC Vocality discovery type **/
#define MAGELLAN_VOCALITY_DISCOVERY_TYPE            "vocality"

/** @brief Application discovery type **/
#define MAGELLAN_APPLICATION_DISCOVERY_TYPE         "application"

/** @brief The default Magellan discovery type **/
#define MAGELLAN_DEFAULT_DISCOVERY_TYPE         MAGELLAN_MDNS_DISCOVERY_TYPE
/** @} */

/** @addtogroup discoveryFilterCodes Magellan Discovery Filter Codes
 *
 * Values returned by the discovery filter callback hook to indicate whether to proceed with
 * normal actions regarding asset discovery/rediscovery/undiscovery.
 *
 *  @{
 */
/** @brief Ignore the event */
static const int MAGELLAN_FILTER_IGNORE = 0;
/** @brief Processing should continue */
static const int MAGELLAN_FILTER_PROCEED = 1;
/** @} */

#endif
