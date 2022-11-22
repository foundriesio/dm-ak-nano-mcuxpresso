/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _FLASH_PARTITIONING_H_
#define _FLASH_PARTITIONING_H_

#ifdef AKNANO_BOARD_MODEL_RT1060
#define BOOT_FLASH_BASE     0x60000000
#define BOOT_FLASH_ACT_APP  0x60040000
#define BOOT_FLASH_CAND_APP 0x60240000
#else
#define BOOT_FLASH_BASE     0x30000000
#define BOOT_FLASH_ACT_APP  0x30040000
#define BOOT_FLASH_CAND_APP 0x30240000
#endif

#endif
