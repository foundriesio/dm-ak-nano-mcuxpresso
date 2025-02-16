/*
 * Copyright 2021 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MCUBOOT_APP_SUPPORT_H__
#define __MCUBOOT_APP_SUPPORT_H__

#define CONFIG_MCUBOOT_FLASH_REMAP_ENABLE

#include "fsl_common.h"
#include "flash_partitioning.h"
#include "image.h"

#define FLASH_AREA_IMAGE_1_OFFSET (BOOT_FLASH_ACT_APP - BOOT_FLASH_BASE)
#define FLASH_AREA_IMAGE_1_SIZE   (BOOT_FLASH_CAND_APP - BOOT_FLASH_ACT_APP)
#define FLASH_AREA_IMAGE_2_OFFSET (FLASH_AREA_IMAGE_1_OFFSET + FLASH_AREA_IMAGE_1_SIZE)
#define FLASH_AREA_IMAGE_2_SIZE   FLASH_AREA_IMAGE_1_SIZE // image2 slot is the same size as image1
#define FLASH_AREA_IMAGE_3_OFFSET (FLASH_AREA_IMAGE_2_OFFSET + FLASH_AREA_IMAGE_2_SIZE)

#define IMAGE_MAGIC       0x96f3b83d
#define IMAGE_HEADER_SIZE 32


#define IMAGE_TLV_INFO_MAGIC      0x6907
#define IMAGE_TLV_PROT_INFO_MAGIC 0x6908


#define BOOT_MAX_ALIGN 8
#define BOOT_FLAG_SET  1

struct image_trailer
{
    uint8_t swap_type;
    uint8_t pad1[BOOT_MAX_ALIGN - 1];
    uint8_t copy_done;
    uint8_t pad2[BOOT_MAX_ALIGN - 1];
    uint8_t image_ok;
    uint8_t pad3[BOOT_MAX_ALIGN - 1];
    uint8_t magic[16];
};

/* Bootloader helper API */
enum
{
    kSwapType_None,         // Default value when there is no upgradable image
    kSwapType_ReadyForTest, // The application needs to switch to this state when finishing the update operation
    kSwapType_Testing,      // The bootloader needs to switch to this state before running the test image
    kSwapType_Permanent,    // The application needs to switch to this state when the self-test is okay
    kSwapType_Fail,
    kSwapType_Fatal,
    kSwapType_Max,
};

typedef struct
{
    uint32_t start;
    uint32_t size;
} partition_t;


extern int32_t bl_verify_image(const uint8_t *data, uint32_t size);

extern status_t bl_get_update_partition_info(partition_t *ptn);
extern status_t bl_update_image_state(uint32_t state);
extern status_t bl_get_image_state(uint32_t *state);

status_t bl_get_image_build_num(uint32_t *iv_build_num, uint8_t image_position);

uint32_t get_active_image(void);

#endif
