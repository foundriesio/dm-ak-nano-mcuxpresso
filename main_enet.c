/*
 * FreeRTOS V1.0.0
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 * Copyright (c) 2013 - 2014, Freescale Semiconductor, Inc.
 * Copyright 2016-2019 NXP
 * Copyright 2022 Foundries.io
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software. If you wish to use our Amazon
 * FreeRTOS name, please do so in a fair use way that does not cause confusion.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

///////////////////////////////////////////////////////////////////////////////
//  Includes
///////////////////////////////////////////////////////////////////////////////
#include "lwipopts.h"

/* SDK Included Files */
#include "fsl_debug_console.h"
#include "ksdk_mbedtls.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "board.h"
/* FreeRTOS Demo Includes */
#include "FreeRTOS.h"
#include "task.h"
#include "iot_logging_task.h"
#include "iot_system_init.h"
#include "aws_dev_mode_key_provisioning.h"
#include "platform/iot_threads.h"
#include "types/iot_network_types.h"
#include "aws_demo.h"

#ifdef AKNANO_BOARD_MODEL_RT1170
#if BOARD_NETWORK_USE_100M_ENET_PORT
#include "fsl_phyksz8081.h"
#else
#include "fsl_phyrtl8211f.h"
#endif
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_enet.h"
#endif

#include "fsl_phy.h"
/* lwIP Includes */
#include "lwip/tcpip.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"
#include "netif/ethernet.h"
#include "ethernetif.h"
#include "lwip/netifapi.h"
#include "fsl_iomuxc.h"
#include "fsl_enet.h"

#ifdef AKNANO_BOARD_MODEL_RT1060
// #include "fsl_enet_mdio.h"
#include "fsl_iomuxc.h"
#include "fsl_phyksz8081.h"
#endif

#if AKNANO_ENABLE_SE05X
#include <ex_sss_boot.h>

ex_sss_boot_ctx_t gex_sss_demo_boot_ctx;
ex_sss_boot_ctx_t * pex_sss_demo_boot_ctx = &gex_sss_demo_boot_ctx;

ex_sss_cloud_ctx_t gex_sss_demo_tls_ctx;
ex_sss_cloud_ctx_t * pex_sss_demo_tls_ctx = &gex_sss_demo_tls_ctx;

const char * g_port_name = NULL;
#endif

/*******************************************************************************
 * Definitions
 ******************************************************************************/
/* MAC address configuration. */
/*
#define configMAC_ADDR                     \
     {                                      \
         0x02, 0x12, 0x13, 0x10, 0x15, 0x25 \
     }
*/

#ifdef AKNANO_BOARD_MODEL_RT1060

#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */

/* Ethernet configuration. */
extern phy_ksz8081_resource_t g_phy_resource;
#define EXAMPLE_ENET         ENET
#define EXAMPLE_PHY_ADDRESS  BOARD_ENET0_PHY_ADDRESS
#define EXAMPLE_PHY_OPS      &phyksz8081_ops
#define EXAMPLE_CLOCK_FREQ   CLOCK_GetFreq(kCLOCK_IpgClk)

#else
#if BOARD_NETWORK_USE_100M_ENET_PORT
extern phy_ksz8081_resource_t g_phy_resource;
#define EXAMPLE_ENET ENET
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS BOARD_ENET0_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS &phyksz8081_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#else
extern phy_rtl8211f_resource_t g_phy_resource;
#define EXAMPLE_ENET          ENET_1G
/* Address of PHY interface. */
#define EXAMPLE_PHY_ADDRESS   BOARD_ENET1_PHY_ADDRESS
/* PHY operations. */
#define EXAMPLE_PHY_OPS       &phyrtl8211f_ops
/* ENET instance select. */
#define EXAMPLE_NETIF_INIT_FN ethernetif1_init
#endif

/* ENET clock frequency. */
#define EXAMPLE_CLOCK_FREQ CLOCK_GetRootClockFreq(kCLOCK_Root_Bus)

