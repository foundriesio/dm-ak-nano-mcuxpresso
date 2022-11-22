/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_LEVEL LOG_INFO

#include "aknano_secret.h"
#include "aknano_provisioning_secret.h"
#include "aknano_priv.h"

int aknano_gen_device_certificate_and_key(
					const char* uuid, const char* factory_name, 
					const char* serial_string, char* cert_buf, char* key_buf);

static char cert_buf[AKNANO_CERT_BUF_SIZE];
static char key_buf[AKNANO_CERT_BUF_SIZE];
#define FLASH_PAGE_SIZE 256
#define FLASH_SECTOR_SIZE 4096

static char temp_buf[FLASH_PAGE_SIZE];

// Sectors used for AKNANO data
#define AKNANO_FLASH_TOTAL_SECTORS 10

int aknano_reset_device_id()
{
    int offset;

    LogInfo(("Clearing device data from flash"));
    for (offset = 0; offset < AKNANO_FLASH_TOTAL_SECTORS * FLASH_SECTOR_SIZE; offset += FLASH_SECTOR_SIZE) {
        ClearFlashSector(offset);
    }
}

int aknano_gen_and_store_random_device_certificate_and_key()
{
    int ret, offset;
    
    char uuid_and_serial[FLASH_PAGE_SIZE];
    // char serial_string[FLASH_PAGE_SIZE];

    aknano_gen_serial_and_uuid(uuid_and_serial, uuid_and_serial+128);
    ret = aknano_gen_device_certificate_and_key(uuid_and_serial, AKNANO_FACTORY_NAME, 
                                          uuid_and_serial+128, cert_buf, key_buf);

    LogInfo(("aknano_gen_random_device_certificate_and_key ret=%d", ret));
    LogInfo(("cert_buf:\r\n%s", cert_buf));
    LogInfo(("key_buf:\r\n%s", key_buf));
    LogInfo(("uuid=%s", uuid_and_serial));
    LogInfo(("serial=%s", uuid_and_serial+128));

    if (ret != 0)
    {
        LogError(("Certificate generation error %d", ret));

        return ret;
    }
    LogError(("Certificate generation DONE. Returning"));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_UUID, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE uuid=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_SERIAL, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE serial=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE cert=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE cert=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_KEY, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("BEFORE key=%s", temp_buf));

    // Save Cert, Key, UUID and Serial to flash
    aknano_reset_device_id();

    for (offset = 0; offset < AKNANO_CERT_BUF_SIZE; offset += FLASH_PAGE_SIZE)
        WriteFlashPage(AKNANO_FLASH_OFF_DEV_CERTIFICATE + offset, cert_buf + offset);

    for (offset = 0; offset < AKNANO_CERT_BUF_SIZE; offset += FLASH_PAGE_SIZE)
        WriteFlashPage(AKNANO_FLASH_OFF_DEV_KEY + offset, key_buf + offset);

    WriteFlashPage(AKNANO_FLASH_OFF_DEV_UUID, uuid_and_serial);

    struct aknano_settings settings = { 0 };
    // Clear execution settings area of the flash
    char flashPageBuffer[256] = { 0 };
    UpdateFlashStoragePage(AKNANO_FLASH_OFF_STATE_BASE, flashPageBuffer);

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER [%x] cert=%s", temp_buf[0], temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_CERTIFICATE, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER cert=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_KEY, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER key=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_UUID, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER uuid=%s", temp_buf));

    // ReadFlashStorage(AKNANO_FLASH_OFF_DEV_SERIAL, temp_buf, FLASH_PAGE_SIZE);
    // LogInfo(("AFTER serial=%s", temp_buf));

    #ifdef AKNANO_ENABLE_SE05X
    LogInfo(("Provisioning Key and Certificate using PKCS#11 interface. Using SE05X"));
    #else
    LogInfo(("Provisioning Key and Certificate using PKCS#11 interface. Using flash device"));
    #endif
    vDevModeKeyProvisioning_new((uint8_t*)key_buf,
                                (uint8_t*)cert_buf );
    LogInfo(("Provisioning done"));
}
