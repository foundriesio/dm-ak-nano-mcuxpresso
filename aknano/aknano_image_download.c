/*
 * Copyright 2022 Foundries.io
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#define LIBRARY_LOG_LEVEL LOG_INFO

/*Include backoff algorithm header for retry logic.*/
// #include "backoff_algorithm.h"

#include "mbedtls/sha256.h"

/* Transport interface include. */
#include "transport_interface.h"

/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

#include "mcuboot_app_support.h"
#include "image.h"
#include "mflash_common.h"
#include "mflash_drv.h"

#include "aknano_priv.h"
#include "flexspi_flash_config.h"

#include <stdio.h>
#include <time.h>

#define AKNANO_REQUEST_BODY ""
#define AKNANO_REQUEST_BODY_LEN sizeof(AKNANO_REQUEST_BODY)-1
#define AKNANO_DOWNLOAD_ENDPOINT AKNANO_FACTORY_UUID ".ostree.foundries.io"
#define AKNANO_DOWNLOAD_PORT AKNANO_DEVICE_GATEWAY_PORT

#include "aknano_secret.h"
static const char donwloadServer_ROOT_CERTIFICATE_PEM[] = AKNANO_DEVICE_GATEWAY_CERTIFICATE;

#define AKNANO_DOWNLOAD_ENDPOINT_LEN sizeof(AKNANO_DOWNLOAD_ENDPOINT)-1

static const uint32_t donwloadServer_ROOT_CERTIFICATE_LENGTH = sizeof( donwloadServer_ROOT_CERTIFICATE_PEM );


static BaseType_t prvConnectToDownloadServer( NetworkContext_t * pxNetworkContext )
{
    ServerInfo_t xServerInfo = { 0 };
    SocketsConfig_t xSocketsConfig = { 0 };
    BaseType_t xStatus = pdPASS;
    TransportSocketStatus_t xNetworkStatus = TRANSPORT_SOCKET_STATUS_SUCCESS;

    /* Initializer server information. */
    xServerInfo.pHostName = AKNANO_DOWNLOAD_ENDPOINT;
    xServerInfo.hostNameLength = AKNANO_DOWNLOAD_ENDPOINT_LEN;
    xServerInfo.port = AKNANO_DOWNLOAD_PORT;

    /* Configure credentials for TLS mutual authenticated session. */
    xSocketsConfig.enableTls = true;
    xSocketsConfig.pAlpnProtos = NULL;
    xSocketsConfig.maxFragmentLength = 0;
    xSocketsConfig.disableSni = false;
    xSocketsConfig.pRootCa = donwloadServer_ROOT_CERTIFICATE_PEM;
    xSocketsConfig.rootCaSize = donwloadServer_ROOT_CERTIFICATE_LENGTH;
    xSocketsConfig.sendTimeoutMs = 1000;
    xSocketsConfig.recvTimeoutMs = 1000;

    /* Establish a TLS session with the HTTP server. This example connects to
     * the HTTP server as specified in democonfigAWS_IOT_ENDPOINT and
     * democonfigAWS_HTTP_PORT in http_demo_mutual_auth_config.h. */
    LogInfo( ( "Establishing a TLS session to %.*s:%d.",
               ( int32_t ) xServerInfo.hostNameLength,
               xServerInfo.pHostName,
               xServerInfo.port ) );

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = SecureSocketsTransport_Connect( pxNetworkContext,
                                                     &xServerInfo,
                                                     &xSocketsConfig );

    if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
    {
        LogError(("Error connecting to binaries download server. Result=%d", xNetworkStatus));
        xStatus = pdFAIL;
    }
    LogInfo(("TLS session to binaries download server established"));

    return xStatus;
}

