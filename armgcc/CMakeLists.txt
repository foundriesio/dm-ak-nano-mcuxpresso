# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# THE VERSION NUMBER
SET (Tutorial_VERSION_MAJOR 1)
SET (Tutorial_VERSION_MINOR 0)

# ENABLE ASM
ENABLE_LANGUAGE(ASM)

SET(CMAKE_STATIC_LIBRARY_PREFIX)
SET(CMAKE_STATIC_LIBRARY_SUFFIX)

SET(CMAKE_EXECUTABLE_LIBRARY_PREFIX)
SET(CMAKE_EXECUTABLE_LIBRARY_SUFFIX)

# CURRENT DIRECTORY
SET(ProjDirPath ${CMAKE_CURRENT_SOURCE_DIR})

SET(EXECUTABLE_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})
SET(LIBRARY_OUTPUT_PATH ${ProjDirPath}/${CMAKE_BUILD_TYPE})

# SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS}")
SET(CMAKE_C_FLAGS_FLEXSPI_NOR " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR_DEBUG} \
    -DXIP_EXTERNAL_FLASH=1 \
    -DXIP_BOOT_HEADER_ENABLE=1 \
    -DDEBUG \
    -DLWIP_TIMEVAL_PRIVATE=0 \
    -DCPU_MIMXRT1176DVMAA_cm7 \
    -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
    -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
    -DPRINTF_ADVANCED_ENABLE=1 \
    -DUSE_RTOS=1 \
    -DLWIP_DNS=1 \
    -DLWIP_DHCP=1 \
    -DHTTP_DO_NOT_USE_CUSTOM_CONFIG=1 \
    -DMQTT_DO_NOT_USE_CUSTOM_CONFIG=1 \
    -DCRYPTO_USE_DRIVER_CAAM \
    -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
    -DMFLASH_FILE_BASEADDR=14221312 \
    -DSDK_OS_FREE_RTOS \
    -DSERIAL_PORT_TYPE_UART=1 \
    -DMCUXPRESSO_SDK \
    -g \
    -O0 \
    -fomit-frame-pointer \
    -Wno-unused-function \
    -mcpu=cortex-m7 \
    -Wall \
    -mfloat-abi=hard \
    -mfpu=fpv5-d16 \
    -mthumb \
    -MMD \
    -MP \
    -fno-common \
    -ffunction-sections \
    -fdata-sections \
    -ffreestanding \
    -fno-builtin \
    -mapcs \
    -std=gnu99 \
")

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_FLEXSPI_NOR} -fdiagnostics-color -DUSE_FLASH=1 -DLWIP_NETIF_API=1 -DMBEDTLS_ERROR_C -DMBEDTLS_MCUX_FREERTOS_THREADING_ALT -DMBEDTLS_CMAC_C")

project(ota_demo_enet)

set(MCUX_SDK_PROJECT_NAME ota_demo_enet.elf)

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)


SET(FreeRtosAwsPath "${ProjDirPath}/../../../rtos/amazon-freertos")
SET(FreeRtosKernelPath "${ProjDirPath}/../../../rtos/freertos/freertos_kernel")