#ifndef EXAMPLE_NETIF_INIT_FN
/*! @brief Network interface initialization function. */
#define EXAMPLE_NETIF_INIT_FN ethernetif0_init
#endif /* EXAMPLE_NETIF_INIT_FN */
#endif

#define EXAMPLE_PHY_RESOURCE &g_phy_resource

#define INIT_SUCCESS 0
#define INIT_FAIL    1

/* @TEST_ANCHOR */

#define LOGGING_TASK_PRIORITY   (tskIDLE_PRIORITY + 1)
#define LOGGING_TASK_STACK_SIZE (200)
#define LOGGING_QUEUE_LENGTH    (16)

#define AKNANO_TASK_STACK_SIZE 9000

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
void BOARD_InitNetwork(void);

extern int initNetwork(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static phy_handle_t phyHandle;

struct netif netif;


#ifdef AKNANO_BOARD_MODEL_RT1060
phy_ksz8081_resource_t g_phy_resource;
#else
#if BOARD_NETWORK_USE_100M_ENET_PORT
phy_ksz8081_resource_t g_phy_resource;
#else
phy_rtl8211f_resource_t g_phy_resource;
#endif
#endif

/*******************************************************************************
 * Code
 ******************************************************************************/
static void MDIO_Init(void)
{
    (void)CLOCK_EnableClock(s_enetClock[ENET_GetInstance(EXAMPLE_ENET)]);
    ENET_SetSMI(EXAMPLE_ENET, EXAMPLE_CLOCK_FREQ, false);
}

static status_t MDIO_Write(uint8_t phyAddr, uint8_t regAddr, uint16_t data)
{
    return ENET_MDIOWrite(EXAMPLE_ENET, phyAddr, regAddr, data);
}

static status_t MDIO_Read(uint8_t phyAddr, uint8_t regAddr, uint16_t *pData)
{
    return ENET_MDIORead(EXAMPLE_ENET, phyAddr, regAddr, pData);
 }

#ifdef AKNANO_BOARD_MODEL_RT1060
void BOARD_InitModuleClock(void)
{
    const clock_enet_pll_config_t config = {
        .enableClkOutput    = true,
        .enableClkOutput25M = false,
        .loopDivider        = 1,
    };
    CLOCK_InitEnetPll(&config);
}

#else
void BOARD_InitModuleClock(void)
{
    const clock_sys_pll1_config_t sysPll1Config = {
        .pllDiv2En = true,
    };
    CLOCK_InitSysPll1(&sysPll1Config);

#if BOARD_NETWORK_USE_100M_ENET_PORT
    clock_root_config_t rootCfg = {.mux = 4, .div = 10}; /* Generate 50M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet1, &rootCfg);
#else
    clock_root_config_t rootCfg = {.mux = 4, .div = 4}; /* Generate 125M root clock. */
    CLOCK_SetRootClock(kCLOCK_Root_Enet2, &rootCfg);
#endif

    /* Select syspll2pfd3, 528*18/24 = 396M */
    CLOCK_InitPfd(kCLOCK_PllSys2, kCLOCK_Pfd3, 24);
    rootCfg.mux = 7;
    rootCfg.div = 2;
    CLOCK_SetRootClock(kCLOCK_Root_Bus, &rootCfg); /* Generate 198M bus clock. */
}

void IOMUXC_SelectENETClock(void)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    IOMUXC_GPR->GPR4 |= 0x3; /* 50M ENET_REF_CLOCK output to PHY and ENET module. */
#else
    IOMUXC_GPR->GPR5 |= IOMUXC_GPR_GPR5_ENET1G_RGMII_EN_MASK; /* bit1:iomuxc_gpr_enet_clk_dir
                                                                 bit0:GPR_ENET_TX_CLK_SEL(internal or OSC) */
#endif
}

