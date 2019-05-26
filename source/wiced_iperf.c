/*
* Copyright (c) 2016, Freescale Semiconductor, Inc.
* Copyright 2016-2019 NXP
* All rights reserved.
*
*
* SPDX-License-Identifier: BSD-3-Clause
*/

/*******************************************************************************
 * Includes
 ******************************************************************************/
#include "lwip/tcpip.h"
#include "lwip/apps/lwiperf.h"
#include "board.h"
#include "wwd.h"
#include "wwd_wiced.h"

#include "fsl_debug_console.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "fsl_common.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AP_SSID "Neo's WIFI"
#define AP_PASS "8220542Xy"
#define AP_SEC WICED_SECURITY_WPA2_MIXED_PSK

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

extern void test_join(void);
extern wwd_result_t test_scan();
extern wiced_result_t wiced_wlan_connectivity_init(void);
extern void add_wlan_interface(void);

extern int linkkit_example_solo(int argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/
static void BOARD_USDHCClockConfiguration(void)
{
    /*configure system pll PFD2 fractional divider to 24*/
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24U);
    /* Configure USDHC clock source and divider */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 0U);
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 0U);
}


static void BOARD_InitNetwork()
{
    wwd_result_t err = WWD_SUCCESS;
    bool wifi_platform_initialized = false;
    bool wifi_station_mode = true;

    wiced_ssid_t ap_ssid = {
        .length = sizeof(AP_SSID) - 1, .value = AP_SSID,
    };

    // Main function to initialize wifi platform
    PRINTF("Initializing WiFi Connection... \r\n");
    err = wiced_wlan_connectivity_init();
    if (err != WWD_SUCCESS)
    {
        PRINTF("Could not initialize wifi connection \n");
    }
    else
    {
        wifi_platform_initialized = true;
    }

    if (wifi_platform_initialized)
    {
        PRINTF("Successfully Initialized WiFi Connection \r\n");

        if (wifi_station_mode)
        {
            err = test_scan();
            if (err != WWD_SUCCESS)
            {
                PRINTF(" Scan Error \n");
            }

            PRINTF("Joining : " AP_SSID "\n");
            (void)host_rtos_delay_milliseconds((uint32_t)1000);
            err = wwd_wifi_join(&ap_ssid, AP_SEC, (uint8_t *)AP_PASS, sizeof(AP_PASS) - 1, NULL, WWD_STA_INTERFACE);
            if (err != WWD_SUCCESS)
            {
                PRINTF("Failed to join  : " AP_SSID " \n");
            }
            else
            {
                PRINTF("Successfully joined : " AP_SSID "\n");
                (void)host_rtos_delay_milliseconds((uint32_t)1000);
                add_wlan_interface();
            }
        }
    }
    else
    {
        vTaskSuspend(NULL);
    }
}



/*!
 * @brief The main function containing client thread.
 */
static void linkkit_task(void *arg)
{
    PRINTF("\r\n************************************************\r\n");
    PRINTF(" Link Kit example\r\n");
    PRINTF("************************************************\r\n");

    BOARD_InitNetwork();
    
    linkkit_example_solo(NULL, NULL);
}

/*!
 * @brief Main function.
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_USDHCClockConfiguration();
    BOARD_InitDebugConsole();

    tcpip_init(NULL, NULL);

    if (xTaskCreate(linkkit_task, "linkkit_task", 1000, NULL, configMAX_PRIORITIES - 4 /*3*/, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }

    vTaskStartScheduler();

    return 0;
}
