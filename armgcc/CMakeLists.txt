################################################################################
# Aktualizr-nano basic build options                                           #
################################################################################

# Use SE05X for device key and certificate provisioning
SET (AKNANO_ENABLE_SE05X 0)

# Enable EdgeLock 2GO Managed. Requires SE05X
SET (AKNANO_ENABLE_EL2GO 0)

# Enable provisioning code
SET (AKNANO_ALLOW_PROVISIONING 1)

# Board type rt1060 or rt1170
SET (AKNANO_BOARD_MODEL rt1060)

# Enable AWS MQTT demo task
SET (AKNANO_ENABLE_AWS_MQTT_DEMO_TASK 0)

################################################################################


################################################################################
# Aktualizr-nano advanced development build options                            #
################################################################################

# Delete local TUF metadata, starting over with provisioned root.1.json
SET (AKNANO_DELETE_TUF_DATA 0)

# Delete provisioned TUF 1.root.json
SET (AKNANO_DELETE_PROVISIONED_TUF_ROOT 0)

# Delete device key, certificate, serial and UUID
SET (AKNANO_RESET_DEVICE_ID 0)

# Enable intentional error during startup, forcing a version rollback
SET (AKNANO_TEST_ROLLBACK 0)

# Enable memory information dump
SET (AKNANO_DUMP_MEMORY_USAGE_INFO 1)

# Enable very basic tests before starting main functionality
SET (AKNANO_SELF_TEST 0)

# Disable reboots
# To be used durign debug sessions where reboot operation can't be performed
SET (AKNANO_DISABLE_REBOOT 0)

# Force a fixed UUID for the device, by using the MAC address as UUID
SET (AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID 0)

# An arbitrary string to add to key serial output locations, facilitating test analysis
SET (AKNANO_TEST_MESSAGE_PREAMBLE "TEST ")

# Dump every TUF metadata that is downloaded from the devide gateway
SET (AKNANO_DUMP_DOWNLOADED_TUF_METADATA 0)

# Default tag used when fetching targets from the device gateway
SET (AKNANO_DEFAULT_TAG "devel")

################################################################################


################################################################################
# Overwrite default values if environment variables are set:
################################################################################

if (DEFINED ENV{AKNANO_ENABLE_EL2GO})
    set (AKNANO_ENABLE_EL2GO $ENV{AKNANO_ENABLE_EL2GO})
endif (DEFINED ENV{AKNANO_ENABLE_EL2GO})

if (DEFINED ENV{AKNANO_ENABLE_SE05X})
    set (AKNANO_ENABLE_SE05X $ENV{AKNANO_ENABLE_SE05X})
endif (DEFINED ENV{AKNANO_ENABLE_SE05X})

if (DEFINED ENV{AKNANO_ALLOW_PROVISIONING})
    set (AKNANO_ALLOW_PROVISIONING $ENV{AKNANO_ALLOW_PROVISIONING})
endif (DEFINED ENV{AKNANO_ALLOW_PROVISIONING})

if (DEFINED ENV{AKNANO_ENABLE_AWS_MQTT_DEMO_TASK})
    set (AKNANO_ENABLE_AWS_MQTT_DEMO_TASK $ENV{AKNANO_ENABLE_AWS_MQTT_DEMO_TASK})
endif (DEFINED ENV{AKNANO_ENABLE_AWS_MQTT_DEMO_TASK})

if (DEFINED ENV{AKNANO_BOARD_MODEL})
    set (AKNANO_BOARD_MODEL $ENV{AKNANO_BOARD_MODEL})
endif (DEFINED ENV{AKNANO_BOARD_MODEL})

if (DEFINED ENV{AKNANO_BOARD_MODEL})
    set (AKNANO_BOARD_MODEL $ENV{AKNANO_BOARD_MODEL})
endif (DEFINED ENV{AKNANO_BOARD_MODEL})

if (DEFINED ENV{AKNANO_DELETE_TUF_DATA})
    set (AKNANO_DELETE_TUF_DATA $ENV{AKNANO_DELETE_TUF_DATA})
endif (DEFINED ENV{AKNANO_DELETE_TUF_DATA})

if (DEFINED ENV{AKNANO_DELETE_PROVISIONED_TUF_ROOT})
    set (AKNANO_DELETE_PROVISIONED_TUF_ROOT $ENV{AKNANO_DELETE_PROVISIONED_TUF_ROOT})
endif (DEFINED ENV{AKNANO_DELETE_PROVISIONED_TUF_ROOT})

