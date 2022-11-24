/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#ifdef AKNANO_ALLOW_PROVISIONING
#define LIBRARY_LOG_LEVEL LOG_INFO

#include "aknano_secret.h"
#include "aknano_provisioning_secret.h"
#include "aknano_priv.h"

int aknano_gen_device_certificate_and_key(
					const char* uuid, const char* factory_name, 
					const char* serial_string, unsigned char* cert_buf, unsigned char* key_buf);

static unsigned char cert_buf[AKNANO_CERT_BUF_SIZE];
static unsigned char key_buf[AKNANO_CERT_BUF_SIZE];
#define FLASH_PAGE_SIZE 256
#define FLASH_SECTOR_SIZE 4096

// Sectors used for AKNANO data
#define AKNANO_FLASH_SECTORS_COUNT 10

int aknano_clear_provisioned_data()
{
    int offset;

    LogInfo(("Clearing provisioned device data from flash"));
    for (offset = 0; offset < AKNANO_FLASH_SECTORS_COUNT * FLASH_SECTOR_SIZE; offset += FLASH_SECTOR_SIZE) {
        ClearFlashSector(offset);
    }
    return 0;
}
int aknano_provision_device()
{
    /* 
     * When EdgeLock 2GO is used, this function only generates the device serial 
     *  and UUID
     */

    int ret = 0;
    int offset;
    char uuid_and_serial[FLASH_PAGE_SIZE];

    aknano_gen_serial_and_uuid(uuid_and_serial, uuid_and_serial+128);
    LogInfo(("uuid=%s", uuid_and_serial));
    LogInfo(("serial=%s", uuid_and_serial+128));

#ifndef AKNANO_ENABLE_EL2GO
    ret = aknano_gen_device_certificate_and_key(uuid_and_serial, AKNANO_FACTORY_NAME, 
                                          uuid_and_serial+128, cert_buf, key_buf);
    LogInfo(("aknano_gen_random_device_certificate_and_key ret=%d", ret));
    LogInfo(("cert_buf:\r\n%s", cert_buf));
    LogInfo(("key_buf:\r\n%s", key_buf));

    if (ret != 0) {
        LogError(("Certificate generation error %d", ret));
        return ret;
    }
#endif

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
    aknano_clear_provisioned_data();

    WriteFlashPage(AKNANO_FLASH_OFF_DEV_UUID, uuid_and_serial);
#ifndef AKNANO_ENABLE_EL2GO
    for (offset = 0; offset < AKNANO_CERT_BUF_SIZE; offset += FLASH_PAGE_SIZE)
        WriteFlashPage(AKNANO_FLASH_OFF_DEV_CERTIFICATE + offset, cert_buf + offset);

    for (offset = 0; offset < AKNANO_CERT_BUF_SIZE; offset += FLASH_PAGE_SIZE)
        WriteFlashPage(AKNANO_FLASH_OFF_DEV_KEY + offset, key_buf + offset);
#endif

    // struct aknano_settings settings = { 0 };
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

#ifndef AKNANO_ENABLE_EL2GO
    #ifdef AKNANO_ENABLE_SE05X
    LogInfo(("Provisioning Key and Certificate using PKCS#11 interface. Using SE05X"));
    #else
    LogInfo(("Provisioning Key and Certificate using PKCS#11 interface. Using flash device"));
    #endif
    ret = vDevModeKeyProvisioning_new((uint8_t*)key_buf,
                                (uint8_t*)cert_buf );
    LogInfo(("Provisioning done"));
#endif
    return ret;
}
#endif