static int HandleReceivedData(const unsigned char* data, int offset, int dataLen, uint32_t partition_log_addr, uint32_t partition_size)
{
#ifdef AKNANO_DRY_RUN
    LogInfo(("** Dry run mode, skipping flash operations"));
    return 0;
#endif
    int32_t retval = 0;

    uint32_t partition_phys_addr;

    uint32_t chunk_flash_addr;
    int32_t chunk_len;

    uint32_t next_erase_addr;

    /* Page buffers */
    uint32_t page_size;

    /* Received/processed data counter */
    uint32_t total_processed = 0;

    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;

    unsigned char page_buffer[MFLASH_PAGE_SIZE];

    LogInfo(("Writing image chunk to flash. offset=%d len=%d", offset, dataLen));
    page_size   = MFLASH_PAGE_SIZE;

    /* Obtain physical address of FLASH to perform operations with */
    // partition_phys_addr = mflash_drv_log2phys((void *)partition_log_addr, partition_size);
    partition_phys_addr = partition_log_addr - BOOT_FLASH_BASE;
    if (partition_phys_addr == MFLASH_INVALID_ADDRESS) {
        LogError(("HandleReceivedData: Invalid partition_log_addr=0x%X", (int)partition_log_addr));
        return -1;
    }

    if (!mflash_drv_is_sector_aligned(partition_phys_addr) || !mflash_drv_is_sector_aligned(partition_size))
    {
        LogError(("store_update_image: partition not aligned"));
        return -1;
    }

    /* Pre-set address of area not erased so far */
    next_erase_addr  = partition_phys_addr + offset;
    chunk_flash_addr = partition_phys_addr + offset;
    total_processed  = 0;

    do
    {
        /* The data is epxected for be received by page sized chunks (except for the last one) */
        int remaining_bytes =  dataLen - total_processed;
        if (remaining_bytes < MFLASH_PAGE_SIZE)
            chunk_len = remaining_bytes;
        else
            chunk_len = MFLASH_PAGE_SIZE;
        
        memcpy(page_buffer, data + total_processed, chunk_len);

        if (chunk_len > 0)
        {
            if (chunk_flash_addr >= partition_phys_addr + partition_size)
            {
                /* Partition boundary exceeded */
                LogError(("store_update_image: partition boundary exceedded"));
                retval = -1;
                break;
            }

            /* Perform erase when encountering next sector */
            if (chunk_flash_addr >= next_erase_addr)
            {
                mflash_result = mflash_drv_sector_erase(next_erase_addr);
                if (mflash_result != 0)
                {
                    LogError(("store_update_image: Error erasing sector %ld", mflash_result));
                    break;
                }
                next_erase_addr += MFLASH_SECTOR_SIZE;
            }

            /* Clear the unused portion of the buffer (applicable to the last chunk) */
            if (chunk_len < page_size)
            {
                memset((uint8_t *)page_buffer + chunk_len, 0xff, page_size - chunk_len);
            }

            /* Program the page */
            mflash_result = mflash_drv_page_program(chunk_flash_addr, (uint32_t*)page_buffer);
            if (mflash_result != 0)
            {
                LogError(("store_update_image: Error storing page %ld", mflash_result));
                break;
            }

            total_processed += chunk_len;
            chunk_flash_addr += chunk_len;

            // LogInfo(("store_update_image: processed %i bytes", total_processed));
        }

    } while (chunk_len == page_size);
    // LogInfo(("Chunk writen. offset=%d len=%d total_processed=%ld", offset, dataLen, total_processed));
    return retval;
}


BaseType_t GetFileSize(size_t* pxFileSize, HTTPResponse_t *xResponse)
{
    /* The location of the file size in pcContentRangeValStr. */
    char * pcFileSizeStr = NULL;

    /* String to store the Content-Range header value. */
    char * pcContentRangeValStr = NULL;
    size_t xContentRangeValStrLength = 0;

    HTTPStatus_t xHTTPStatus = HTTPClient_ReadHeader( xResponse,
                                             ( char * ) httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD,
                                             ( size_t ) httpexampleHTTP_CONTENT_RANGE_HEADER_FIELD_LENGTH,
                                             ( const char ** ) &pcContentRangeValStr,
                                             &xContentRangeValStrLength );

    if( xHTTPStatus != HTTPSuccess )
    {
        LogError( ( "Failed to read Content-Range header from HTTP response: Error=%s.",
                    HTTPClient_strerror( xHTTPStatus ) ) );
        return pdFAIL;
    }

    /* Parse the Content-Range header value to get the file size. */
    pcFileSizeStr = strstr( pcContentRangeValStr, "/" );

    if( pcFileSizeStr == NULL )
    {
        LogError( ( "'/' not present in Content-Range header value: %s.",
                    pcContentRangeValStr ) );
        return pdFAIL;
    }

    pcFileSizeStr += sizeof( char );
    *pxFileSize = ( size_t ) strtoul( pcFileSizeStr, NULL, 10 );

    if( ( *pxFileSize == 0 ) || ( *pxFileSize == UINT32_MAX ) )
    {
        LogError( ( "Error using strtoul to get the file size from %s: xFileSize=%d.",
                    pcFileSizeStr, ( int ) *pxFileSize ) );
        return pdFAIL;
    }

    LogInfo( ( "The file is %d bytes long.", ( int ) *pxFileSize ) );
    return pdPASS;
}

