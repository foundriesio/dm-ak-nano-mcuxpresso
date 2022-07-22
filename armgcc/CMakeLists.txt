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
SET(CMAKE_C_FLAGS_FLEXSPI_NOR_DEBUG " \
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

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_FLEXSPI_NOR_DEBUG} -fdiagnostics-color -DUSE_FLASH=1 -DLWIP_NETIF_API=1 -DMBEDTLS_ERROR_C -DMBEDTLS_MCUX_FREERTOS_THREADING_ALT -DMBEDTLS_CMAC_C")



project(ota_demo_enet)

set(MCUX_SDK_PROJECT_NAME ota_demo_enet.elf)

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")

include(${ProjDirPath}/flags.cmake)

include(${ProjDirPath}/config.cmake)

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
"${ProjDirPath}/../pin_mux.c"
"${ProjDirPath}/../pin_mux.h"
"${ProjDirPath}/../mcuboot_app_support.c"
"${ProjDirPath}/../mcuboot_app_support.h"
"${ProjDirPath}/../flash_partitioning.h"
"${ProjDirPath}/../board.c"
"${ProjDirPath}/../board.h"
"${ProjDirPath}/../clock_config.c"
"${ProjDirPath}/../clock_config.h"
"${ProjDirPath}/../dcd.c"
"${ProjDirPath}/../dcd.h"
"${ProjDirPath}/../../../../../Middleware/lwip/src/apps/sntp/sntp.c"
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
"${ProjDirPath}/../../../../../Middleware/unity/src/unity.c"
"${ProjDirPath}/../../../../../Middleware/unity/extras/fixture/src/unity_fixture.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/tests/common/aws_test_framework.c"
"${ProjDirPath}/../evkmimxrt1170_connect_cm4_cm7side.jlinkscript"

"${ProjDirPath}/../../../../../Middleware/NXP/lwip_ethernet/enet_ethernetif.c"
"${ProjDirPath}/../../../../../Middleware/NXP/lwip_ethernet/enet_ethernetif_kinetis.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/lwip_osal/src/sys_arch.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/api_lib.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/api_msg.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/err.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/if_api.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/netbuf.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/netdb.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/netifapi.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/sockets.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/api/tcpip.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/apps/sntp/sntp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/altcp_alloc.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/altcp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/altcp_tcp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/def.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/dns.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/inet_chksum.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/init.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ip.c"
# "${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/acd.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/autoip.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/dhcp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/etharp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/icmp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/igmp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/ip4_addr.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/ip4.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv4/ip4_frag.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/dhcp6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/ethip6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/icmp6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/inet6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/ip6_addr.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/ip6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/ip6_frag.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/mld6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/ipv6/nd6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/mem.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/memp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/netif.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/pbuf.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/raw.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/stats.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/sys.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/tcp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/tcp_in.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/tcp_out.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/timeouts.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/core/udp.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/bridgeif.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/bridgeif_fdb.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/ethernet.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/lowpan6_ble.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/lowpan6.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/lowpan6_common.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/slipif.c"
"${ProjDirPath}/../../../../../Middleware/lwip/src/netif/zepif.c"

"${ProjDirPath}/../../../../../core/components/flash/mflash/mflash_file.c"
"${ProjDirPath}/../../../../../core/components/flash/mflash/mimxrt1170/mflash_drv.c"

"${ProjDirPath}/../../../../../Middleware/mbedtls/library/aes.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/aesni.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/arc4.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/aria.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/asn1parse.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/asn1write.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/base64.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/bignum.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/blowfish.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/camellia.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ccm.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/certs.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/chacha20.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/chachapoly.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/cipher.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/cipher_wrap.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/cmac.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ctr_drbg.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/debug.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/des.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/dhm.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ecdh.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ecdsa.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ecjpake.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ecp.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ecp_curves.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/entropy.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/entropy_poll.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/error.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/gcm.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/havege.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/hkdf.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/hmac_drbg.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/md2.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/md4.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/md5.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/md.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/memory_buffer_alloc.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/net_sockets.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/nist_kw.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/oid.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/padlock.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pem.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pk.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pkcs11.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pkcs12.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pkcs5.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pkparse.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pk_wrap.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/pkwrite.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/platform.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/platform_util.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/poly1305.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_client.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_driver_wrappers.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_ecp.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_rsa.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_se.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_slot_management.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_crypto_storage.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/psa_its_file.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ripemd160.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/rsa.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/rsa_internal.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/sha1.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/sha256.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/sha512.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_cache.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_ciphersuites.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_cli.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_cookie.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_msg.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_srv.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_ticket.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_tls13_keys.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/ssl_tls.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/threading.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/timing.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/version.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/version_features.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509_create.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509_crl.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509_crt.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509_csr.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509write_crt.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/x509write_csr.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/xtea.c"
"${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/aes_alt.c"
"${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/des_alt.c"
"${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/ecp_alt.c"
"${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/ecp_alt_ksdk.c"
"${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/ecp_curves_alt.c"
"${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/ksdk_mbedtls.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/croutine.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/event_groups.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/list.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/portable/GCC/ARM_CM4F/port.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/portable/MemMang/heap_4.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/queue.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/stream_buffer.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/tasks.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/timers.c"
"${ProjDirPath}/../../../../../core/devices/MIMXRT1171/drivers/fsl_clock.c"
"${ProjDirPath}/../../../../../core/devices/MIMXRT1176/drivers/fsl_anatop_ai.c"
"${ProjDirPath}/../../../../../core/devices/MIMXRT1176/drivers/fsl_pmu.c"
"${ProjDirPath}/../../../../../core/devices/MIMXRT1176/gcc/startup_MIMXRT1176_cm7.S"
"${ProjDirPath}/../../../../../core/devices/MIMXRT1176/system_MIMXRT1176_cm7.c"

