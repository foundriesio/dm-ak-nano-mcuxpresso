/*
 * Copyright (c) 2016, Freescale Semiconductor, Inc.
 * Copyright 2016-2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "fsl_flexspi.h"
#include "app.h"
#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
#include "fsl_cache.h"
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

/*******************************************************************************
 * Variables
 *****************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
status_t flexspi_nor_wait_bus_busy(FLEXSPI_Type *base)
{
    /* Wait status ready. */
    bool isBusy;
    uint32_t readValue;
    status_t status;
    flexspi_transfer_t flashXfer;

    flashXfer.deviceAddress = 0;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READSTATUSREG;
    flashXfer.data          = &readValue;
    flashXfer.dataSize      = 1;

    do
    {
        status = FLEXSPI_TransferBlocking(base, &flashXfer);

        if (status != kStatus_Success)
        {
            return status;
        }
        if (FLASH_BUSY_STATUS_POL)
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = true;
            }
            else
            {
                isBusy = false;
            }
        }
        else
        {
            if (readValue & (1U << FLASH_BUSY_STATUS_OFFSET))
            {
                isBusy = false;
            }
            else
            {
                isBusy = true;
            }
        }

    } while (isBusy);

    return status;
}


// FIXME: set those in an appropriate header file
// FLEXSPI
#define UPDATE_EXAMPLE_FLEXSPI                        FLEXSPI1
#define UPDATE_EXAMPLE_FLEXSPI_AMBA_BASE              FlexSPI1_AMBA_BASE

status_t sfw_flash_read_ipc(uint32_t address, void *buffer, size_t length)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Prepare page program command */
    flashXfer.deviceAddress = address & (~UPDATE_EXAMPLE_FLEXSPI_AMBA_BASE);
    flashXfer.port          = kFLEXSPI_PortA1;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READ_FAST_QUAD;
    flashXfer.data          = (uint32_t *)(void *)buffer;
    flashXfer.dataSize      = length;
    status                  = FLEXSPI_TransferBlocking(UPDATE_EXAMPLE_FLEXSPI, &flashXfer);

    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(UPDATE_EXAMPLE_FLEXSPI);

    return status;
}

status_t sfw_flash_read(uint32_t dstAddr, void *buf, size_t len)
{
	uint32_t addr = dstAddr | UPDATE_EXAMPLE_FLEXSPI_AMBA_BASE;

//	DCACHE_CleanInvalidateByRange(addr, len);
	memcpy(buf, (void *)addr, len);

	return 0;
}