add_executable(${MCUX_SDK_PROJECT_NAME} 
"${ProjDirPath}/../config_files/FreeRTOSConfig.h"
"${ProjDirPath}/../config_files/aws_mbedtls_config.h"
#"${ProjDirPath}/../config_files/aws_demo_config.h"
#"${ProjDirPath}/../config_files/aws_wifi_config.h"
#"${ProjDirPath}/../config_files/core_http_config.h"
#"${ProjDirPath}/../config_files/core_mqtt_config.h"
"${ProjDirPath}/../config_files/core_pkcs11_config.h"
"${ProjDirPath}/../config_files/FreeRTOSIPConfig.h"
"${ProjDirPath}/../config_files/iot_config.h"
"${ProjDirPath}/../config_files/iot_secure_sockets_config.h"
"${ProjDirPath}/../config_files/ota_config.h"
"${ProjDirPath}/../config_files/ota_demo_config.h"
"${ProjDirPath}/../config_files/aws_iot_network_config.h"
"${ProjDirPath}/../main_enet.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../mcuboot_app_support.c"
"${ProjDirPath}/../mcuboot_app_support.h"
"${ProjDirPath}/../flash_partitioning.h"
"${ProjDirPath}/../read_button_task.c"
"${ProjDirPath}/../aknano_device_gateway.c"
"${ProjDirPath}/../aknano_targets_manifest.c"
"${ProjDirPath}/../aknano_flash_storage.c"
"${ProjDirPath}/../aknano_image_download.c"
"${ProjDirPath}/../aknano_rest_api.c"
"${ProjDirPath}/../aknano.c"
"${ProjDirPath}/../flexspi_nor_flash_ops.c"
"${ProjDirPath}/../ota_flag/sbl_ota_flag.c"
"${ProjDirPath}/../tests/aknano_test.c"

"${ProjDirPath}/../board/pin_mux.c"
"${ProjDirPath}/../board/pin_mux.h"
"${ProjDirPath}/../board/board.c"
"${ProjDirPath}/../board/board.h"
"${ProjDirPath}/../board/clock_config.c"
"${ProjDirPath}/../board/clock_config.h"
"${ProjDirPath}/../board/dcd.c"
"${ProjDirPath}/../board/dcd.h"
"${ProjDirPath}/../board/evkmimxrt1170_connect_cm4_cm7side.jlinkscript"

"${FreeRtosAwsPath}/demos/demo_runner/iot_demo_freertos.c"
"${FreeRtosAwsPath}/demos/dev_mode_key_provisioning/src/aws_dev_mode_key_provisioning.c"
"${FreeRtosAwsPath}/demos/network_manager/aws_iot_network_manager.c"
"${FreeRtosAwsPath}/libraries/3rdparty/mbedtls_utils/mbedtls_error.c"
"${FreeRtosAwsPath}/libraries/3rdparty/mbedtls_utils/mbedtls_utils.c"
"${FreeRtosAwsPath}/libraries/abstractions/platform/freertos/iot_clock_freertos.c"
"${FreeRtosAwsPath}/libraries/abstractions/platform/freertos/iot_threads_freertos.c"
"${FreeRtosAwsPath}/libraries/abstractions/transport/secure_sockets/transport_secure_sockets.c"
"${FreeRtosAwsPath}/libraries/c_sdk/standard/common/iot_init.c"
"${FreeRtosAwsPath}/libraries/c_sdk/standard/common/taskpool/iot_taskpool.c"
"${FreeRtosAwsPath}/libraries/freertos_plus/standard/crypto/src/iot_crypto.c"
"${FreeRtosAwsPath}/libraries/freertos_plus/standard/tls/src/iot_tls.c"
"${FreeRtosAwsPath}/libraries/freertos_plus/standard/utils/src/iot_system_init.c"
"${FreeRtosAwsPath}/libraries/logging/iot_logging_task_dynamic_buffers.c"
"${FreeRtosAwsPath}/libraries/logging/iot_logging.c"
"${FreeRtosAwsPath}/tests/common/aws_test.c"
"${FreeRtosAwsPath}/tests/common/aws_test_framework.c"
"${FreeRtosAwsPath}/tests/common/iot_test_freertos.c"

"${FreeRtosKernelPath}/croutine.c"
"${FreeRtosKernelPath}/event_groups.c"
"${FreeRtosKernelPath}/list.c"
"${FreeRtosKernelPath}/portable/GCC/ARM_CM4F/port.c"
"${FreeRtosKernelPath}/portable/MemMang/heap_4.c"
"${FreeRtosKernelPath}/queue.c"
"${FreeRtosKernelPath}/stream_buffer.c"
"${FreeRtosKernelPath}/tasks.c"
"${FreeRtosKernelPath}/timers.c"

## "${FreeRtosAwsPath}/libraries/3rdparty/lwip_osal/src/sys_arch.c" # -> this file lacks a few functions
"${ProjDirPath}/../../../poc/3rdparty/middleware/freertos/lwip_osal/src/sys_arch.c"
"${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/lwip_ethernet/enet_ethernetif_kinetis.c"
"${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/lwip_ethernet/enet_ethernetif.c"
"${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/mbedtls_port/ksdk/aes_alt.c"
"${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/mbedtls_port/ksdk/ecp_alt.c"
"${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/mbedtls_port/ksdk/ecp_curves_alt.c"
"${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/mbedtls_port/ksdk/ksdk_mbedtls.c"

"${ProjDirPath}/../../../poc/3rdparty/rtos/freertos/vendors/nxp/pkcs11/common/iot_pkcs11_pal.c"
"${ProjDirPath}/../../../poc/3rdparty/rtos/freertos/vendors/nxp/secure_sockets/lwip/iot_secure_sockets.c"

"${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1171/drivers/fsl_clock.c"
"${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176/drivers/fsl_anatop_ai.c"
"${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176/drivers/fsl_pmu.c"
"${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S"
"${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176/system_MIMXRT1176_cm7.c"

"${ProjDirPath}/../../../middleware/freertos/backoffAlgorithm/source/backoff_algorithm.c"
"${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/core_http_client.c"
"${ProjDirPath}/../../../middleware/freertos/coreJSON/source/core_json.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/core_pkcs11.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/core_pki_utils.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/portable/mbedtls/core_pkcs11_mbedtls.c"

"${ProjDirPath}/../../../middleware/http-parser/http_parser.c"

"${ProjDirPath}/../../../middleware/lwip/src/api/api_lib.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/api_msg.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/err.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/if_api.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/netbuf.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/netdb.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/netifapi.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/sockets.c"
"${ProjDirPath}/../../../middleware/lwip/src/api/tcpip.c"
"${ProjDirPath}/../../../middleware/lwip/src/apps/sntp/sntp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/altcp_alloc.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/altcp_tcp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/altcp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/def.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/dns.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/inet_chksum.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/init.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ip.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/autoip.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/dhcp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/etharp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/icmp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/igmp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/ip4_addr.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/ip4_frag.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/ipv4/ip4.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/dhcp6.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/ethip6.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/icmp6.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/inet6.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/ip6_addr.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/ip6_frag.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/ip6.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/mld6.c"
# "${ProjDirPath}/../../../middleware/lwip/src/core/ipv6/nd6.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/mem.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/memp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/netif.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/pbuf.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/raw.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/stats.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/sys.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/tcp_in.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/tcp_out.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/tcp.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/timeouts.c"
"${ProjDirPath}/../../../middleware/lwip/src/core/udp.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/bridgeif_fdb.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/bridgeif.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/ethernet.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/lowpan6_ble.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/lowpan6_common.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/lowpan6.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/slipif.c"
"${ProjDirPath}/../../../middleware/lwip/src/netif/zepif.c"

"${ProjDirPath}/../../../middleware/mbedtls/library/aes.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/aesni.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/arc4.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/aria.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/asn1parse.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/asn1write.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/base64.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/bignum.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/blowfish.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/camellia.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ccm.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/certs.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/chacha20.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/chachapoly.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/cipher_wrap.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/cipher.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/cmac.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ctr_drbg.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/debug.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/des.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/dhm.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ecdh.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ecdsa.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ecjpake.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ecp_curves.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ecp.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/entropy_poll.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/entropy.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/error.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/error.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/gcm.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/havege.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/hkdf.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/hmac_drbg.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/md.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/md2.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/md4.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/md5.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/memory_buffer_alloc.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/net_sockets.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/nist_kw.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/oid.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/padlock.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pem.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pk_wrap.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pk.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pkcs11.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pkcs12.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pkcs5.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pkparse.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/pkwrite.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/platform_util.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/platform.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_client.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_driver_wrappers.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_ecp.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_rsa.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_se.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_slot_management.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto_storage.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_crypto.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/psa_its_file.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/rsa_internal.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/rsa.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/sha1.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/sha256.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/sha512.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_cache.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_ciphersuites.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_cli.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_cookie.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_msg.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_srv.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_ticket.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_tls.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/ssl_tls13_keys.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/threading.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/timing.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/version_features.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/version.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509_create.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509_crl.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509_crt.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509_csr.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509write_crt.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/x509write_csr.c"
"${ProjDirPath}/../../../middleware/mbedtls/library/xtea.c"

"${ProjDirPath}/../../../middleware/unity/src/unity.c"
"${ProjDirPath}/../../../middleware/unity/extras/fixture/src/unity_fixture.c"
)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_AKNANO_TEST EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUNITY_INCLUDE_CONFIG_H -DAKNANO_TEST")
endif(CMAKE_AKNANO_TEST EQUAL 1)


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../..
    ${ProjDirPath}/../board
    ${ProjDirPath}/../config_files
    ${ProjDirPath}/../tests

    ${FreeRtosAwsPath}/libraries/logging/include/
    ${ProjDirPath}/../../../middleware/pkcs11/
    ${ProjDirPath}/../../../poc/3rdparty/middleware/freertos/lwip_osal/include/
    ${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/lwip_ethernet/
    ${ProjDirPath}/../../../poc/3rdparty/middleware/nxp/mbedtls_port/ksdk/
    ${ProjDirPath}/../../../middleware/freertos/backoffAlgorithm/source/include/
    ${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/include/
    ${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/interface/
    ${ProjDirPath}/../../../middleware/freertos/coreJSON/source/include/
    ${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/include/
    ${ProjDirPath}/../../../middleware/freertos/corePKCS11/test/shared-config/
    ${ProjDirPath}/../../../middleware/freertos/pkcs11/
    ${ProjDirPath}/../../../middleware/freertos/tls_freertos/
    ${ProjDirPath}/../../../middleware/http-parser/
    ${ProjDirPath}/../../../middleware/lwip/src/include/
    ${ProjDirPath}/../../../middleware/mbedtls/include/
    ${ProjDirPath}/../../../middleware/mbedtls/include/mbedtls
    ${ProjDirPath}/../../../middleware/mbedtls/library/
    ${ProjDirPath}/../../../middleware/unity/extras/fixture/src/
    ${ProjDirPath}/../../../middleware/unity/extras/memory/src/
    ${ProjDirPath}/../../../middleware/unity/src/
    ${FreeRtosAwsPath}/demos/common/pkcs11_helpers
    ${FreeRtosAwsPath}/demos/dev_mode_key_provisioning/include/
    ${FreeRtosAwsPath}/demos/include/
    ${FreeRtosAwsPath}/demos/network_manager/
    ${FreeRtosAwsPath}/libraries/3rdparty/lwip_osal/include/
    ${FreeRtosAwsPath}/libraries/3rdparty/mbedtls_utils/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/freertos/include/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/platform/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/private/
    ${FreeRtosAwsPath}/libraries/abstractions/secure_sockets/include/
    ${FreeRtosAwsPath}/libraries/abstractions/transport/secure_sockets/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/private/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/types/
    ${FreeRtosAwsPath}/libraries/coreHTTP/source/dependency/3rdparty/http_parser/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/crypto/include/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/tls/include/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/tls/include/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/utils/include/
    ${FreeRtosAwsPath}/tests/include/
    ${FreeRtosKernelPath}/include/
    ${FreeRtosKernelPath}/portable/GCC/ARM_CM4F/
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../mcuxsdk/
    ${ProjDirPath}/../../../
    ${ProjDirPath}/../../../middleware
    ${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176    
    ${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions
    ${ProjDirPath}/../../../rtos/amazon-freertos/tools/cmake
)

include(afr)

#Include Entry cmake component
include(all_devices)

# include modules
include(driver_mdio-enet)

include(driver_phy-device-ksz8081)

include(driver_cache_armv7_m7)

include(driver_flexspi)

# include(middleware_lwip_enet_ethernetif)

# include(middleware_lwip)

include(driver_phy-common)

include(driver_enet)

include(utility_debug_console)

include(component_mflash_file)

# include(backoff_algorithm)

# include(component_mflash_rt1170)

# include(middleware_mbedtls)

# include(middleware_mbedtls_port_ksdk)

# include(middleware_mbedtls_rt2)

# include(middleware_freertos-aws_iot_demos_dev_mode_key_provisioning)

# include(middleware_freertos-aws_iot_vendor_nxp_secure_sockets_lwip)

# include(middleware_freertos-kernel)

# include(middleware_freertos-kernel_heap_4)

include(all_lib_device_MIMXRT1176_cm7)

include(driver_igpio)

# include(driver_dcp)

# include(driver_trng)

# include(middleware_freertos-aws_iot_network_manager)

# include(middleware_freertos-aws_iot_demos_ota_demo_helpers)

include(driver_caam)

include(driver_dcdc_soc)

include(driver_phy-device-rtl8211f)

# include(middleware_freertos-aws_iot_demos_corehttp) # 3rdparty

# include(middleware_freertos-aws_iot_libraries_corehttp) # 3rdparty

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/ota_demo.bin)

