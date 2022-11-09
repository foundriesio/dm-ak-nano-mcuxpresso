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

/* offset needs to be alligned to MFLASH_SECTOR_SIZE (4K) */
status_t WriteDataToFlash(int offset, void *data, size_t data_len)
{
    size_t total_processed = 0;
    size_t chunk_len;
    unsigned char page_buffer[MFLASH_PAGE_SIZE];
    int32_t chunk_flash_addr = offset;
    // int32_t chunk_len;
    int32_t mflash_result;
    int32_t next_erase_addr = offset;
    status_t ret = 0;

    do
    {
        /* The data is epxected for be received by page sized chunks (except for the last one) */
        int remaining_bytes =  data_len - total_processed;
        if (remaining_bytes < MFLASH_PAGE_SIZE)
            chunk_len = remaining_bytes;
        else
            chunk_len = MFLASH_PAGE_SIZE;
        
        memcpy(page_buffer, data + total_processed, chunk_len);

        if (chunk_len > 0)
        {
            // if (chunk_flash_addr >= partition_phys_addr + partition_size)
            // {
            //     /* Partition boundary exceeded */
            //     LogError(("store_update_image: partition boundary exceedded"));
            //     retval = -1;
            //     break;
            // }

            /* Perform erase when encountering next sector */
            if (chunk_flash_addr >= next_erase_addr)
            {
                mflash_result = mflash_drv_sector_erase(AKNANO_STORAGE_FLASH_OFFSET + next_erase_addr);
                if (mflash_result != 0)
                {
                    LogError(("store_update_image: Error erasing sector %ld", mflash_result));
                    ret = -2;
                    break;
                }
                next_erase_addr += MFLASH_SECTOR_SIZE;
            }

            /* Clear the unused portion of the buffer (applicable to the last chunk) */
            if (chunk_len < MFLASH_PAGE_SIZE)
            {
                memset((uint8_t *)page_buffer + chunk_len, 0xFF, MFLASH_PAGE_SIZE - chunk_len);
            }

            /* Program the page */
            mflash_result = mflash_drv_page_program(AKNANO_STORAGE_FLASH_OFFSET + chunk_flash_addr, (uint32_t*)page_buffer);
            if (mflash_result != 0)
            {
                LogError(("store_update_image: Error storing page %ld", mflash_result));
                ret = -1;
                break;
            }

            total_processed += chunk_len;
            chunk_flash_addr += chunk_len;

            // LogInfo(("store_update_image: processed %i bytes", total_processed));
        }

    } while (chunk_len == MFLASH_PAGE_SIZE);
    
    return ret;
}
