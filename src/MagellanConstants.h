/*
 *  Copyright (c) 2020 Rally Tactical Systems, Inc.
 *  All rights reserved.
 */

#ifndef MAGELLANCONSTANTS_H
#define MAGELLANCONSTANTS_H

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

/** @brief A NULL value for a Magellan token **/
#define MAGELLAN_NULL_TOKEN                 nullptr

/** @brief The default Magellan service type for **/
#define MAGELLAN_DEFAULT_SERVICE_TYPE       "_magellan._tcp"

#endif