if (DEFINED ENV{AKNANO_RESET_DEVICE_ID})
    set (AKNANO_RESET_DEVICE_ID $ENV{AKNANO_RESET_DEVICE_ID})
endif (DEFINED ENV{AKNANO_RESET_DEVICE_ID})

if (DEFINED ENV{AKNANO_TEST_ROLLBACK})
    set (AKNANO_TEST_ROLLBACK $ENV{AKNANO_TEST_ROLLBACK})
endif (DEFINED ENV{AKNANO_TEST_ROLLBACK})

if (DEFINED ENV{AKNANO_DUMP_MEMORY_USAGE_INFO})
    set (AKNANO_DUMP_MEMORY_USAGE_INFO $ENV{AKNANO_DUMP_MEMORY_USAGE_INFO})
endif (DEFINED ENV{AKNANO_DUMP_MEMORY_USAGE_INFO})

if (DEFINED ENV{AKNANO_SELF_TEST})
    set (AKNANO_SELF_TEST $ENV{AKNANO_SELF_TEST})
endif (DEFINED ENV{AKNANO_SELF_TEST})

if (DEFINED ENV{AKNANO_DISABLE_REBOOT})
    set (AKNANO_DISABLE_REBOOT $ENV{AKNANO_DISABLE_REBOOT})
endif (DEFINED ENV{AKNANO_DISABLE_REBOOT})

if (DEFINED ENV{AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID})
    set (AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID $ENV{AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID})
endif (DEFINED ENV{AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID})

if (DEFINED ENV{AKNANO_TEST_MESSAGE_PREAMBLE})
    set (AKNANO_TEST_MESSAGE_PREAMBLE $ENV{AKNANO_TEST_MESSAGE_PREAMBLE})
endif (DEFINED ENV{AKNANO_TEST_MESSAGE_PREAMBLE})

if (DEFINED ENV{AKNANO_DUMP_DOWNLOADED_TUF_METADATA})
    set (AKNANO_DUMP_DOWNLOADED_TUF_METADATA $ENV{AKNANO_DUMP_DOWNLOADED_TUF_METADATA})
endif (DEFINED ENV{AKNANO_DUMP_DOWNLOADED_TUF_METADATA})

if (DEFINED ENV{AKNANO_DEFAULT_TAG})
    set (AKNANO_DEFAULT_TAG $ENV{AKNANO_DEFAULT_TAG})
endif (DEFINED ENV{AKNANO_DEFAULT_TAG})

################################################################################


# CROSS COMPILER SETTING
SET(CMAKE_SYSTEM_NAME Generic)
CMAKE_MINIMUM_REQUIRED (VERSION 3.10.0)

# # THE VERSION NUMBER
# SET (Tutorial_VERSION_MAJOR 1)
# SET (Tutorial_VERSION_MINOR 0)

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

project(ota_demo_enet)

set(MCUX_SDK_PROJECT_NAME ota_demo_enet.elf)

SET(CMAKE_STATIC_LIBRARY_PREFIX "lib")

SET(CMAKE_STATIC_LIBRARY_SUFFIX ".a")