"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/ota_base64.c"
"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/ota.c"
"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/ota_cbor.c"
"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/ota_http.c"
"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/ota_interface.c"
"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/ota_mqtt.c"
"${ProjDirPath}/../../../../../Middleware/AWS/ota/source/portable/os/ota_os_freertos.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/backoffAlgorithm/source/backoff_algorithm.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/coreJSON/source/core_json.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/corePKCS11/source/core_pkcs11.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/corePKCS11/source/core_pki_utils.c"
"${ProjDirPath}/../../../../../Middleware/FreeRTOS/corePKCS11/source/portable/mbedtls/core_pkcs11_mbedtls.c"
"${ProjDirPath}/../../../../../Middleware/mbedtls/library/error.c"
"${ProjDirPath}/../../../../../Middleware/NXP/hostLib/libCommon/infra/sm_demo_utils_rtos.c"

"${ProjDirPath}/../../../../../examples/common/logging/logging.c"

"${ProjDirPath}/../../../../../added/rtos/freertos/demos/demo_runner/iot_demo_freertos.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/demos/dev_mode_key_provisioning/src/aws_dev_mode_key_provisioning.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/demos/network_manager/aws_iot_network_manager.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/3rdparty/mbedtls_utils/mbedtls_error.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/3rdparty/mbedtls_utils/mbedtls_utils.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/freertos/iot_clock_freertos.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/freertos/iot_metrics.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/freertos/iot_network_freertos.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/freertos/iot_threads_freertos.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/transport/secure_sockets/transport_secure_sockets.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/coreHTTP/source/core_http_client.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/coreHTTP/source/dependency/3rdparty/http_parser/http_parser.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/iot_init.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/taskpool/iot_taskpool.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/crypto/src/iot_crypto.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/tls/src/iot_tls.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/utils/src/iot_system_init.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/logging/iot_logging.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/libraries/logging/iot_logging_task_dynamic_buffers.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/tests/common/aws_test.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/tests/common/iot_test_freertos.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/vendors/nxp/pkcs11/common/iot_pkcs11_pal.c"
"${ProjDirPath}/../../../../../added/rtos/freertos/vendors/nxp/secure_sockets/lwip/iot_secure_sockets.c"
)

# target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
#     ${ProjDirPath}/../config_files
#     ${ProjDirPath}/../../..
#     ${ProjDirPath}/..
#     ${ProjDirPath}/../tests
#     ${ProjDirPath}/../../../../../rtos/freertos/libraries/3rdparty/CMock/vendor/unity/src/
#     ${ProjDirPath}/../../../../../rtos/freertos/libraries/3rdparty/CMock/vendor/unity/extras/fixture/src/
#     ${ProjDirPath}/../../../../../rtos/freertos/tests/include/
#     ${ProjDirPath}/../../../../../rtos/freertos/libraries/3rdparty/CMock/vendor/unity/extras/memory/src/
#     ${ProjDirPath}/../../../../../components/phy/device/phyrtl8211f/
# )

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_AKNANO_TEST EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUNITY_INCLUDE_CONFIG_H -DAKNANO_TEST")
endif(CMAKE_AKNANO_TEST EQUAL 1)


