#define LIBRARY_LOG_LEVEL LOG_INFO

/*Include backoff algorithm header for retry logic.*/
// #include "backoff_algorithm.h"

/* Transport interface include. */
#include "transport_interface.h"

/* Transport interface implementation include header for TLS. */
#include "transport_secure_sockets.h"

#include "mcuboot_app_support.h"
#include "mflash_common.h"
#include "mflash_drv.h"

#include "aknano_priv.h"


#define AKNANO_DOWNLOAD_ENDPOINT "detsch.gobolinux.org"

#define AKNANO_DOWNLOAD_ENDPOINT_LEN sizeof(AKNANO_DOWNLOAD_ENDPOINT)-1


#define AKNANO_REQUEST_BODY ""
#define AKNANO_REQUEST_BODY_LEN sizeof(AKNANO_REQUEST_BODY)-1

static const char donwloadServer_ROOT_CERTIFICATE_PEM[] =
"-----BEGIN CERTIFICATE-----\n"
"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\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIFFjCCAv6gAwIBAgIRAJErCErPDBinU/bWLiWnX1owDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMjAwOTA0MDAwMDAw\n"
"WhcNMjUwOTE1MTYwMDAwWjAyMQswCQYDVQQGEwJVUzEWMBQGA1UEChMNTGV0J3Mg\n"
"RW5jcnlwdDELMAkGA1UEAxMCUjMwggEiMA0GCSqGSIb3DQEBAQUAA4IBDwAwggEK\n"
"AoIBAQC7AhUozPaglNMPEuyNVZLD+ILxmaZ6QoinXSaqtSu5xUyxr45r+XXIo9cP\n"
"R5QUVTVXjJ6oojkZ9YI8QqlObvU7wy7bjcCwXPNZOOftz2nwWgsbvsCUJCWH+jdx\n"
"sxPnHKzhm+/b5DtFUkWWqcFTzjTIUu61ru2P3mBw4qVUq7ZtDpelQDRrK9O8Zutm\n"
"NHz6a4uPVymZ+DAXXbpyb/uBxa3Shlg9F8fnCbvxK/eG3MHacV3URuPMrSXBiLxg\n"
"Z3Vms/EY96Jc5lP/Ooi2R6X/ExjqmAl3P51T+c8B5fWmcBcUr2Ok/5mzk53cU6cG\n"
"/kiFHaFpriV1uxPMUgP17VGhi9sVAgMBAAGjggEIMIIBBDAOBgNVHQ8BAf8EBAMC\n"
"AYYwHQYDVR0lBBYwFAYIKwYBBQUHAwIGCCsGAQUFBwMBMBIGA1UdEwEB/wQIMAYB\n"
"Af8CAQAwHQYDVR0OBBYEFBQusxe3WFbLrlAJQOYfr52LFMLGMB8GA1UdIwQYMBaA\n"
"FHm0WeZ7tuXkAXOACIjIGlj26ZtuMDIGCCsGAQUFBwEBBCYwJDAiBggrBgEFBQcw\n"
"AoYWaHR0cDovL3gxLmkubGVuY3Iub3JnLzAnBgNVHR8EIDAeMBygGqAYhhZodHRw\n"
"Oi8veDEuYy5sZW5jci5vcmcvMCIGA1UdIAQbMBkwCAYGZ4EMAQIBMA0GCysGAQQB\n"
"gt8TAQEBMA0GCSqGSIb3DQEBCwUAA4ICAQCFyk5HPqP3hUSFvNVneLKYY611TR6W\n"
"PTNlclQtgaDqw+34IL9fzLdwALduO/ZelN7kIJ+m74uyA+eitRY8kc607TkC53wl\n"
"ikfmZW4/RvTZ8M6UK+5UzhK8jCdLuMGYL6KvzXGRSgi3yLgjewQtCPkIVz6D2QQz\n"
"CkcheAmCJ8MqyJu5zlzyZMjAvnnAT45tRAxekrsu94sQ4egdRCnbWSDtY7kh+BIm\n"
"lJNXoB1lBMEKIq4QDUOXoRgffuDghje1WrG9ML+Hbisq/yFOGwXD9RiX8F6sw6W4\n"
"avAuvDszue5L3sz85K+EC4Y/wFVDNvZo4TYXao6Z0f+lQKc0t8DQYzk1OXVu8rp2\n"
"yJMC6alLbBfODALZvYH7n7do1AZls4I9d1P4jnkDrQoxB3UqQ9hVl3LEKQ73xF1O\n"
"yK5GhDDX8oVfGKF5u+decIsH4YaTw7mP3GFxJSqv3+0lUFJoi5Lc5da149p90Ids\n"
"hCExroL1+7mryIkXPeFM5TgO9r0rvZaBFOvV2z0gp35Z0+L4WPlbuEjN/lxPFin+\n"
"HlUjr8gRsI3qfJOQFy/9rKIJR0Y/8Omwt/8oTWgy1mdeHmmjk7j1nYsvC9JSQ6Zv\n"
"MldlTTKB3zhThV1+XWYp6rjd5JW1zbVWEkLNxE7GJThEUG3szgBVGP7pSWTUTsqX\n"
"nLRbwHOoq7hHwg==\n"
"-----END CERTIFICATE-----\n"
"-----BEGIN CERTIFICATE-----\n"
"MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw\n"
"TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh\n"
"cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4\n"
"WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu\n"
"ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY\n"
"MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc\n"
"h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+\n"
"0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U\n"
"A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW\n"
"T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH\n"
"B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC\n"
"B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv\n"
"KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn\n"
"OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn\n"
"jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw\n"
"qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI\n"
"rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV\n"
"HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq\n"
"hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL\n"
"ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ\n"
"3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK\n"
"NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5\n"
"ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur\n"
"TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC\n"
"jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc\n"
"oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq\n"
"4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA\n"
"mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d\n"
"emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=\n"
"-----END CERTIFICATE-----\n";

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
    xServerInfo.port = 443;

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
#if 0
    LogInfo( ( "Establishing a TLS session to %.*s:%d.",
               ( int32_t ) xServerInfo.hostNameLength,
               xServerInfo.pHostName,
               xServerInfo.port ) );
