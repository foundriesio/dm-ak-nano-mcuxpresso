#include "logging_levels.h"
#define LIBRARY_LOG_NAME "aknano_flash"
#define LIBRARY_LOG_LEVEL LOG_INFO
#include "logging_stack.h"

#include "mflash_common.h"
#include "mflash_drv.h"

#define AKNANO_STORAGE_FLASH_OFFSET 0x600000

status_t InitFlashStorage()
{
    int mflash_result = mflash_drv_init();
    if (mflash_result != 0)
    {
        LogError(("mflash_drv_init error %d", mflash_result));
        return -1;
    }
    return 0;
}

status_t ReadFlashStorage(int offset, void *output, size_t outputMaxLen)
{
    int mflash_result = mflash_drv_read(AKNANO_STORAGE_FLASH_OFFSET + offset, output, outputMaxLen / 4 * 4);
    if (mflash_result != 0)
    {
        LogError(("ReadFlashStorage: mflash_drv_read error %d", mflash_result));
        return -1;
    }
    return 0;
}

// Data needs to be a 256 bytes array
status_t UpdateFlashStoragePage(int offset, void *data)
{
    int mflash_result = mflash_drv_sector_erase(AKNANO_STORAGE_FLASH_OFFSET + offset);
    if (mflash_result != 0)
    {
        LogError(("UpdateFlashStoragePage: mflash_drv_sector_erase error %d", mflash_result));
        return -1;
    }

    mflash_result = mflash_drv_page_program(AKNANO_STORAGE_FLASH_OFFSET + offset, data);
    if (mflash_result != 0)
    {
        LogError(("UpdateFlashStoragePage: mflash_drv_page_program error %d", mflash_result));
        return -1;
    }

    return 0;
}
