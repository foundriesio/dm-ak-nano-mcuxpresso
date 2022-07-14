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

void write_update_type(uint8_t type)
{
    uint8_t write_buf;
    status_t status;
    uint32_t primask;
    
    write_buf = type;

    PRINTF("write update type = 0x%x\r\n", write_buf);

    primask = DisableGlobalIRQ();    
    status = sfw_flash_write(UPDATE_TYPE_FLAG_ADDRESS, &write_buf, 1);
    if (status) 
    {
        PRINTF("write update type: failed to write current update type\r\n");
        return;
    }
    EnableGlobalIRQ(primask);
}

void print_image_version(void)
{
    struct image_version *img_ver;
    uint8_t img_version[8];
    
    sfw_flash_read(FLASH_AREA_IMAGE_1_OFFSET + IMAGE_VERSION_OFFSET, img_version, 8);
    img_ver = (struct image_version *)img_version;
    
    LogInfo(("Current image verison: %d.%d.%d\r\n", img_ver->iv_major, img_ver->iv_minor, img_ver->iv_revision));
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

// FIXME: cleanup this

const uint32_t boot_remap_magic[] = {
    0xf395c277,
    0x7fefd260,
    0x0f505235,
    0x8079b62c,
};

#define REMAP_MAGIC_ARR_SZ \
    (sizeof boot_remap_magic / sizeof boot_remap_magic[0])

#define REMAP_TRAILER_SIZE     sizeof(struct remap_trailer)
      
const uint32_t REMAP_MAGIC_SZ = sizeof boot_remap_magic;

const uint32_t REMAP_MAX_ALIGN = BOOT_MAX_ALIGN;

static uint32_t
remap_image_position_off(void)
{
    // assert(offsetof(struct remap_trailer, image_position) == 0);
    return BOOT_FLASH_ACT_APP - REMAP_MAGIC_SZ - REMAP_MAX_ALIGN * 2;
}

void set_boot_image_position(uint8_t imagePosition)
{
    uint32_t allignedImagePosition = imagePosition;
    int ret = flexspi_nor_flash_program(FLEXSPI, remap_image_position_off() - BOOT_FLASH_BASE, &allignedImagePosition, 1);
    LogInfo(("imagePosition imagePosition=%d ret = %d\r\n", imagePosition, ret));
}

bool is_image_confirmed(void)
{
    sfw_flash_read(REMAP_FLAG_ADDRESS, &s_remap_trailer, 32);
    LogInfo(("s_remap_trailer.image_ok=%d\r\n", s_remap_trailer.image_ok));
    return s_remap_trailer.image_ok == 0xFF;
}

/* write the remap image trailer */
int enable_image(void)
{
    uint32_t off;
    status_t status;
    uint32_t primask;
 
    sfw_flash_read(REMAP_FLAG_ADDRESS, &s_remap_trailer, 32);
    
    primask = DisableGlobalIRQ();
    status = sfw_flash_erase(FLASH_AREA_IMAGE_1_OFFSET - SECTOR_SIZE, SECTOR_SIZE);
   
    EnableGlobalIRQ(primask);

    memset((void *)&s_remap_trailer, 0xff, IMAGE_TRAILER_SIZE);
    memcpy((void *)s_remap_trailer.magic, boot_img_magic, sizeof(boot_img_magic));

    off = REMAP_FLAG_ADDRESS + 16;
    
    primask = DisableGlobalIRQ();
    status = sfw_flash_write(off, (void *)&s_remap_trailer.magic, IMAGE_TRAILER_SIZE - 16);
    if (status) 
    {
        LogError(("enable_image: failed to write remap flag\r\n"));
        return -1;
    }
    EnableGlobalIRQ(primask);
    return 0;
}

int write_image_ok(void)
{
    uint32_t off;
    status_t status;
    uint32_t primask;
    
    sfw_flash_read(REMAP_FLAG_ADDRESS, &s_remap_trailer, 32);
    
    primask = DisableGlobalIRQ();
    status = sfw_flash_erase(FLASH_AREA_IMAGE_1_OFFSET - SECTOR_SIZE, SECTOR_SIZE);
    
    EnableGlobalIRQ(primask);
    
    memset((void *)s_remap_trailer.magic, 0xff, IMAGE_TRAILER_SIZE - 16);

    s_remap_trailer.image_ok = 0xFF;
    s_remap_trailer.pad1[3] = 0x0;
    
    off = REMAP_FLAG_ADDRESS;

    LogInfo(("Write OK flag: off = 0x%x\r\n", off));
    
    primask = DisableGlobalIRQ();
    status = sfw_flash_write(off, (void *)&s_remap_trailer, IMAGE_TRAILER_SIZE);
    if (status) 
    {
        return -1;
    }
    EnableGlobalIRQ(primask);
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
uint8_t read_ota_status(void)
{
    uint32_t off;
    struct swap_trailer swap_trailer;

    off = FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE - IMAGE_TRAILER_SIZE;
    
    sfw_flash_read(off, &swap_trailer, 32);

    if (swap_trailer.copy_done == 0xFF)
    {
        return 0x00;
    }
    else if (swap_trailer.copy_done == 0x01)
    {
        return 0x01;
    }
    else
    {
        return 0xFF;
    }      
}

struct swap_trailer s_swap_trailer;
/* write the image trailer at the end of the flash partition */
int enable_image(void)
{
    uint32_t off;
    status_t status;
    uint32_t primask;
#ifdef SOC_LPC55S69_SERIES
    /* The flash of LPC55xx have the limit of offset when do write operation*/
    uint8_t write_buff[512];
    memset(write_buff, 0xff, 512);
#endif    
    
    memset((void *)&s_swap_trailer, 0xff, IMAGE_TRAILER_SIZE);
    memcpy((void *)s_swap_trailer.magic, boot_img_magic, sizeof(boot_img_magic));

#ifdef SOC_LPC55S69_SERIES
    memcpy(&write_buff[512 - IMAGE_TRAILER_SIZE], (void *)&s_swap_trailer, IMAGE_TRAILER_SIZE);
    off = FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - 512;
#else
    off = FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE - IMAGE_TRAILER_SIZE;
#endif

    PRINTF("write magic number offset = 0x%x\r\n", off);

    primask = DisableGlobalIRQ();
#ifdef SOC_LPC55S69_SERIES
    status = sfw_flash_write(off, write_buff, 512);
#else
    // FIXME
    status = flexspi_nor_flash_program(EXAMPLE_FLEXSPI, off, (void *)&s_swap_trailer, IMAGE_TRAILER_SIZE);
    // status = sfw_flash_write(off, (void *)&s_swap_trailer, IMAGE_TRAILER_SIZE);
#endif
    if (status) 
    {
        PRINTF("enable_image: failed to write trailer2\r\n");
        return -1;
    }
    EnableGlobalIRQ(primask);
    return 0;
}
int write_image_ok(void)
{
    uint32_t off;
    status_t status;
    uint32_t primask;
#ifdef SOC_LPC55S69_SERIES
    /* The flash of LPC55xx have the limit of offset when do write operation*/
    static uint8_t write_buff[512];
    memset(write_buff, 0xff, 512);
#endif    
    /* Erase update type flag */
    primask = DisableGlobalIRQ();
#if !defined(SOC_LPC55S69_SERIES)
    status = sfw_flash_erase(FLASH_AREA_IMAGE_1_OFFSET - SECTOR_SIZE, SECTOR_SIZE);
#endif
    
    EnableGlobalIRQ(primask);    

    memset((void *)&s_swap_trailer, 0xff, IMAGE_TRAILER_SIZE);
    memcpy((void *)s_swap_trailer.magic, boot_img_magic, sizeof(boot_img_magic));
    
    s_swap_trailer.image_ok= BOOT_FLAG_SET;

#ifdef SOC_LPC55S69_SERIES
    off = FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE - 512;
    sfw_flash_read(off, write_buff, 512);
    memcpy(&write_buff[512 - IMAGE_TRAILER_SIZE], (void *)&s_swap_trailer, IMAGE_TRAILER_SIZE);
#else
    off = FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE - IMAGE_TRAILER_SIZE;
#endif

    PRINTF("Write OK flag: off = 0x%x\r\n", off);
    
    primask = DisableGlobalIRQ();
#ifdef SOC_LPC55S69_SERIES
    sfw_flash_erase(off, 512);
    status = sfw_flash_write(off, write_buff, 512);
#else
    status = sfw_flash_erase(FLASH_AREA_IMAGE_2_OFFSET - SECTOR_SIZE, SECTOR_SIZE);
    status = sfw_flash_write(off, (void *)&s_swap_trailer, IMAGE_TRAILER_SIZE);
#endif
    if (status) 
    {
        return;
    }
    EnableGlobalIRQ(primask);
}
#endif
