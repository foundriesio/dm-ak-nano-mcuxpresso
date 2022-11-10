SET (AKNANO_ENABLE_SE05X 0)
SET (AKNANO_ENABLE_EL2GO 0)
SET (AKNANO_ALLOW_PROVISIONING 0)

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


project(ota_demo_enet)

set(MCUX_SDK_PROJECT_NAME ota_demo_enet.elf)

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")


if(CMAKE_BOARD_MODEL STREQUAL rt1170)
include(${ProjDirPath}/flags_rt1170.cmake)
include(${ProjDirPath}/config_rt1170.cmake)
endif(CMAKE_BOARD_MODEL STREQUAL rt1170)

if(CMAKE_BOARD_MODEL STREQUAL rt1060)
include(${ProjDirPath}/flags_rt1060.cmake)
include(${ProjDirPath}/config_rt1060.cmake)
endif(CMAKE_BOARD_MODEL STREQUAL rt1060)

if(AKNANO_ALLOW_PROVISIONING EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_ALLOW_PROVISIONING")
endif(AKNANO_ALLOW_PROVISIONING EQUAL 1)

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
"${ProjDirPath}/../aknano_client.c"
"${ProjDirPath}/../aknano_tuf_client.c"
"${ProjDirPath}/../aknano.c"
"${ProjDirPath}/../aknano_net.c"
"${ProjDirPath}/../flexspi_nor_flash_ops.c"
"${ProjDirPath}/../tests/aknano_test.c"
"${ProjDirPath}/../../libtufnano/libtufnano.c"

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
"${ProjDirPath}/../../../middleware/lwip/port/enet_ethernetif_kinetis.c"
"${ProjDirPath}/../../../middleware/lwip/port/enet_ethernetif.c"
"${ProjDirPath}/../../../poc/3rdparty/rtos/freertos/vendors/nxp/secure_sockets/lwip/iot_secure_sockets.c"

"${ProjDirPath}/../../../middleware/freertos/backoffAlgorithm/source/backoff_algorithm.c"
"${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/core_http_client.c"
"${ProjDirPath}/../../../middleware/freertos/coreJSON/source/core_json.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/core_pkcs11.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/core_pki_utils.c"

"${ProjDirPath}/../../../middleware/http-parser/http_parser.c"

"${ProjDirPath}/../../../middleware/lwip/src/apps/sntp/sntp.c"

"${ProjDirPath}/../../../middleware/unity/src/unity.c"
"${ProjDirPath}/../../../middleware/unity/extras/fixture/src/unity_fixture.c"
)

if(CMAKE_BOARD_MODEL STREQUAL rt1170)
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "${ProjDirPath}/../board_rt1170/pin_mux.c"
    "${ProjDirPath}/../board_rt1170/pin_mux.h"
    "${ProjDirPath}/../board_rt1170/board.c"
    "${ProjDirPath}/../board_rt1170/board.h"
    "${ProjDirPath}/../board_rt1170/clock_config.c"
    "${ProjDirPath}/../board_rt1170/clock_config.h"
    "${ProjDirPath}/../board_rt1170/dcd.c"
    "${ProjDirPath}/../board_rt1170/dcd.h"
    "${ProjDirPath}/../board_rt1170/evkmimxrt1170_connect_cm4_cm7side.jlinkscript"

    )
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_BOARD_MODEL_RT1170")

    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../board_rt1170
    )
    SET(CMAKE_C_FLAGS_FLEXSPI_NOR " \
    ${CMAKE_C_FLAGS_FLEXSPI_NOR} \
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
endif(CMAKE_BOARD_MODEL STREQUAL rt1170)

if(CMAKE_BOARD_MODEL STREQUAL rt1060)

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
    "${ProjDirPath}/../board_rt1060/pin_mux.c"
    "${ProjDirPath}/../board_rt1060/pin_mux.h"
    "${ProjDirPath}/../board_rt1060/board.c"
    "${ProjDirPath}/../board_rt1060/board.h"
    "${ProjDirPath}/../board_rt1060/clock_config.c"
    "${ProjDirPath}/../board_rt1060/clock_config.h"
    "${ProjDirPath}/../board_rt1060/dcd.c"
    "${ProjDirPath}/../board_rt1060/dcd.h"

    "${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1062/gcc/startup_MIMXRT1062.S"
    )
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_BOARD_MODEL_RT1060")

    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/../board_rt1060
    )

    SET(CMAKE_C_FLAGS_FLEXSPI_NOR " \
        ${CMAKE_C_FLAGS_FLEXSPI_NOR} \
        -DXIP_EXTERNAL_FLASH=1 \
        -DXIP_BOOT_HEADER_ENABLE=1 \
        -DDEBUG \
        -DLWIP_TIMEVAL_PRIVATE=0 \
        -DCPU_MIMXRT1062DVL6A \
        -DFSL_FEATURE_PHYKSZ8081_USE_RMII50M_MODE \
        -DFSL_SDK_ENABLE_DRIVER_CACHE_CONTROL=1 \
        -DUSE_RTOS=1 \
        -DLWIP_DISABLE_PBUF_POOL_SIZE_SANITY_CHECKS=1 \
        -DMFLASH_FILE_BASEADDR=7340032 \
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
    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${ProjDirPath}/../../../mcuxsdk/drivers/trng/
    )