void BOARD_ENETFlexibleConfigure(enet_config_t *config)
{
#if BOARD_NETWORK_USE_100M_ENET_PORT
    config->miiMode = kENET_RmiiMode;
#else
    config->miiMode = kENET_RgmiiMode;
#endif
}
#endif

int initNetwork(void)
{
    // return 0;
    ethernetif_config_t enet_config = {
        .phyHandle   = &phyHandle,
        .phyAddr     = EXAMPLE_PHY_ADDRESS,
        .phyOps      = EXAMPLE_PHY_OPS,
        .phyResource = EXAMPLE_PHY_RESOURCE,
        .srcClockHz  = EXAMPLE_CLOCK_FREQ,
    };

#ifdef configMAC_ADDR
    enet_config.macAddress = configMAC_ADDR;
#else
     (void)SILICONID_ConvertToMacAddr(&enet_config.macAddress);
#endif
    tcpip_init(NULL, NULL);

    err_t ret;
    ret = netifapi_netif_add(&netif, NULL, NULL, NULL, &enet_config, EXAMPLE_NETIF_INIT_FN, tcpip_input);
    if (ret != (err_t)ERR_OK)
    {
        (void)PRINTF("netifapi_netif_add: %d\r\n", ret);
        while (true)
        {
        }
    }
    ret = netifapi_netif_set_default(&netif);
    if (ret != (err_t)ERR_OK)
    {
        (void)PRINTF("netifapi_netif_set_default: %d\r\n", ret);
        while (true)
        {
        }
    }
    ret = netifapi_netif_set_up(&netif);
    if (ret != (err_t)ERR_OK)
    {
        (void)PRINTF("netifapi_netif_set_up: %d\r\n", ret);
        while (true)
        {
        }
    }

#ifdef AKNANO_USE_STATIC_NETWORK_SETTINGS
	#include "lwip/dns.h"
	ip4_addr_t netif_dns;
	IP4_ADDR(&netif_dns, 8, 8, 8, 8);
	dns_setserver(0, &netif_dns);
    IP4_ADDR(&netif.ip, 192, 168, 15, 8);
    IP4_ADDR(&netif.netmask, 255, 255, 255, 0);
    IP4_ADDR(&netif.gw, 192, 168, 15, 1);
#else
    while (ethernetif_wait_linkup(&netif, 5000) != ERR_OK)
    {
        (void)PRINTF("PHY Auto-negotiation failed. Please check the cable connection and link partner setting.\r\n");
    }

    configPRINTF(("Getting IP address from DHCP ...\r\n"));
    ret = netifapi_dhcp_start(&netif);
    if (ret != (err_t)ERR_OK)
    {
        (void)PRINTF("netifapi_dhcp_start: %d\r\n", ret);
        while (true)
        {
        }
    }
    (void)ethernetif_wait_ipv4_valid(&netif, ETHERNETIF_WAIT_FOREVER);
    configPRINTF(("IPv4 Address: %s\r\n", ipaddr_ntoa(&netif.ip_addr)));
    configPRINTF(("DHCP OK\r\n"));
#endif

    return INIT_SUCCESS;
}

void print_string(const char *string)
{
    PRINTF(string);
}

int start_aknano( bool xAwsIotMqttMode,
                        const char * pIdentifier,
                        void * pNetworkServerInfo,
                        void * pNetworkCredentialInfo,
                        const IotNetworkInterface_t * pxNetworkInterface );
int initTime();
int initStorage();
#if defined(AKNANO_ENABLE_EL2GO) && defined(AKNANO_ALLOW_PROVISIONING)
void aknano_start_el2go_task();
#endif

