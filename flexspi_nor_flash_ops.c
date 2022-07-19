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
extern flexspi_device_config_t deviceconfig;
extern const uint32_t customLUT[CUSTOM_LUT_LENGTH];

/*******************************************************************************
 * Code
 ******************************************************************************/
#if (defined CACHE_MAINTAIN) && (CACHE_MAINTAIN == 1)
void flexspi_nor_disable_cache(flexspi_cache_status_t *cacheStatus)
{
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    /* Disable D cache. */
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
        cacheStatus->DCacheEnableFlag = true;
    }
#endif /* __DCACHE_PRESENT */

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    /* Disable I cache. */
    if (SCB_CCR_IC_Msk == (SCB_CCR_IC_Msk & SCB->CCR))
    {
        SCB_DisableICache();
        cacheStatus->ICacheEnableFlag = true;
    }
#endif /* __ICACHE_PRESENT */

#elif (defined FSL_FEATURE_SOC_LMEM_COUNT) && (FSL_FEATURE_SOC_LMEM_COUNT != 0U)
    /* Disable code bus cache and system bus cache */
    if (LMEM_PCCCR_ENCACHE_MASK == (LMEM_PCCCR_ENCACHE_MASK & LMEM->PCCCR))
    {
        L1CACHE_DisableCodeCache();
        cacheStatus->codeCacheEnableFlag = true;
    }
    if (LMEM_PSCCR_ENCACHE_MASK == (LMEM_PSCCR_ENCACHE_MASK & LMEM->PSCCR))
    {
        L1CACHE_DisableSystemCache();
        cacheStatus->systemCacheEnableFlag = true;
    }

#elif (defined FSL_FEATURE_SOC_CACHE64_CTRL_COUNT) && (FSL_FEATURE_SOC_CACHE64_CTRL_COUNT != 0U)
    /* Disable cache */
    CACHE64_DisableCache(EXAMPLE_CACHE);
    cacheStatus->CacheEnableFlag = true;
#endif
}

void flexspi_nor_enable_cache(flexspi_cache_status_t cacheStatus)
{
#if (defined __CORTEX_M) && (__CORTEX_M == 7U)
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    if (cacheStatus.DCacheEnableFlag)
    {
        /* Enable D cache. */
        SCB_EnableDCache();
    }
#endif /* __DCACHE_PRESENT */

#if defined(__ICACHE_PRESENT) && (__ICACHE_PRESENT == 1U)
    if (cacheStatus.ICacheEnableFlag)
    {
        /* Enable I cache. */
        SCB_EnableICache();
    }
#endif /* __ICACHE_PRESENT */

#elif (defined FSL_FEATURE_SOC_LMEM_COUNT) && (FSL_FEATURE_SOC_LMEM_COUNT != 0U)
    if (cacheStatus.codeCacheEnableFlag)
    {
        /* Enable code cache. */
        L1CACHE_EnableCodeCache();
    }

    if (cacheStatus.systemCacheEnableFlag)
    {
        /* Enable system cache. */
        L1CACHE_EnableSystemCache();
    }

#elif (defined FSL_FEATURE_SOC_CACHE64_CTRL_COUNT) && (FSL_FEATURE_SOC_CACHE64_CTRL_COUNT != 0U)
    if (cacheStatus.CacheEnableFlag)
    {
        /* Enable cache. */
        CACHE64_EnableCache(EXAMPLE_CACHE);
    }
#endif
}
#endif

status_t flexspi_nor_write_enable(FLEXSPI_Type *base, uint32_t baseAddr)
{
    flexspi_transfer_t flashXfer;
    status_t status;

    /* Write enable */
    flashXfer.deviceAddress = baseAddr;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = kFLEXSPI_Command;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_WRITEENABLE;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);

    return status;
}

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

status_t flexspi_nor_enable_quad_mode(FLEXSPI_Type *base)
{
    flexspi_transfer_t flashXfer;
    status_t status;
    uint32_t writeValue = FLASH_QUAD_ENABLE;

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    /* Enable quad mode. */
    flashXfer.deviceAddress = 0;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_WRITESTATUSREG;
    flashXfer.data          = &writeValue;
    flashXfer.dataSize      = 1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    return status;
}

