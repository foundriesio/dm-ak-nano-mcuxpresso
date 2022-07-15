// SPDX-License-Identifier: BSD-3-Clause
/*
 * Copyright 2019-2021 NXP
 * All rights reserved.
 *
 */
// #include "sfw.h"

#define LIBRARY_LOG_LEVEL LOG_INFO
#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <stdio.h>

#include "aknano_priv.h"


#include "fsl_debug_console.h"
#include "flash_info.h"
#include "sbl_ota_flag.h"
#if defined(SOC_LPC55S69_SERIES)
#include "iap_flash_ops.h"
#else
#include "flexspi_flash_config.h"
#endif

const uint32_t boot_img_magic[] = {
    0xf395c277,
    0x7fefd260,
    0x0f505235,
    0x8079b62c,
};

void print_image_version(void)
{
    struct image_version *img_ver;
    uint8_t img_version[8];
    
    sfw_flash_read(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, img_version, 8);
    img_ver = (struct image_version *)img_version;
    
    LogInfo(("Current image verison: %d.%d.%d\r\n", img_ver->iv_major, img_ver->iv_minor, img_ver->iv_revision));
    vTaskDelay(1000);
}

#ifdef SOC_REMAP_ENABLE
struct remap_trailer s_remap_trailer;

uint8_t read_ota_status(void)
{
    struct remap_trailer remap_trailer;

    sfw_flash_read_ipc(REMAP_FLAG_ADDRESS, (uint8_t *)&remap_trailer, 32);

    if (remap_trailer.image_ok == 0xFF)
    {
        return 0x00;
    }
    else if (remap_trailer.image_ok == 0x04)
    {
        return 0x01;
    }
    else
    {
        return 0xFF;
    }    
}

void dump_remap_trailer()
{
    sfw_flash_read(REMAP_FLAG_ADDRESS, &s_remap_trailer, 32);
    LogInfo(("s_remap_trailer.image_position=%d", s_remap_trailer.image_position));
    LogInfo(("s_remap_trailer.image_ok=%d", s_remap_trailer.image_ok));
    LogInfo(("s_remap_trailer.magic %02X:%02X:%02X:%02X %02X:%02X:%02X:%02X %02X:%02X:%02X:%02X %02X:%02X:%02X:%02X",
        s_remap_trailer.magic[0],
        s_remap_trailer.magic[1],
        s_remap_trailer.magic[2],
        s_remap_trailer.magic[3],
        s_remap_trailer.magic[4],
        s_remap_trailer.magic[5],
        s_remap_trailer.magic[6],
        s_remap_trailer.magic[7],
        s_remap_trailer.magic[8],
        s_remap_trailer.magic[9],
        s_remap_trailer.magic[10],
        s_remap_trailer.magic[11],
        s_remap_trailer.magic[12],
        s_remap_trailer.magic[13],
        s_remap_trailer.magic[14],
        s_remap_trailer.magic[15]
        ));
}

bool is_image_confirmed(void)
{
    sfw_flash_read(REMAP_FLAG_ADDRESS, &s_remap_trailer, 32);
    LogInfo(("s_remap_trailer.image_ok=%d\r\n", s_remap_trailer.image_ok));
    return s_remap_trailer.image_ok == 0xFF;
}

/* write the remap image trailer */
int enable_image_and_set_boot_image_position(uint8_t imagePosition)
{
    status_t status;
    uint32_t primask;
    uint32_t write_buffer[FLASH_PAGE_SIZE / 4];

    memset(write_buffer, 0xFF, sizeof(write_buffer));
    mflash_drv_sector_erase(FLASH_AREA_IMAGE_1_OFFSET - SECTOR_SIZE);
    memset((void *)&s_remap_trailer, 0xff, IMAGE_TRAILER_SIZE);
    memcpy((void *)s_remap_trailer.magic, boot_img_magic, sizeof(boot_img_magic));
    s_remap_trailer.image_position = imagePosition;
    memcpy(((uint8_t*)write_buffer) + (FLASH_PAGE_SIZE - 32), &s_remap_trailer, 32);
    status = mflash_drv_page_program(FLASH_AREA_IMAGE_1_OFFSET - FLASH_PAGE_SIZE, (uint32_t*)write_buffer);
    if (status) 
    {
        LogError(("enable_image: failed to write remap flag\r\n"));
        return -1;
    }
    dump_remap_trailer();
    return 0;
}

int write_image_ok(void)
{
    status_t status;
    uint32_t write_buffer[SECTOR_SIZE / 4];
    
    sfw_flash_read(REMAP_FLAG_ADDRESS, &s_remap_trailer, 32);
    mflash_drv_sector_erase(FLASH_AREA_IMAGE_1_OFFSET - SECTOR_SIZE);
    memset((void *)s_remap_trailer.magic, 0xff, IMAGE_TRAILER_SIZE - 16);
    s_remap_trailer.image_ok = 0xFF;
    s_remap_trailer.pad1[3] = 0x0;
    memcpy(((uint8_t*)write_buffer) + FLASH_PAGE_SIZE - 32, &s_remap_trailer, 32);
    status = mflash_drv_page_program(FLASH_AREA_IMAGE_1_OFFSET - FLASH_PAGE_SIZE, (uint32_t*)write_buffer);
    if (status) 
    {
        return -1;
    }
    return 0;
}

void SBL_EnableRemap(uint32_t start_addr, uint32_t end_addr, uint32_t off)
{
    uint32_t * remap_start  = (uint32_t *)REMAPADDRSTART;
    uint32_t * remap_end    = (uint32_t *)REMAPADDREND;
    uint32_t * remap_offset = (uint32_t *)REMAPADDROFFSET;

#ifdef SOC_IMXRT1170_SERIES
    *remap_start = start_addr + 1;
#else
    *remap_start = start_addr;
#endif
    *remap_end = end_addr;
    *remap_offset = off;
}

void SBL_DisableRemap(void)
{
    uint32_t * remap_start  = (uint32_t *)REMAPADDRSTART;
    uint32_t * remap_end    = (uint32_t *)REMAPADDREND;
    uint32_t * remap_offset = (uint32_t *)REMAPADDROFFSET;

    *remap_start = 0;
    *remap_end = 0;
    *remap_offset = 0;
}

#else

#endif