SET(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

SET(THREADX_ARCH "cortex_m7")

SET(THREADX_TOOLCHAIN "gnu")


if(AKNANO_BOARD_MODEL STREQUAL rt1170)
    include(${ProjDirPath}/flags_rt1170.cmake)
    include(${ProjDirPath}/config_rt1170.cmake)
endif(AKNANO_BOARD_MODEL STREQUAL rt1170)

if(AKNANO_BOARD_MODEL STREQUAL rt1060)
    include(${ProjDirPath}/flags_rt1060.cmake)
    include(${ProjDirPath}/config_rt1060.cmake)
endif(AKNANO_BOARD_MODEL STREQUAL rt1060)

include(${ProjDirPath}/../../libtufnano/tufFilePaths.cmake)
include(${ProjDirPath}/../../../middleware/coreMQTT-Agent/mqttAgentFilePaths.cmake)
include(${ProjDirPath}/../../../middleware/coreMQTT/mqttFilePaths.cmake)


SET(FreeRtosAwsPath "${ProjDirPath}/../../../rtos/amazon-freertos")
SET(FreeRtosKernelPath "${ProjDirPath}/../../../rtos/freertos/freertos_kernel")

add_executable(${MCUX_SDK_PROJECT_NAME}
"${ProjDirPath}/../config_files/FreeRTOSConfig.h"
"${ProjDirPath}/../config_files/aws_mbedtls_config.h"
"${ProjDirPath}/../config_files/core_pkcs11_config.h"
"${ProjDirPath}/../config_files/FreeRTOSIPConfig.h"
"${ProjDirPath}/../config_files/iot_config.h"
"${ProjDirPath}/../config_files/iot_secure_sockets_config.h"
"${ProjDirPath}/../config_files/aws_iot_network_config.h"
"${ProjDirPath}/../main_enet.c"
"${ProjDirPath}/../lwipopts.h"
"${ProjDirPath}/../mcuboot_app_support.c"
"${ProjDirPath}/../mcuboot_app_support.h"
"${ProjDirPath}/../flash_partitioning.h"
"${ProjDirPath}/../read_button_task.c"
"${ProjDirPath}/../aknano_client.c"
"${ProjDirPath}/../aws_mqtt_starter.c"
"${ProjDirPath}/../flexspi_nor_flash_ops.c"
${TUF_SOURCES}
${TUF_FIAT_SOURCES}
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

"${FreeRtosKernelPath}/croutine.c"
"${FreeRtosKernelPath}/event_groups.c"
"${FreeRtosKernelPath}/list.c"
"${FreeRtosKernelPath}/portable/GCC/ARM_CM4F/port.c"
"${FreeRtosKernelPath}/portable/MemMang/heap_4.c"
"${FreeRtosKernelPath}/queue.c"
"${FreeRtosKernelPath}/stream_buffer.c"
"${FreeRtosKernelPath}/tasks.c"
"${FreeRtosKernelPath}/timers.c"

"${ProjDirPath}/../../../middleware/lwip/port/enet_ethernetif_kinetis.c"
"${ProjDirPath}/../../../middleware/lwip/port/enet_ethernetif.c"
"${ProjDirPath}/../../../poc/3rdparty/rtos/freertos/vendors/nxp/secure_sockets/lwip/iot_secure_sockets.c"

"${ProjDirPath}/../../../middleware/freertos/backoffAlgorithm/source/backoff_algorithm.c"
"${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/core_http_client.c"
"${ProjDirPath}/../../../middleware/freertos/coreJSON/source/core_json.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/core_pkcs11.c"
"${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/core_pki_utils.c"
"${ProjDirPath}/../../../middleware/inih/ini.c"

"${ProjDirPath}/../../../middleware/http-parser/http_parser.c"

"${ProjDirPath}/../../../middleware/lwip/src/apps/sntp/sntp.c"

"${ProjDirPath}/../../../middleware/unity/src/unity.c"
"${ProjDirPath}/../../../middleware/unity/extras/fixture/src/unity_fixture.c"
"${ProjDirPath}/../../../rtos/amazon-freertos/demos/coreMQTT_Agent/mqtt_agent_task.c"
"${ProjDirPath}/../../../rtos/amazon-freertos/demos/coreMQTT_Agent/subscription_manager.c"
"${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions/mqtt_agent/freertos_agent_message.c"
"${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions/mqtt_agent/freertos_command_pool.c"
"${ProjDirPath}/../../../rtos/amazon-freertos/demos/common/pkcs11_helpers/pkcs11_helpers.c"
"${ProjDirPath}/../../../rtos/amazon-freertos/demos/coreMQTT_Agent/simple_sub_pub_demo.c"
${MQTT_AGENT_SOURCES}
${MQTT_SOURCES}
${MQTT_SERIALIZER_SOURCES}
)

if(AKNANO_BOARD_MODEL STREQUAL rt1170)
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
endif(AKNANO_BOARD_MODEL STREQUAL rt1170)

if(AKNANO_BOARD_MODEL STREQUAL rt1060)

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

    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${ProjDirPath}/../../../mcuxsdk/drivers/trng/
    )
endif(AKNANO_BOARD_MODEL STREQUAL rt1060)

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} ${CMAKE_C_FLAGS_FLEXSPI_NOR} -fdiagnostics-color -DUSE_FLASH=1 -DLWIP_NETIF_API=1 -DMBEDTLS_ERROR_C -DMBEDTLS_CMAC_C -DJSON_QUERY_KEY_SEPARATOR=\\'/\\'")
SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DCACHE_MODE_WRITE_THROUGH")

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DCONFIG_MCUBOOT_FLASH_REMAP_DOWNGRADE_SUPPORT")

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_debug)

