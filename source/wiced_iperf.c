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

#include "kv_api.h"
#include "flexspi_hyper_flash_ops.h"

#include "ewmain.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define AP_SSID "iPhone"
#define AP_PASS "12345678"
#define AP_SEC WICED_SECURITY_WPA2_AES_PSK

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
#if 0
            PRINTF("Joining : " AP_SSID "\n");
            (void)host_rtos_delay_milliseconds((uint32_t)1000);
            
            do {
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
                    break;
                }
            } while (err != WWD_SUCCESS);
#endif
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
    wm_run(NULL, NULL);
   // linkkit_example_solo(NULL, NULL);
}

/*******************************************************************************
* FUNCTION:
*   GuiThread
*
* DESCRIPTION:
*   The EwThread processes the Embeded Wizard application.
*
* ARGUMENTS:
*   arg - not used.
*
* RETURN VALUE:
*   None.
*
*******************************************************************************/
static void GuiThread( void* arg )
{
  /* initialize Embedded Wizard application */
  if ( EwInit() == 0 )
    return;

  EwPrintSystemInfo();

  /* process the Embedded Wizard main loop */
  while( EwProcess())
    ;

  /* de-initialize Embedded Wizard application */
  EwDone();
}

void app_wait_wifi_connect(void ){

	char wifi_ssid[40]={0};
	char wifi_key[40] = {0};
	int ssid_len = 40;
	int key_len = 40;
	wwd_result_t err = WWD_SUCCESS;
	int cnt = 0x0e;
	while((wwd_wifi_get_ap_is_up() != WICED_TRUE)){
		
		if(HAL_Kv_Get("wifi_ssid", wifi_ssid, &ssid_len) == 0){
		  if(ssid_len != 1 || wifi_ssid[0] != 0xff){
            

		    if(HAL_Kv_Get("wifi_key", wifi_key, &key_len) == 0){
		       if(key_len != 1){
					wiced_ssid_t ap_ssid = {0};

					ap_ssid.length = strlen(wifi_ssid);
					memcpy(ap_ssid.value,wifi_ssid,ap_ssid.length);
					err = wwd_wifi_join(&ap_ssid, AP_SEC, (uint8_t *)wifi_key, strlen(wifi_key), NULL, WWD_STA_INTERFACE);
					if (err != WWD_SUCCESS)
					{
						PRINTF("Failed to join  : %s",wifi_ssid);
					}
					else
					{
						PRINTF("Successfully joined : %s\r\n",wifi_ssid);
						(void)host_rtos_delay_milliseconds((uint32_t)1000);
						add_wlan_interface();
						break;
					}
		       }
		    }
		  }
		}
        
        HAL_SleepMs(1000);
        if((cnt++ & 0x0f) == 0x0f){
          PRINTF("Join AP failed by using KV info\r\n");
        }
    } 
}


static uint8_t app_wifi_ib_same(char *ssid, char *key){
	char wifi_ssid[40]={0};
	char wifi_key[40] = {0};
	int ssid_len = 40;
	int key_len = 40;
	if((HAL_Kv_Get("wifi_ssid", wifi_ssid, &ssid_len) == 0) && (strncmp(ssid,wifi_ssid,strlen(wifi_ssid)) == 0)){
	    if((HAL_Kv_Get("wifi_key", wifi_key, &key_len) == 0) &&(strncmp(key,wifi_key,strlen(wifi_key)) == 0)){
	 
			HAL_Printf("Same WiFi IB inputed\r\n");
			return 1;

	    }
	}
	return 0;


}

void app_process_wifi_config(char *ssid, char *key){
	if((ssid == NULL) && (key == NULL)){
		uint8_t value_invalid = 0xff;
		HAL_Kv_Set("wifi_ssid", &value_invalid, 1, 0);
		HAL_Kv_Set("wifi_key", &value_invalid, 1, 0);
		wwd_wifi_leave(WWD_STA_INTERFACE);

	}else if(app_wifi_ib_same(ssid,key) == 0){
		HAL_Kv_Set("wifi_ssid", ssid, strlen(ssid), 0);
		HAL_Kv_Set("wifi_key", key, strlen(key), 0);
	}

}

void app_process_recive_cmd(char *buff, uint8_t len){
  uint8_t ptr = 2;
  uint8_t i = 0;
  if(buff[0] == 'c'){//connect wifi
		char wifi_ssid[40]={0};
		char wifi_key[40] = {0};
		if(buff[1] == ' '){
			while(buff[ptr] != ' '){
				wifi_ssid[i++] = buff[ptr++];
			}
			ptr++;
			i=0;
			while(buff[ptr] != '\r' && (ptr<len)){
				wifi_key[i++] = buff[ptr++];
			}
			if(app_wifi_ib_same(wifi_ssid,wifi_key) == 0){
				HAL_Kv_Set("wifi_ssid", wifi_ssid, strlen(wifi_ssid), 0);
				HAL_Kv_Set("wifi_key", wifi_key, strlen(wifi_key), 0);
			}
                        wiced_ssid_t ap_ssid = {0};

                        ap_ssid.length = strlen(wifi_ssid);
                        memcpy(ap_ssid.value,wifi_ssid,ap_ssid.length);
                        //wwd_wifi_join(&ap_ssid, AP_SEC, (uint8_t *)wifi_key, strlen(wifi_key), NULL, WWD_STA_INTERFACE);
                        HAL_Printf("join wifi:%s....\r\n",wifi_ssid);
		}

  }else if(buff[0] == 'f'){//factory new module
  	uint8_t value_invalid = 0xff;
	HAL_Kv_Set("wifi_ssid", &value_invalid, 1, 0);
	HAL_Kv_Set("wifi_key", &value_invalid, 1, 0);
	wwd_wifi_leave(WWD_STA_INTERFACE);
        HAL_Printf("Factory wifi module....\r\n");
  }else if(buff[0] == 'w'){
  	wm_cli_process(buff+1, len - 1);
  	

  }else{

  	HAL_Printf("Unknown command\r\n");

  }
  
}

/*!
 * @brief Main function.
 */
int main(void)
{
    BOARD_ConfigMPU();
    BOARD_InitPins();
    BOARD_InitI2C1Pins();
    BOARD_InitSemcPins();
    BOARD_BootClockRUN();
    BOARD_USDHCClockConfiguration();
    BOARD_InitDebugConsole();
	ShellInit();
    //log_init();
	#if 1
    flexspi_hyper_flash_init();
    kv_init();
    
    tcpip_init(NULL, NULL);
#if 1
    /* create thread that drives the Embedded Wizard GUI application... */
    EwPrint( "Create UI thread...                          " );
    xTaskCreate( GuiThread, "EmWi_Task", 1280, NULL, 2, NULL );
    EwPrint( "[OK]\n" );
#endif
    if (xTaskCreate(linkkit_task, "linkkit_task", 1000, NULL, 3, NULL) != pdPASS)
    {
        PRINTF("Task creation failed!.\r\n");
        while (1)
            ;
    }
#endif
    vTaskStartScheduler();

    return 0;
}
