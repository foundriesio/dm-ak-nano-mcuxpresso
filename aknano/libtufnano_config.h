/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __LIBTUFNANO_CONFIG_H__
#define __LIBTUFNANO_CONFIG_H__

#define LIBRARY_LOG_NAME "tufnano"
#define LIBRARY_LOG_LEVEL LOG_DEBUG

#include "logging_stack.h"

#define log_debug LogDebug
#define log_info LogInfo
#define log_error LogError

#define TUF_SIGNATURES_PER_ROLE_MAX_COUNT 2

#endif