#else
    LogInfo( ( "Establishing a TLS session to binaries download server port %d.",
               xServerInfo.port ) );
#endif

    /* Attempt to create a mutually authenticated TLS connection. */
    xNetworkStatus = SecureSocketsTransport_Connect( pxNetworkContext,
                                                     &xServerInfo,
                                                     &xSocketsConfig );

    LogInfo(("prvConnectToDownloadServer result=%d", xNetworkStatus));
    if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
    {
        xStatus = pdFAIL;
    }

    return xStatus;
}


static int HandleReceivedData(const unsigned char* data, int offset, int dataLen, uint32_t partition_log_addr, uint32_t partition_size)
{
    LogInfo(("** Handling received data offset=%d dataLen=%d  [ 0x%02x 0x%02x 0x%02x 0x%02x ] ...", offset, dataLen, data[0], data[1], data[2], data[3]));
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
    // uint32_t *page_buffer;

    /* Received/processed data counter */
    uint32_t total_processed = 0;

    /* Preset result code to indicate "no error" */
    int32_t mflash_result = 0;

    unsigned char page_buffer[MFLASH_PAGE_SIZE];

    page_size   = MFLASH_PAGE_SIZE;

    /* Obtain physical address of FLASH to perform operations with */
    partition_phys_addr = mflash_drv_log2phys((void *)partition_log_addr, partition_size);
    if (partition_phys_addr == MFLASH_INVALID_ADDRESS)
        return -1;


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
                    LogError(("store_update_image: Error erasing sector %d", mflash_result));
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
                LogError(("store_update_image: Error storing page %d", mflash_result));
                break;
            }

            total_processed += chunk_len;
            chunk_flash_addr += chunk_len;

            // LogInfo(("store_update_image: processed %i bytes", total_processed));
        }

    } while (chunk_len == page_size);
    LogInfo(("** Handled received data offset=%d dataLen=%d total_processed=%d", offset, dataLen, total_processed));
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
                    pcFileSizeStr, ( int32_t ) *pxFileSize ) );
        return pdFAIL;
    }

    LogInfo( ( "The file is %d bytes long.", ( int32_t ) *pxFileSize ) );
    return pdPASS;
}