# set(CMAKE_MODULE_PATH
#     ${ProjDirPath}/../../../../../components/phy/mdio/enet
#     ${ProjDirPath}/../../../../../components/phy/device/phyksz8081
#     ${ProjDirPath}/../../../../../components/phy/device/phyrtl8211f
#     ${ProjDirPath}/../../../../../devices/MIMXRT1176/drivers/cm7
#     ${ProjDirPath}/../../../../../devices/MIMXRT1176/drivers
#     ${ProjDirPath}/../../../../../Middleware/lwip
#     ${ProjDirPath}/../../../../../components/phy
#     ${ProjDirPath}/../../../../../devices/MIMXRT1176    
#     ${ProjDirPath}/../../../../../devices/MIMXRT1176/utilities
#     ${ProjDirPath}/../../../../../components/flash/mflash
#     ${ProjDirPath}/../../../../../components/flash/mflash/mimxrt1170
#     ${ProjDirPath}/../../../../../Middleware/mbedtls
#     ${ProjDirPath}/../../../../../rtos/freertos
#     ${ProjDirPath}/../../../../../rtos/freertos/freertos_kernel
#     ${ProjDirPath}/../../../../../components/uart
#     ${ProjDirPath}/../../../../../components/serial_manager
#     ${ProjDirPath}/../../../../../components/lists
#     ${ProjDirPath}/../../../../../CMSIS/Core/Include
# )

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../config_files
    ${ProjDirPath}/../tests
    ${ProjDirPath}/../../..
    ${ProjDirPath}/../../../../../core/devices/MIMXRT1176/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/corePKCS11/test/shared-config/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/coreMQTT/source/interface/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/include/
    ${ProjDirPath}/../../../../../Middleware/lwip/src/include/
    ${ProjDirPath}/../../../../../Middleware/NXP/mbedtls_port/ksdk/
    ${ProjDirPath}/../../../../../core/components/flash/mflash/
    ${ProjDirPath}/../../../../../core/components/flash/mflash/mimxrt1170/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/include/private/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/lwip_osal/include/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/kernel/portable/GCC/ARM_CM4F/
    ${ProjDirPath}/../../../../../added/rtos/freertos/demos/dev_mode_key_provisioning/include/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/corePKCS11/source/include/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/pkcs11/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/coreJSON/source/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/include/platform/
    ${ProjDirPath}/../../../../../added/rtos/freertos/demos/common/pkcs11_helpers
    ${ProjDirPath}/../../../../../added/rtos/freertos/demos/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/coreHTTP/source/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/logging/include/
    ${ProjDirPath}/../../../../../Middleware/unity/src/
    ${ProjDirPath}/../../../../../Middleware/NXP/lwip_ethernet/
    ${ProjDirPath}/../../../../../Middleware/mbedtls/include/
    ${ProjDirPath}/../../../../../Middleware/mbedtls/library/
    ${ProjDirPath}/../../../../../added/rtos/freertos/tests/include/
    ${ProjDirPath}/../../../../../Middleware/unity/extras/fixture/src/
    ${ProjDirPath}/../../../../../Middleware/unity/extras/memory/src/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/utils/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/include/
    ${ProjDirPath}/../../../../../added/rtos/rtos/freertos/demos/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/freertos/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/transport/secure_sockets/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/secure_sockets/include/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/backoffAlgorithm/source/include/
    ${ProjDirPath}/../../../../../Middleware/AWS/ota/source/include/
    ${ProjDirPath}/../../../../../Middleware/AWS/ota/source/dependency/3rdparty/tinycbor/src/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/abstractions/platform/include/private/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/coreHTTP/source/dependency/3rdparty/http_parser/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/tls/include/
    ${ProjDirPath}/../../../../../Middleware/FreeRTOS/tls_freertos/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/tls/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/freertos_plus/standard/crypto/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/3rdparty/mbedtls_utils/
    ${ProjDirPath}/../../../../../added/rtos/freertos/demos/network_manager/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/include/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/include/types/
    ${ProjDirPath}/../../../../../added/rtos/freertos/libraries/c_sdk/standard/common/include/
)

#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../../../core
    ${ProjDirPath}/../../../../../
    ${ProjDirPath}/../../../../../Middleware
    ${ProjDirPath}/../../../../../core/devices/MIMXRT1176    
)

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

# include(component_mflash_file)

# include(component_mflash_rt1170)

# include(middleware_mbedtls)

# include(middleware_mbedtls_port_ksdk)

# include(middleware_mbedtls_rt2)

# include(middleware_freertos-aws_iot_demos_dev_mode_key_provisioning)

# include(middleware_freertos-aws_iot_vendor_nxp_secure_sockets_lwip)

# include(middleware_freertos-kernel)

# include(middleware_freertos-kernel_heap_4)

# include(device_MIMXRT1176_startup)

include(driver_igpio)

# include(driver_dcp)

# include(driver_trng)

# include(middleware_freertos-aws_iot_network_manager)

# include(middleware_freertos-aws_iot_demos_ota_demo_helpers)

include(driver_caam)

include(driver_dcdc_soc)

include(driver_phy-device-rtl8211f)

# include(middleware_freertos-aws_iot_demos_corehttp) # added

# include(middleware_freertos-aws_iot_libraries_corehttp) # added

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--start-group)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE m)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE c)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE gcc)

target_link_libraries(${MCUX_SDK_PROJECT_NAME} PRIVATE nosys)

TARGET_LINK_LIBRARIES(${MCUX_SDK_PROJECT_NAME} PRIVATE -Wl,--end-group)

ADD_CUSTOM_COMMAND(TARGET ${MCUX_SDK_PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_OBJCOPY}
-Obinary ${EXECUTABLE_OUTPUT_PATH}/${MCUX_SDK_PROJECT_NAME} ${EXECUTABLE_OUTPUT_PATH}/ota_demo.bin)