if(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)
     target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE MBEDTLS_CONFIG_FILE="aws_mbedtls_config.h")
endif(CMAKE_BUILD_TYPE STREQUAL flexspi_nor_release)

if (AKNANO_DELETE_TUF_DATA EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_DELETE_TUF_DATA")
endif (AKNANO_DELETE_TUF_DATA EQUAL 1)

if (AKNANO_DELETE_PROVISIONED_TUF_ROOT EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_DELETE_PROVISIONED_TUF_ROOT")
endif (AKNANO_DELETE_PROVISIONED_TUF_ROOT EQUAL 1)

if (AKNANO_RESET_DEVICE_ID EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_RESET_DEVICE_ID")
endif (AKNANO_RESET_DEVICE_ID EQUAL 1)

if (AKNANO_TEST_ROLLBACK EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_TEST_ROLLBACK")
endif (AKNANO_TEST_ROLLBACK EQUAL 1)

if (AKNANO_DUMP_MEMORY_USAGE_INFO EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_DUMP_MEMORY_USAGE_INFO")
endif (AKNANO_DUMP_MEMORY_USAGE_INFO EQUAL 1)

if (AKNANO_DISABLE_REBOOT EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_DISABLE_REBOOT")
endif (AKNANO_DISABLE_REBOOT EQUAL 1)

if (AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID")
endif (AKNANO_USE_MAC_ADDRESS_AS_DEVICE_UUID EQUAL 1)

if (DEFINED ENV{AKNANO_EDGELOCK2GO_HOSTNAME})
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DEDGELOCK2GO_HOSTNAME=\\\"$ENV{AKNANO_EDGELOCK2GO_HOSTNAME}\\\"")
endif (DEFINED ENV{AKNANO_EDGELOCK2GO_HOSTNAME})

SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DSNTP_SET_SYSTEM_TIME=aknano_client_sntp_set_system_time")


execute_process(
    COMMAND git -C ../../../poc/ log --format=%H -n 1
    OUTPUT_VARIABLE AKNANO_MANIFEST_COMMIT_ID
)

execute_process(
    COMMAND git -C ../../aktualizr-nano log --format=%H -n 1
    OUTPUT_VARIABLE AKNANO_COMMIT_ID
)

IF (NOT AKNANO_MANIFEST_COMMIT_ID STREQUAL "")
    string(STRIP "${AKNANO_MANIFEST_COMMIT_ID}" AKNANO_MANIFEST_COMMIT_ID)
    # SET (AKNANO_MANIFEST_COMMIT_ID "MANIFEST_HASH_${AKNANO_MANIFEST_COMMIT_ID}")
    target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE AKNANO_MANIFEST_COMMIT_ID="${AKNANO_MANIFEST_COMMIT_ID}")
ENDIF (NOT AKNANO_MANIFEST_COMMIT_ID STREQUAL "")

IF (NOT AKNANO_COMMIT_ID STREQUAL "")
    string(STRIP "${AKNANO_COMMIT_ID}" AKNANO_COMMIT_ID)
    # SET (AKNANO_COMMIT_ID "AKNANO_HASH_${AKNANO_COMMIT_ID}")
    target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE AKNANO_COMMIT_ID="${AKNANO_COMMIT_ID}")
ENDIF (NOT AKNANO_COMMIT_ID STREQUAL "")

IF (NOT AKNANO_TEST_MESSAGE_PREAMBLE STREQUAL "")
    target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE AKNANO_TEST_MESSAGE_PREAMBLE="${AKNANO_TEST_MESSAGE_PREAMBLE}")
ENDIF (NOT AKNANO_TEST_MESSAGE_PREAMBLE STREQUAL "")

IF (NOT AKNANO_DEFAULT_TAG STREQUAL "")
    target_compile_definitions(${MCUX_SDK_PROJECT_NAME}  PRIVATE AKNANO_DEFAULT_TAG="${AKNANO_DEFAULT_TAG}")
ENDIF (NOT AKNANO_DEFAULT_TAG STREQUAL "")

if (AKNANO_DUMP_DOWNLOADED_TUF_METADATA EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_DUMP_DOWNLOADED_TUF_METADATA")
endif (AKNANO_DUMP_DOWNLOADED_TUF_METADATA EQUAL 1)

message(STATUS "MANIFEST_HASH: ${AKNANO_COMMIT_ID}")
message(STATUS "AKNANO_HASH: ${AKNANO_MANIFEST_COMMIT_ID}")
message(STATUS "TEST_MESSAGE_PREAMBLE: ${AKNANO_TEST_MESSAGE_PREAMBLE}")


target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
    ${ProjDirPath}/..
    ${ProjDirPath}/../../..
    ${ProjDirPath}/../config_files
    ${TUF_INCLUDE_PUBLIC_DIRS}
    ${FreeRtosAwsPath}/libraries/logging/include/
    ${ProjDirPath}/../../../middleware/pkcs11/
    ${ProjDirPath}/../../../middleware/freertos/backoffAlgorithm/source/include/
    ${MQTT_INCLUDE_PUBLIC_DIRS}
    ${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/include/
    ${ProjDirPath}/../../../middleware/freertos/coreHTTP/source/interface/
    ${ProjDirPath}/../../../middleware/freertos/coreJSON/source/include/
    ${ProjDirPath}/../../../middleware/freertos/corePKCS11/source/include/
    ${ProjDirPath}/../../../middleware/http-parser/
    ${ProjDirPath}/../../../middleware/lwip/port/
    ${ProjDirPath}/../../../middleware/mbedtls/port/ksdk/
    ${ProjDirPath}/../../../middleware/unity/extras/fixture/src/
    ${ProjDirPath}/../../../middleware/unity/extras/memory/src/
    ${ProjDirPath}/../../../middleware/unity/src/
    ${ProjDirPath}/../../../middleware/inih/
    ${FreeRtosAwsPath}/demos/common/pkcs11_helpers
    ${FreeRtosAwsPath}/demos/dev_mode_key_provisioning/include/
    ${FreeRtosAwsPath}/demos/include/
    ${FreeRtosAwsPath}/demos/network_manager/
    ${FreeRtosAwsPath}/libraries/3rdparty/mbedtls_utils/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/freertos/include/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/
    ${FreeRtosAwsPath}/libraries/abstractions/platform/include/platform/
    ${FreeRtosAwsPath}/libraries/abstractions/secure_sockets/include/
    ${FreeRtosAwsPath}/libraries/abstractions/transport/secure_sockets/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/private/
    ${FreeRtosAwsPath}/libraries/c_sdk/standard/common/include/types/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/crypto/include/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/tls/include/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/tls/include/
    ${FreeRtosAwsPath}/libraries/freertos_plus/standard/utils/include/
    ${FreeRtosKernelPath}/include/
    ${FreeRtosKernelPath}/portable/GCC/ARM_CM4F/
    ${MQTT_AGENT_INCLUDE_PUBLIC_DIRS}
    ${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions/mqtt_agent/include
)

if(AKNANO_BOARD_MODEL STREQUAL rt1060)
    #path for entry cmake
    set(CMAKE_MODULE_PATH
        ${ProjDirPath}/../../aktualizr-nano/
        ${ProjDirPath}/../../../mcuxsdk/
        ${ProjDirPath}/../../../mcuxsdk/components/phy/mdio/enet/
        ${ProjDirPath}/../../../middleware/CMSIS_5/Core/Include/
        ${ProjDirPath}/../../../
        ${ProjDirPath}/../../../middleware
        ${ProjDirPath}/../../../middleware/mbedtls
        ${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1162
        ${ProjDirPath}/../../../mcuxsdk/devices/
        ${ProjDirPath}/../../../mcuxsdk/drivers/lpi2c/
        ${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions
        ${ProjDirPath}/../../../rtos/amazon-freertos/tools/cmake
        ${ProjDirPath}/../../../mcuxsdk/components/silicon_id
        ${ProjDirPath}/../../../mcuxsdk/components/silicon_id/socs/rt10xx
        ${ProjDirPath}/../../../rtos/freertos/freertos_kernel
    )
endif(AKNANO_BOARD_MODEL STREQUAL rt1060)

if(AKNANO_BOARD_MODEL STREQUAL rt1170)
    #path for entry cmake
    set(CMAKE_MODULE_PATH
        ${ProjDirPath}/../../aktualizr-nano/
        ${ProjDirPath}/../../../mcuxsdk/
        ${ProjDirPath}/../../../
        ${ProjDirPath}/../../../middleware
        ${ProjDirPath}/../../../mcuxsdk/components/phy/mdio/enet/
        ${ProjDirPath}/../../../middleware/CMSIS_5/Core/Include/
        ${ProjDirPath}/../../../middleware/mbedtls
        ${ProjDirPath}/../../../mcuxsdk/devices/MIMXRT1176
        ${ProjDirPath}/../../../mcuxsdk/drivers/lpi2c/
        ${ProjDirPath}/../../../rtos/amazon-freertos/libraries/abstractions
        ${ProjDirPath}/../../../rtos/amazon-freertos/tools/cmake
        ${ProjDirPath}/../../../rtos/freertos/freertos_kernel
        ${ProjDirPath}/../../../mcuxsdk/components/silicon_id
        ${ProjDirPath}/../../../mcuxsdk/components/silicon_id/socs/rt10xx
    )
endif(AKNANO_BOARD_MODEL STREQUAL rt1170)


include(afr)

#Include Entry cmake component
include(all_devices)

# include modules
# include(driver_mdio-enet)

include(driver_phy-device-ksz8081)

include(driver_cache_armv7_m7)

include(driver_flexspi)

# include(middleware_lwip_enet_ethernetif)

# include(middleware_lwip)

include(middleware_aktualizr-nano)

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

if(AKNANO_SELF_TEST EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DUNITY_INCLUDE_CONFIG_H -DAKNANO_TEST")
    target_sources(${MCUX_SDK_PROJECT_NAME} PRIVATE
        "${FreeRtosAwsPath}/tests/common/aws_test_framework.c"
        "${FreeRtosAwsPath}/tests/common/iot_test_freertos.c"
    )

    target_include_directories(${MCUX_SDK_PROJECT_NAME} PRIVATE
        ${FreeRtosAwsPath}/tests/include/
    )

    include(middleware_aktualizr-nano_tests)
endif(AKNANO_SELF_TEST EQUAL 1)

if(AKNANO_BOARD_MODEL STREQUAL rt1060)
    include(device_startup)
    include(device_system)
    include(driver_dcp)

    include(driver_trng)
    include(middleware_mbedtls_MIMXRT1062)

    include(middleware_mbedtls_port_ksdk)

    include(middleware_mbedtls_rt)
    include(middleware_lwip_enet_ethernetif_MIMXRT1062)

    include(middleware_lwip)
    include(component_silicon_id_MIMXRT1062)
endif(AKNANO_BOARD_MODEL STREQUAL rt1060)


if(AKNANO_BOARD_MODEL STREQUAL rt1170)
    include(all_lib_device_MIMXRT1176_cm7)

    include(device_startup_MIMXRT1176_cm7)
    include(device_system_MIMXRT1176_cm7)

    include(middleware_mbedtls_MIMXRT1176_cm7)
    include(middleware_lwip_enet_ethernetif_MIMXRT1176_cm7)
    include(middleware_mbedtls_port_ksdk)


    include(driver_caam)
    include(driver_dcdc_soc)

    include(driver_phy-device-rtl8211f)
    include(component_silicon_id_MIMXRT1176_cm7)
endif(AKNANO_BOARD_MODEL STREQUAL rt1170)

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
        "${SE05X_SDK_PATH}/sss/ex/src/ex_sss_se05x_auth.c"
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
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_SSS_DEBUG=1")
    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_PKCS11_DEBUG=1")

    # SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNX_LOG_ENABLE_USECASES_DEBUG=1")
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
        SET (CMAKE_MODULE_PATH
            ${CMAKE_MODULE_PATH}
            ${SE05X_SDK_FROM_ZIP_PATH}
        )
        include(middleware_se_hostlib_nxp_iot_agent_lib)
        include(middleware_aktualizr-nano_el2go)

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
        "${ProjDirPath}/../../../poc/3rdparty/middleware/freertos/corePKCS11/source/portable/mbedtls/core_pkcs11_mbedtls.c"
        "${ProjDirPath}/../../../poc/3rdparty/rtos/freertos/vendors/nxp/pkcs11/common/iot_pkcs11_pal.c"
    )
endif(AKNANO_ENABLE_SE05X EQUAL 1)

if(AKNANO_ALLOW_PROVISIONING EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_ALLOW_PROVISIONING")
    include(middleware_aktualizr-nano_provisioning)
endif(AKNANO_ALLOW_PROVISIONING EQUAL 1)

if(AKNANO_ENABLE_AWS_MQTT_DEMO_TASK EQUAL 1)
    SET(CMAKE_C_FLAGS  "${CMAKE_C_FLAGS} -DAKNANO_ENABLE_AWS_MQTT_DEMO_TASK")
endif(AKNANO_ENABLE_AWS_MQTT_DEMO_TASK EQUAL 1)