static BaseType_t prvDownloadFile(NetworkContext_t *pxNetworkContext,
                                 const TransportInterface_t * pxTransportInterface,
                                 const char * pcPath, uint8_t image_position, struct aknano_context *aknano_context)
{
    /* Return value of this method. */
    BaseType_t xStatus = pdFAIL;
    HTTPStatus_t xHTTPStatus = HTTPSuccess;

    /* Configurations of the initial request headers that are passed to
    * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo;
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t xResponse;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders;

    /* The size of the file we are trying to download . */
    size_t xFileSize = 0;

    /* The number of bytes we want to request with in each range of the file
     * bytes. */
    size_t xNumReqBytes = 0;
    /* xCurByte indicates which starting byte we want to download next. */
    size_t xCurByte = 0;
    uint32_t dstAddr;
    uint8_t sha256_bytes[AKNANO_SHA256_LEN];

    if(image_position == 0x01) {
        dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
    } else if(image_position == 0x02) {
        dstAddr = FLASH_AREA_IMAGE_1_OFFSET;
    } else {
        dstAddr = FLASH_AREA_IMAGE_2_OFFSET;
    }

    configASSERT( pcPath != NULL );


    /* Initialize all HTTP Client library API structs to 0. */
    ( void ) memset( &xRequestHeaders, 0, sizeof( xRequestHeaders ) );
    ( void ) memset( &xRequestInfo, 0, sizeof( xRequestInfo ) );
    ( void ) memset( &xResponse, 0, sizeof( xResponse ) );

    xResponse.getTime = AkNanoGetTime; //nondet_boot();// ? NULL : GetCurrentTimeStub;

    /* Initialize the request object. */
    xRequestInfo.pHost = AKNANO_DOWNLOAD_ENDPOINT;
    xRequestInfo.hostLen = AKNANO_DOWNLOAD_ENDPOINT_LEN;
    xRequestInfo.pMethod = HTTP_METHOD_GET;
    xRequestInfo.methodLen = httpexampleHTTP_METHOD_GET_LENGTH;
    xRequestInfo.pPath = pcPath;
    xRequestInfo.pathLen = strlen( pcPath );

    /* Set "Connection" HTTP header to "keep-alive" so that multiple requests
     * can be sent over the same established TCP connection. This is done in
     * order to download the file in parts. */
    // Current server closes the connection anyway, and reports an error. So just disable the line bellow
    // xRequestInfo.reqFlags = HTTP_REQUEST_KEEP_ALIVE_FLAG;

    /* Set the buffer used for storing request headers. */
    xRequestHeaders.pBuffer = ucUserBuffer;
    xRequestHeaders.bufferLen = democonfigUSER_BUFFER_LENGTH;

    /* Initialize the response object. The same buffer used for storing request
     * headers is reused here. */
    xResponse.pBuffer = ucUserBuffer;
    xResponse.bufferLen = democonfigUSER_BUFFER_LENGTH;

    xNumReqBytes = democonfigRANGE_REQUEST_LENGTH;
    xStatus = pdPASS;

    int32_t stored = 0;
    /* Initialize SHA256 calculation for FW image */
    mbedtls_sha256_init(&aknano_context->sha256_context);
    mbedtls_sha256_starts(&aknano_context->sha256_context, 0);

    xFileSize = aknano_context->selected_target.expected_size;
    /* Here we iterate sending byte range requests until the full file has been
     * downloaded. We keep track of the next byte to download with xCurByte, and
     * increment by xNumReqBytes after each iteration. When xCurByte reaches
     * xFileSize, we stop downloading. */
    while( ( xStatus == pdPASS ) && ( xHTTPStatus == HTTPSuccess ) && ( xCurByte < xFileSize ) )
    {

        xHTTPStatus = HTTPClient_InitializeRequestHeaders( &xRequestHeaders,
                                                           &xRequestInfo );

        if( xHTTPStatus == HTTPSuccess )
        {
            xHTTPStatus = HTTPClient_AddRangeHeader( &xRequestHeaders,
                                                     xCurByte,
                                                     xCurByte + xNumReqBytes - 1 );
        }
        else
        {
            LogError( ( "Failed to initialize HTTP request headers: Error=%s.",
                        HTTPClient_strerror( xHTTPStatus ) ) );
        }

        if( xHTTPStatus == HTTPSuccess )
        {
            LogInfo( ( ANSI_COLOR_GREEN "Downloading new image. Retrieving bytes %d-%d of %d" ANSI_COLOR_RESET,
                       ( int ) ( xCurByte ),
                       ( int ) ( xCurByte + xNumReqBytes - 1 ),
                       ( int ) xFileSize));
            LogDebug( ( "Request Headers:\n%.*s",
                        ( int ) xRequestHeaders.headersLen,
                        ( char * ) xRequestHeaders.pBuffer ) );
            xHTTPStatus = HTTPClient_Send( pxTransportInterface,
                                           &xRequestHeaders,
                                           NULL,
                                           0,
                                           &xResponse,
                                           0 );
        }
        else
        {
            LogError( ( "Failed to add Range header to request headers: Error=%s.",
                        HTTPClient_strerror( xHTTPStatus ) ) );
        }

        if( xHTTPStatus == HTTPSuccess )
        {
            LogDebug( ( "Received HTTP response from %s %s...",
                        "binary download server", pcPath ) );
            LogDebug( ( "Response Headers:\n%.*s",
                        ( int ) xResponse.headersLen,
                        xResponse.pHeaders ) );
            LogDebug( ( "Response Body Len: %d",
                       ( int ) xResponse.bodyLen) );
            if (xStatus == pdPASS) {
                // FIXME
                #define MAX_FIRMWARE_SIZE 0x100000

                mbedtls_sha256_update(&aknano_context->sha256_context, xResponse.pBody, xResponse.bodyLen);
                if (HandleReceivedData(xResponse.pBody, xCurByte, xResponse.bodyLen, dstAddr + BOOT_FLASH_BASE, MAX_FIRMWARE_SIZE) < 0)
                {
                    LogError(("Error during HandleReceivedData"));
                    xStatus = pdFAIL;
                    break;
                }

                stored += xResponse.bodyLen;

                /* We increment by the content length because the server may not
                * have sent us the range we requested. */
                xCurByte += xResponse.bodyLen;

                if( ( xFileSize - xCurByte ) < xNumReqBytes )
                {
                    xNumReqBytes = xFileSize - xCurByte;
                }
            }

            xStatus = ( xResponse.statusCode == httpexampleHTTP_STATUS_CODE_PARTIAL_CONTENT ) ? pdPASS : pdFAIL;
        }
        else
        {
            // It is normal to get in here, because of the current server sinalization that the connection will be closed
            LogError( ( "An error occurred in downloading the file. "
                        "Failed to send HTTP GET request to %s %s: Error=%s.",
                        "binary download server", pcPath, HTTPClient_strerror( xHTTPStatus ) ) );
        }

        if( xStatus != pdPASS )
        {
            LogError( ( "Received an invalid response from the server "
                        "(Status Code: %u).",
                        xResponse.statusCode ) );
        }
    }
    LogInfo(("Disconnecting"));
    SecureSocketsTransport_Disconnect( pxNetworkContext );

    if (stored != aknano_context->selected_target.expected_size) {
        LogInfo( ( ANSI_COLOR_MAGENTA "Actual file size (%ld bytes) does not match expected size (%ld bytes)" ANSI_COLOR_RESET, stored, aknano_context->selected_target.expected_size));
        xStatus = pdFAIL;
    } else {
        LogInfo( ( ANSI_COLOR_MAGENTA "Actual file size (%ld bytes) matches expected size (%ld bytes)" ANSI_COLOR_RESET, stored, aknano_context->selected_target.expected_size));
    }

    if (( xStatus == pdPASS ) && ( xHTTPStatus == HTTPSuccess )) {
        mbedtls_sha256_finish_ret(&aknano_context->sha256_context, sha256_bytes);
        if (memcmp(&aknano_context->selected_target.expected_hash, sha256_bytes, sizeof(sha256_bytes))) {
            LogInfo((ANSI_COLOR_RED "Downloaded image SHA256 does not match the expected value" ANSI_COLOR_RESET));
            return false;
        } else {
            LogInfo((ANSI_COLOR_MAGENTA "Downloaded image SHA256 matches the expected value" ANSI_COLOR_RESET));
        }

#ifndef AKNANO_DRY_RUN
        partition_t update_partition;
        if (bl_get_update_partition_info(&update_partition) != kStatus_Success)
        {
            /* Could not get update partition info */
            LogError(("Could not get update partition info"));
            return pdFAIL;
        }
        LogInfo(("Validating image of size %d", stored));

        struct image_header *ih;
        ih = (struct image_header *)update_partition.start;
        if (bl_verify_image((void *)update_partition.start, stored) <= 0)
        {
            /* Image validation failed */
            LogError(("Image validation failed magic=0x%X", ih->ih_magic));
            return false;
        } else {
            LogInfo(("Image validation succeded"));
            return true;
        }
#endif
    } else {
        return false;
    }
}