endif(CMAKE_BOARD_MODEL STREQUAL rt1060)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_FLEXSPI_NOR} -fdiagnostics-color -DUSE_FLASH=1 -DLWIP_NETIF_API=1 -DMBEDTLS_ERROR_C -DMBEDTLS_MCUX_FREERTOS_THREADING_ALT -DMBEDTLS_CMAC_C -DJSON_QUERY_KEY_SEPARATOR=\\'/\\'")

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if(CMAKE_AKNANO_TEST EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUNITY_INCLUDE_CONFIG_H -DAKNANO_TEST")
endif(CMAKE_AKNANO_TEST EQUAL 1)


# SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_FORCE_DEVICE_CLEANUP")
# SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_TEST_ROLLBACK")

target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../..
    ${ProjDirPath}/../config_files
    ${ProjDirPath}/../tests
    ${ProjDirPath}/../../libtufnano
    
    ${FreeRtosAwsPath}/libraries/logging/include/
    ${ProjDirPath}/../../../middleware/pkcs11/
    ${ProjDirPath}/../../../poc/3rdparty/middleware/freertos/lwip_osal/include/
    ${ProjDirPath}/../../../middleware/freertos/backoffAlgorithm/source/include/
    ${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/include/
    ${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/interface/
    ${ProjDirPath}/../../../middleware/freertos/coreJSON/source/include/
    ${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/include/
    ${ProjDirPath}/../../../middleware/freertos/corePKCS11/test/shared-config/
    ${ProjDirPath}/../../../middleware/freertos/pkcs11/
    ${ProjDirPath}/../../../middleware/freertos/tls_freertos/
    ${ProjDirPath}/../../../middleware/http-parser/
    ${ProjDirPath}/../../../middleware/lwip/port/
    ${ProjDirPath}/../../../middleware/mbedtls/port/ksdk/
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

if(CMAKE_BOARD_MODEL STREQUAL rt1060)
#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../mcuxsdk/
    ${ProjDirPath}/../../../
    ${ProjDirPath}/../../../middleware
    ${ProjDirPath}/../../../middleware/mbedtls
    ${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1162
    ${ProjDirPath}/../../../mcuxsdk/devices/
    ${ProjDirPath}/../../../mcuxsdk/drivers/lpi2c/
    ${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions
    ${ProjDirPath}/../../../rtos/amazon-freertos/tools/cmake
)
endif(CMAKE_BOARD_MODEL STREQUAL rt1060)

if(CMAKE_BOARD_MODEL STREQUAL rt1170)
#path for entry cmake
set(CMAKE_MODULE_PATH
    ${ProjDirPath}/../../../mcuxsdk/
    ${ProjDirPath}/../../../
    ${ProjDirPath}/../../../middleware
    ${ProjDirPath}/../../../middleware/mbedtls
    ${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176
    ${ProjDirPath}/../../../mcuxsdk/drivers/lpi2c/
    ${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions
    ${ProjDirPath}/../../../rtos/amazon-freertos/tools/cmake
)
endif(CMAKE_BOARD_MODEL STREQUAL rt1170)


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

include(device_CMSIS)
include(driver_clock)
include(CMSIS_Include_core_cm)
include(middleware_lwip)

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

if(CMAKE_BOARD_MODEL STREQUAL rt1060)

include(device_startup)
include(device_system)
# include(all_lib_device_MIMXRT1176_cm7)
include(driver_dcp)

include(driver_trng)
include(middleware_mbedtls_MIMXRT1062)
include(middleware_lwip_enet_ethernetif_MIMXRT1062)

endif(CMAKE_BOARD_MODEL STREQUAL rt1060)


if(CMAKE_BOARD_MODEL STREQUAL rt1170)

include(all_lib_device_MIMXRT1176_cm7)

include(device_startup_MIMXRT1176_cm7)
include(device_system_MIMXRT1176_cm7)

include(middleware_mbedtls_MIMXRT1176_cm7)
include(middleware_lwip_enet_ethernetif_MIMXRT1176_cm7)
include(middleware_mbedtls_port_ksdk)


include(driver_caam)
include(driver_dcdc_soc)

include(driver_phy-device-rtl8211f)

endif(CMAKE_BOARD_MODEL STREQUAL rt1170)

include(driver_igpio)


# include(driver_trng)

# include(middleware_freertos-aws_iot_network_manager)

# include(middleware_freertos-aws_iot_demos_ota_demo_helpers)


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


if(AKNANO_ENABLE_SE05X EQUAL 1)
    SET(SE05X_SDK_PATH "${ProjDirPath}/../../../middleware/plug-and-trust")
    SET(SE05X_SDK_FROM_ZIP_PATH "${ProjDirPath}/../../../poc/3rdparty/middleware/plug-and-trust")

    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_ENABLE_SE05X")

    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        "${ProjDirPath}/../../../middleware/mbedtls/library/ecdh.c"
        "${SE05X_SDK_PATH}/sss/ex/src/ex_sss_boot.c"
        "${SE05X_SDK_PATH}/sss/ex/src/ex_sss_se05x.c"
        "${SE05X_SDK_PATH}/sss/src/fsl_sss_apis.c"
        "${SE05X_SDK_PATH}/sss/src/fsl_sss_util_asn1_der.c"
        "${SE05X_SDK_PATH}/sss/src/fsl_sss_util_rsa_sign_utils.c"
        "${SE05X_SDK_PATH}/sss/src/fsl_sss_util_asn1_der.c"
        "${SE05X_SDK_PATH}/sss/src/mbedtls/fsl_sss_mbedtls_apis.c"
        "${SE05X_SDK_PATH}/sss/src/se05x/fsl_sss_se05x_apis.c"
        "${SE05X_SDK_PATH}/sss/src/se05x/fsl_sss_se05x_eckey.c"
        "${SE05X_SDK_PATH}/sss/src/se05x/fsl_sss_se05x_mw.c"
        "${SE05X_SDK_PATH}/sss/src/se05x/fsl_sss_se05x_policy.c"
        "${SE05X_SDK_PATH}/sss/src/se05x/fsl_sss_se05x_scp03.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/infra/sm_connect.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/infra/global_platf.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/nxScp/nxScp03_Com.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/smCom.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/smComT1oI2C.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/T1oI2C/phNxpEse_Api.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/T1oI2C/phNxpEsePal_i2c.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/T1oI2C/phNxpEseProto7816_3.c"
        # "${SE05X_SDK_PATH}/hostlib/hostLib/platform/ksdk/timer_kinetis_bm.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/se05x_03_xx_xx/se05x_APDU.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/se05x/src/se05x_tlv.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/se05x/src/se05x_ECC_curves.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/se05x/src/se05x_mw.c"
        "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/log/nxLog.c"
        "${SE05X_SDK_PATH}/sss/ex/src/ex_sss_boot_connectstring.c"

        # SE050 PKCS11
        "${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/pkcs11_mbedtls_utils.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/sss_pkcs11_pal_asymm.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/sss_pkcs11_pal_core.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/sss_pkcs11_pal_helpers.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/sss_pkcs11_pal_object.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/sss_pkcs11_pal_symm.c"

        "${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/platform/ksdk/ax_reset.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/platform/ksdk/timer_kinetis.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/platform/ksdk/timer_kinetis_freertos.c"
        "${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/platform/ksdk/i2c_imxrt.c"
    )

    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${SE05X_SDK_PATH}/sss/ex/inc/
        ${SE05X_SDK_PATH}/sss/inc/
        ${SE05X_SDK_PATH}/hostlib/hostLib/inc/
        ${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/log/
        ${SE05X_SDK_PATH}/hostlib/hostLib/se05x_03_xx_xx/
        ${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/
        ${SE05X_SDK_PATH}/sss/plugin/mbedtls/
        ${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/smCom/T1oI2C/
        ${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/infra/
        ${SE05X_SDK_PATH}/hostlib/hostLib/platform/inc/
        ${SE05X_SDK_FROM_ZIP_PATH}/sss/port/ksdk/
        ${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/libCommon/smCom/
        ${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/inc/
        ${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/platform/inc/
        ${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/libCommon/infra/
        ${SE05X_SDK_FROM_ZIP_PATH}/sss/plugin/pkcs11/
        ${ProjDirPath}/../../../mcuxsdk/components/phy/device/phyrtl8211f/

        # El2Go
        ${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/tstUtil/
    )

    # SE05X
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDK_DEBUGCONSOLE=1")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDK_DEBUGCONSOLE_UART")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMCUXPRESSO_SDK")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_SMCOM_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_APP_DEBUG=1")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_SSS_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_PKCS11_DEBUG=1")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_USECASES_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_VCOM_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_PKCS11_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_SSCP_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_MBEDTLS_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_HOSTLIB_DEBUG=1")

    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFLOW_VERBOSE")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_DES_C")

    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSDK_I2C_BASED_COMPONENT_USED")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__USE_CMIS")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__MCUXPRESSO")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__NEWLIB")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_DES_C")

    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DVERBOSE_APDU_LOGS=1")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPKCS11_SESSION_OPEN")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_CONSOLE_TRANSFER_NON_BLOCKING=1")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_CONSOLE_RECEIVE_BUFFER_LEN=4096")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DDEBUG_CONSOLE_ENABLE_ECHO=1")

    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__stack_size__=0x2000")
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -D__heap_size__=0x10000")

    include(driver_lpi2c)
    include(driver_lpi2c_freertos)

    if(AKNANO_ENABLE_EL2GO EQUAL 1)
        SET (CMAKE_MODULE_PATH $CMAKE_MODULE_PATH
            ${SE05X_SDK_FROM_ZIP_PATH}
        )
        include(middleware_se_hostlib_nxp_iot_agent_lib)

        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DAKNANO_ENABLE_EL2GO")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DPB_FIELD_16BIT")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DSSS_HAVE_MBEDTLS_ALT_SSS")

        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_SSL_KEEP_PEER_CERTIFICATE")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_X509_CRL_PARSE_C")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_ECDH_ALT")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_ECDH_GEN_PUBLIC_ALT")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_ECDH_COMPUTE_SHARED_ALT")
        SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DMBEDTLS_SSL_MAX_FRAGMENT_LENGTH")

        # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_APP_DEBUG")

        target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
            "${ProjDirPath}/../aknano_el2go.c"
            "${SE05X_SDK_PATH}/hostlib/hostLib/libCommon/infra/sm_apdu.c"
            "${SE05X_SDK_PATH}/sss/plugin/mbedtls/ecp_alt_sss.c"
            "${SE05X_SDK_PATH}/sss/plugin/mbedtls/ecdh_alt_ax.c"
            "${SE05X_SDK_PATH}/sss/plugin/mbedtls/ecdsa_verify_alt.c"
            "${SE05X_SDK_PATH}/sss/plugin/mbedtls/ecp_alt_sss.c"
            "${SE05X_SDK_PATH}/sss/plugin/mbedtls/sss_mbedtls.c"
            "${SE05X_SDK_PATH}/sss/plugin/mbedtls/sss_mbedtls_rsa.c"
            "${SE05X_SDK_FROM_ZIP_PATH}/hostlib/hostLib/mbedtls/src/ecdh_alt.c"
        )
    endif(AKNANO_ENABLE_EL2GO EQUAL 1)
else()
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        # PKCS#11 using mflash
        "${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/portable/mbedtls/core_pkcs11_mbedtls.c"
        "${ProjDirPath}/../../../poc/3rdparty/rtos/freertos/vendors/nxp/pkcs11/common/iot_pkcs11_pal.c"
    )
endif(AKNANO_ENABLE_SE05X EQUAL 1)