void vApplicationDaemonTaskStartupHook(void)
{
    configPRINTF(("AKNano vApplicationDaemonTaskStartupHook.\r\n"));

    if (SYSTEM_Init() == pdPASS)
    {
        if (initNetwork() != 0)
        {
            configPRINTF(("Network init failed, stopping demo.\r\n"));
            vTaskDelete(NULL);
        }
        else
        {
            initTime();
            initStorage();

            static demoContext_t otaDemoContext = {.networkTypes                = AWSIOT_NETWORK_TYPE_ETH,
                                                   .demoFunction                = start_aknano,
                                                   .networkConnectedCallback    = NULL,
                                                   .networkDisconnectedCallback = NULL};

            Iot_CreateDetachedThread(runDemoTask, &otaDemoContext, (tskIDLE_PRIORITY + 1),
                                     AKNANO_TASK_STACK_SIZE);
        }
    }
}

void btn_read_task(void *pvParameters);
#define led_toggle_task_PRIO (configMAX_PRIORITIES - 1)
#define btn_press_task_PRIO (configMAX_PRIORITIES - 1)

void sampleAppTask( void * pvParameters );

int main(void)
{
    gpio_pin_config_t gpio_config = {kGPIO_DigitalOutput, 0, kGPIO_NoIntmode};

    BOARD_ConfigMPU();

#ifdef AKNANO_BOARD_MODEL_RT1060
    BOARD_InitBootPins();
    BOARD_InitBootClocks();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    IOMUXC_EnableMode(IOMUXC_GPR, kIOMUXC_GPR_ENET1TxClkOutputDir, true);

    GPIO_PinInit(GPIO1, 9, &gpio_config);
    GPIO_PinInit(GPIO1, 10, &gpio_config);
    /* Pull up the ENET_INT before RESET. */
    GPIO_WritePinOutput(GPIO1, 10, 1);
    GPIO_WritePinOutput(GPIO1, 9, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO1, 9, 1);

    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;

#else
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    BOARD_InitModuleClock();

    SCB_DisableDCache();

    IOMUXC_SelectENETClock();

#if BOARD_NETWORK_USE_100M_ENET_PORT
    BOARD_InitEnetPins();
    GPIO_PinInit(GPIO12, 12, &gpio_config);
    GPIO_WritePinOutput(GPIO12, 12, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO12, 12, 1);
    SDK_DelayAtLeastUs(6, CLOCK_GetFreq(kCLOCK_CpuClk));
#else
    BOARD_InitEnet1GPins();
    GPIO_PinInit(GPIO11, 14, &gpio_config);
    /* For a complete PHY reset of RTL8211FDI-CG, this pin must be asserted low for at least 10ms. And
     * wait for a further 30ms(for internal circuits settling time) before accessing the PHY register */
    GPIO_WritePinOutput(GPIO11, 14, 0);
    SDK_DelayAtLeastUs(10000, CLOCK_GetFreq(kCLOCK_CpuClk));
    GPIO_WritePinOutput(GPIO11, 14, 1);
    SDK_DelayAtLeastUs(30000, CLOCK_GetFreq(kCLOCK_CpuClk));

    EnableIRQ(ENET_1G_MAC0_Tx_Rx_1_IRQn);
    EnableIRQ(ENET_1G_MAC0_Tx_Rx_2_IRQn);
#endif
    MDIO_Init();
    g_phy_resource.read  = MDIO_Read;
    g_phy_resource.write = MDIO_Write;
#endif

    CRYPTO_InitHardware();

    xLoggingTaskInitialize(LOGGING_TASK_STACK_SIZE, LOGGING_TASK_PRIORITY, LOGGING_QUEUE_LENGTH);

        // if( xTaskCreate( prvLoggingTask, "Logging", usStackSize, NULL, uxPriority, NULL ) == pdPASS )
    xTaskCreate(btn_read_task, "btn_read_task", 2048, NULL, btn_press_task_PRIO, NULL);

    // xTaskCreate(sampleAppTask, "sntp_task", 4096, NULL, btn_press_task_PRIO, NULL);
#if defined(AKNANO_ENABLE_EL2GO) && defined(AKNANO_ALLOW_PROVISIONING)
    aknano_start_el2go_task();
#endif
    vTaskStartScheduler();
    for (;;)
        ;
}