#include <time.h>

static BaseType_t prvDownloadFile(NetworkContext_t *pxNetworkContext,
                                 const TransportInterface_t * pxTransportInterface,
                                           const char * pcPath )
{
    /* Return value of this method. */
    BaseType_t xStatus = pdFAIL;
    HTTPStatus_t xHTTPStatus = HTTPSuccess;
    int retriesLimit = 20;

    /* Configurations of the initial request headers that are passed to
    * #HTTPClient_InitializeRequestHeaders. */
    HTTPRequestInfo_t xRequestInfo;
    /* Represents a response returned from an HTTP server. */
    HTTPResponse_t xResponse;
    /* Represents header data that will be sent in an HTTP request. */
    HTTPRequestHeaders_t xRequestHeaders;

    /* The size of the file we are trying to download . */
    size_t xFileSize = 0;
    size_t pxFileSize;

    /* The number of bytes we want to request with in each range of the file
     * bytes. */
    size_t xNumReqBytes = 0;
    /* xCurByte indicates which starting byte we want to download next. */
    size_t xCurByte = 0;

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

    // /* Verify the file exists by retrieving the file size. */
    #define FILE_SIZE_UNSET 999999999
    xFileSize = FILE_SIZE_UNSET;
    /* Set the number of bytes to request in each iteration, defined by the user
     * in democonfigRANGE_REQUEST_LENGTH. */
    // if( xFileSize < democonfigRANGE_REQUEST_LENGTH )
    // {
    //     xNumReqBytes = xFileSize;
    // }
    // else
    // {
        xNumReqBytes = democonfigRANGE_REQUEST_LENGTH;
    // }
    xStatus = pdPASS;



    partition_t update_partition;
    int32_t stored = 0;
    if (bl_get_update_partition_info(&update_partition) != kStatus_Success)
    {
        /* Could not get update partition info */
        LogError(("Could not get update partition info"));
        return pdFAIL;
    }
    

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
            LogInfo( ( "Downloading bytes %d-%d, out of %d total bytes, from %s...:  ",
                       ( int32_t ) ( xCurByte ),
                       ( int32_t ) ( xCurByte + xNumReqBytes - 1 ),
                       ( int32_t ) xFileSize == FILE_SIZE_UNSET? -1 : xFileSize,
                       "binary download server" ) );
            LogDebug( ( "Request Headers:\n%.*s",
                        ( int32_t ) xRequestHeaders.headersLen,
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
            if (xFileSize == FILE_SIZE_UNSET && GetFileSize(&pxFileSize, &xResponse) == pdPASS) {
                xFileSize = pxFileSize;
                LogInfo( ( "Setting xFileSize=%d", xFileSize));
            }

            LogInfo( ( "Received HTTP response from %s %s...",
                        "binary download server", pcPath ) );
            LogDebug( ( "Response Headers:\n%.*s",
                        ( int32_t ) xResponse.headersLen,
                        xResponse.pHeaders ) );
            LogInfo( ( "Response Body Len: %d\n",
                       ( int32_t ) xResponse.bodyLen) );


            HandleReceivedData(xResponse.pBody, xCurByte, xResponse.contentLength, update_partition.start, update_partition.size);
            stored += xResponse.contentLength;

            /* We increment by the content length because the server may not
             * have sent us the range we requested. */
            xCurByte += xResponse.contentLength;

            if( ( xFileSize - xCurByte ) < xNumReqBytes )
            {
                xNumReqBytes = xFileSize - xCurByte;
            }

            xStatus = ( xResponse.statusCode == httpexampleHTTP_STATUS_CODE_PARTIAL_CONTENT ) ? pdPASS : pdFAIL;
        }
        else
        {
#if 0
            // It is normal to get in here, because of the current server sinalization that the connection will be closed
            LogError( ( "An error occurred in downloading the file. "
                        "Failed to send HTTP GET request to %s %s: Error=%s.",
                        "binary download server", pcPath, HTTPClient_strerror( xHTTPStatus ) ) );
#endif
        }

            // Force re-connection
            if (xHTTPStatus == HTTPNetworkError && retriesLimit-- > 0) {
                // LogInfo(("Disconnecting retriesLimit=%d", retriesLimit));
                SecureSocketsTransport_Disconnect( pxNetworkContext );
                LogInfo(("Reconnecting"));
                vTaskDelay(50 / portTICK_PERIOD_MS);
                BaseType_t ret;
                ret = prvConnectToDownloadServer(pxNetworkContext);
                LogInfo(("Reconnect result = %d", ret));
                if (ret == pdPASS)
                    xHTTPStatus = HTTPSuccess;
            }

        if( xStatus != pdPASS )
        {
            LogError( ( "Received an invalid response from the server "
                        "(Status Code: %u).",
                        xResponse.statusCode ) );
        }
    }

    if (( xStatus == pdPASS ) && ( xHTTPStatus == HTTPSuccess )) {
#ifndef AKNANO_DRY_RUN
        LogInfo(("Validating image of size %d", stored));

        struct image_header *ih;
        // struct image_tlv_info *it;
        // uint32_t decl_size;
        // uint32_t tlv_size;
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
    TransportSocketStatus_t xNetworkStatus;
    BaseType_t xIsConnectionEstablished = pdFALSE;
    SecureSocketsTransportParams_t secureSocketsTransportParams = { 0 };

    /* Upon return, pdPASS will indicate a successful demo execution.
    * pdFAIL will indicate some failures occurred during execution. The
    * user of this demo must check the logs for any failure codes. */
    BaseType_t xDemoStatus = pdPASS;

    xNetworkContext.pParams = &secureSocketsTransportParams;
    xDemoStatus = prvConnectToDownloadServer(&xNetworkContext);
    // xDemoStatus = connectToServerWithBackoffRetries( prvConnectToServer,
    //                                                     &xNetworkContext );


    LogInfo(("AkNanoDownloadAndFlashImage: prvConnectToServer Result: %d", xDemoStatus));
    if( xDemoStatus == pdPASS )
    {
        /* Set a flag indicating that a TLS connection exists. */
        xIsConnectionEstablished = pdTRUE;

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
        char *relative_path = aknano_context->aknano_json_data.selected_target.uri + strlen("https://") + strlen(AKNANO_DOWNLOAD_ENDPOINT);
        LogInfo(("Download relativepath=%s", relative_path));
        xDemoStatus = prvDownloadFile(&xNetworkContext, &xTransportInterface, 
            relative_path);
    }

    /**************************** Disconnect. ******************************/

    /* Close the network connection to clean up any system resources that the
        * demo may have consumed. */
    if( xIsConnectionEstablished == pdTRUE )
    {
        /* Close the network connection.  */
        xNetworkStatus = SecureSocketsTransport_Disconnect( &xNetworkContext );

        if( xNetworkStatus != TRANSPORT_SOCKET_STATUS_SUCCESS )
        {
            xDemoStatus = pdFAIL;
            LogError( ( "SecureSocketsTransport_Disconnect() failed to close the network connection. "
                        "StatusCode=%d.", ( int ) xNetworkStatus ) );
        }
    }
    return xDemoStatus;
}
