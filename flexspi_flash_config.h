/*
 * Copyright 2018-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifndef _FLEXSPI_FLASH_CONFIG_H_
#define _FLEXSPI_FLASH_CONFIG_H_

#include "fsl_flexspi.h"


/*******************************************************************************
 * Definitions
 ******************************************************************************/
/*${macro:start}*/
#define FLASH_PAGE_SIZE                 256
#define SECTOR_SIZE                     0x1000 /* 4K */
/*${macro:end}*/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*${prototype:start}*/
status_t sfw_flash_read(uint32_t dstAddr, void *buf, size_t len);
status_t sfw_flash_read_ipc(uint32_t address, uint8_t *buffer, uint32_t length);
/*${prototype:end}*/

#endif /* _FLEXSPI_FLASH_H_ */