#if defined(NOR_CMD_LUT_SEQ_IDX_SETREADPARAMETER) && NOR_CMD_LUT_SEQ_IDX_SETREADPARAMETER
status_t flexspi_nor_set_read_parameter(
    FLEXSPI_Type *base, uint8_t burstLength, bool enableWrap, uint8_t dummyCycle, bool resetPinSelected)
{
    flexspi_transfer_t flashXfer;
    status_t status;
    uint32_t readParameterRegVal = ((uint32_t)resetPinSelected << RESET_PIN_SELECTED_REG_SHIFT) |
                                   ((uint32_t)dummyCycle << DUMMY_CYCLES_REG_SHIFT) |
                                   ((uint32_t)enableWrap << WRAP_ENABLE_REG_SHIFT) |
                                   ((uint32_t)burstLength << BURST_LEGNTH_REG_SHIFT);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_cache_status_t cacheStatus;
    flexspi_nor_disable_cache(&cacheStatus);
#endif

    /* Write enable */
    status = flexspi_nor_write_enable(base, 0);

    if (status != kStatus_Success)
    {
        return status;
    }

    flashXfer.deviceAddress = 0;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = kFLEXSPI_Write;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_SETREADPARAMETER;
    flashXfer.data          = &readParameterRegVal;
    flashXfer.dataSize      = 1;

    status = FLEXSPI_TransferBlocking(base, &flashXfer);
    if (status != kStatus_Success)
    {
        return status;
    }

    status = flexspi_nor_wait_bus_busy(base);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(base);

#if defined(CACHE_MAINTAIN) && CACHE_MAINTAIN
    flexspi_nor_enable_cache(cacheStatus);
#endif

    return status;
}
#endif


// FIXME: set those in an appropriate header file
// FLEXSPI
#define FLASH_CONFIG_SECTORSIZE             4u * 1024u

#define UPDATE_EXAMPLE_FLEXSPI                        FLEXSPI1
#define UPDATE_FLASH_SIZE_KB                          (UPDATE_COMPONENT_FLASHIAP_SIZE/1024) //0x10000
#define UPDATE_EXAMPLE_FLEXSPI_AMBA_BASE              FlexSPI1_AMBA_BASE
#define UPDATE_SECTOR_SIZE                            (FLASH_CONFIG_SECTORSIZE)  //0x40000
#define UPDATE_EXAMPLE_FLEXSPI_CLOCK                  kCLOCK_FlexSpi
#define UPDATE_FLEXSPI_BASE_ADDRESS_MASK              (UPDATE_FLASH_SIZE_KB * 0x400 -1)
#define UPDATE_FLASH_CONFIG_PAGESIZE               256
#define UPDATE_COMPONENT_FLASHIAP_SIZE 4194304

#define WINBOND_W25QxxxJV

#if defined(Macronix_MX25UM51345G)||defined(Macronix_MX25UM51345G_2nd)
status_t sfw_flash_read_ipc(uint32_t address, void *buffer, size_t length)
{
    status_t status;
    flexspi_transfer_t flashXfer;

    /* Read page. */
    flashXfer.deviceAddress = address;
    flashXfer.port          = FLASH_PORT;
    flashXfer.cmdType       = kFLEXSPI_Read;
    flashXfer.SeqNumber     = 1;
    flashXfer.seqIndex      = NOR_CMD_LUT_SEQ_IDX_READ;
    flashXfer.data          = (uint32_t *)(void *)buffer;
    flashXfer.dataSize      = length;

    status = FLEXSPI_TransferBlocking(EXAMPLE_FLEXSPI, &flashXfer);

    return status;
}
#elif defined(ISSI_AT25SFxxxA)||defined(ISSI_IS25LPxxxA)||defined(ISSI_IS25WPxxxA)||defined(WINBOND_W25QxxxJV)
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
#endif


status_t sfw_flash_read(uint32_t dstAddr, void *buf, size_t len)
{
	uint32_t addr = dstAddr | UPDATE_EXAMPLE_FLEXSPI_AMBA_BASE;

//	DCACHE_CleanInvalidateByRange(addr, len);
	memcpy(buf, (void *)addr, len);

	return 0;
}


status_t sfw_flash_init(void)
{
#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    bool DCacheEnableFlag = false;
    /* Disable D cache. */
    if (SCB_CCR_DC_Msk == (SCB_CCR_DC_Msk & SCB->CCR))
    {
        SCB_DisableDCache();
        DCacheEnableFlag = true;
    }
#endif /* __DCACHE_PRESENT */

    /* Update LUT table. */
    FLEXSPI_UpdateLUT(EXAMPLE_FLEXSPI, 0, &customLUT[0], CUSTOM_LUT_LENGTH);

    /* Do software reset. */
    FLEXSPI_SoftwareReset(EXAMPLE_FLEXSPI);

#if defined(__DCACHE_PRESENT) && (__DCACHE_PRESENT == 1U)
    if (DCacheEnableFlag)
    {
        /* Enable D cache. */
        SCB_EnableDCache();
    }
#endif /* __DCACHE_PRESENT */

    return kStatus_Success;
}