int AkNanoDownloadAndFlashImage(struct aknano_context *aknano_context)
{
    /* The transport layer interface used by the HTTP Client library. */
    TransportInterface_t xTransportInterface;
    /* The network context for the transport layer interface. */
    NetworkContext_t xNetworkContext = { 0 };
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };

    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToDownloadServer(&xNetworkContext);
    // xDemoStatus = connectToServerWithBackoffRetries( prvConnectToServer,
    //                                                     &xNetworkContext );

    LogInfo(("AkNanoDownloadAndFlashImage: prvConnectToServer Result: %ld", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        /* Define the transport interface. */
        xTransportInterface.pNetworkContext = &xNetworkContext;
        xTransportInterface.send = SecureSocketsTransport_Send;
        xTransportInterface.recv = SecureSocketsTransport_Recv;
    }
    else
    {
        /* Log error to indicate connection failure after all
            * reconnect attempts are over. */
        LogError( ( "Failed to connect to HTTP server") );
    }

    if( xDemoStatus == pdPASS )
    {
        uint8_t *h = aknano_context->selected_target.expected_hash;
        char relative_path[AKNANO_MAX_URI_LENGTH];
        sprintf(relative_path, "/mcu/files/"
        "%02x/%02x%02x%02x%02x%02x%02x%02x"
        "%02x%02x%02x%02x%02x%02x%02x%02x"
        "%02x%02x%02x%02x%02x%02x%02x%02x"
        "%02x%02x%02x%02x%02x%02x%02x%02x"
        ".bin",
        *(h+0), *(h+1), *(h+2), *(h+3), *(h+4), *(h+5), *(h+6), *(h+7),
        *(h+8), *(h+9), *(h+10), *(h+11), *(h+12), *(h+13), *(h+14), *(h+15),
        *(h+16),*(h+17), *(h+18), *(h+19), *(h+20), *(h+21), *(h+22), *(h+23),
        *(h+24), *(h+25), *(h+26), *(h+27), *(h+28), *(h+29), *(h+30), *(h+31));
        LogInfo(("Download relative path=%s", relative_path));
        xDemoStatus = prvDownloadFile(&xNetworkContext, &xTransportInterface, 
            relative_path, aknano_context->settings->image_position, aknano_context);
    }

    /**************************** Disconnect. ******************************/

    /* Close the network connection to clean up any system resources that the
        * demo may have consumed. */
    return xDemoStatus;
}
