#include "infra_config.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
//#include "arm_math.h"
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "infra_compat.h"
#ifdef INFRA_MEM_STATS
    #include "infra_mem_stats.h"
#endif
#include "dev_model_api.h"
#include "dm_wrapper.h"
#include "cJSON.h"
#ifdef ATM_ENABLED
    #include "at_api.h"
#endif

#include "FreeRTOS.h"
#include "portmacro.h"
#include "semphr.h"
#include "timers.h"
#include "queue.h"

#include "fsl_pwm.h"

#include "board.h"
#include "fsl_gpio.h"
#include "fsl_iomuxc.h"
#include "fsl_debug_console.h"
typedef struct {
    int master_devid;
    int cloud_connected;
    int master_initialized;
	uint8_t light_onoff_st;
	uint8_t light_lightness;
} lighting_example_ctx_t;

static TimerHandle_t lighting_change_ln_timer = NULL;
static uint8_t current_lightness_level = 0;
static uint8_t lightness_step = 0;

static lighting_example_ctx_t g_lighting_ctx;

#define EXAMPLE_TRACE(...)                                          \
    do {                                                            \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);     \
        HAL_Printf(__VA_ARGS__);                                    \
        HAL_Printf("\033[0m\r\n");                                  \
    } while (0)

#define EXAMPLE_MASTER_DEVID            (0)
#define EXAMPLE_YIELD_TIMEOUT_MS        (100)

static void pwm_internal_setup(void ){
	uint16_t deadTimeVal;
	pwm_signal_param_t pwmSignal;
	uint32_t pwmSourceClockInHz;
	uint32_t pwmFrequencyInHz = 100;

	pwmSourceClockInHz = CLOCK_GetFreq(kCLOCK_IpgClk);

	/* Set deadtime count, we set this to about 650ns */
	deadTimeVal = ((uint64_t)pwmSourceClockInHz * 650) / 1000000000;

	pwmSignal.pwmChannel	   = kPWM_PwmA;
	pwmSignal.level 		   = kPWM_HighTrue;
	pwmSignal.dutyCyclePercent = g_lighting_ctx.light_lightness; /* 1 percent dutycycle */
	pwmSignal.deadtimeValue    = deadTimeVal;


	/*********** PWMA_SM0 - phase A, configuration, setup 2 channel as an example ************/
	PWM_SetupPwm(PWM4, kPWM_Module_0, &pwmSignal, 1, kPWM_SignedCenterAligned, pwmFrequencyInHz,
				 pwmSourceClockInHz);
}

static void lightness_timeout_cb(void ){
	uint8_t set_lightness = 0;
	if(current_lightness_level > g_lighting_ctx.light_lightness){
		
		set_lightness = current_lightness_level--;

	}else{
		set_lightness = current_lightness_level++;

	}
	PWM_UpdatePwmDutycycle(PWM4, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, set_lightness);
	PWM_SetPwmLdok(PWM4, kPWM_Control_Module_0, true);
	PWM4->SM[0].DISMAP[0] = 0;
	if(current_lightness_level == g_lighting_ctx.light_lightness){
		xTimerDelete(lighting_change_ln_timer, 0);
		lighting_change_ln_timer = NULL;
	}

	PRINTF("Lightness set to %d, target lightness level %d\r\n",set_lightness,g_lighting_ctx.light_lightness);


}

static void lighting_init(void ){
#if 1
	//For Lighting Demo
	IOMUXC_SetPinMux(
	  IOMUXC_GPIO_AD_B1_08_FLEXPWM4_PWMA00,   /* GPIO_AD_B1_08 is configured as FLEXPWM4_PWMA00 */
	  0U);                                    /* Software Input On Field: Input Path is determined by functionality */
//IOMUXC_SetPinConfig
	IOMUXC_SetPinConfig(
	  IOMUXC_GPIO_AD_B1_08_FLEXPWM4_PWMA00,   /* GPIO_AD_B1_08 is configured as FLEXPWM4_PWMA00 */
	  0x10B0U);                                    /* Software Input On Field: Input Path is determined by functionality */


#else
	IOMUXC_SetPinMux(
	  IOMUXC_GPIO_AD_B1_08_GPIO1_IO24,        /* GPIO_AD_B0_09 is configured as GPIO1_IO09 */
	  0U);                                    /* Software Input On Field: Input Path is determined by functionality */
	IOMUXC_SetPinConfig(
	  IOMUXC_GPIO_AD_B1_08_GPIO1_IO24,        /* GPIO_AD_B0_09 PAD functional properties : */
	  0x10B0U);                               /* Slew Rate Field: Slow Slew Rate
	                                             Drive Strength Field: R0/6
	                                             Speed Field: medium(100MHz)
	                                             Open Drain Enable Field: Open Drain Disabled
	                                             Pull / Keep Enable Field: Pull/Keeper Enabled
	                                             Pull / Keep Select Field: Keeper
	                                             Pull Up / Down Config. Field: 100K Ohm Pull Down
	                                             Hyst. Enable Field: Hysteresis Disabled */
	gpio_pin_config_t led_config = {kGPIO_DigitalOutput, 1, kGPIO_NoIntmode};
	/* Init output LED GPIO. */
	GPIO_PinInit(GPIO1, 24U, &led_config);                                          
#endif




/* Structure of initialize PWM */
	pwm_config_t pwmConfig;
	static uint16_t delay;
	uint32_t pwmVal = 50;
	uint16_t i;

   

	CLOCK_SetDiv(kCLOCK_AhbDiv, 0x2); /* Set AHB PODF to 2, divide by 3 */
	CLOCK_SetDiv(kCLOCK_IpgDiv, 0x3); /* Set IPG PODF to 3, divede by 4 */

	/* Set the PWM Fault inputs to a low value */
	XBARA_Init(XBARA1);
	XBARA_SetSignalsConnection(XBARA1, kXBARA1_InputLogicLow, kXBARA1_OutputFlexpwm4Fault0);
  
	PRINTF("FlexPWM driver example\n");

	/*
	 * pwmConfig.enableDebugMode = false;
	 * pwmConfig.enableWait = false;
	 * pwmConfig.reloadSelect = kPWM_LocalReload;
	 * pwmConfig.faultFilterCount = 0;
	 * pwmConfig.faultFilterPeriod = 0;
	 * pwmConfig.clockSource = kPWM_BusClock;
	 * pwmConfig.prescale = kPWM_Prescale_Divide_1;
	 * pwmConfig.initializationControl = kPWM_Initialize_LocalSync;
	 * pwmConfig.forceTrigger = kPWM_Force_Local;
	 * pwmConfig.reloadFrequency = kPWM_LoadEveryOportunity;
	 * pwmConfig.reloadLogic = kPWM_ReloadImmediate;
	 * pwmConfig.pairOperation = kPWM_Independent;
	 */
	PWM_GetDefaultConfig(&pwmConfig);

	/* Use full cycle reload */
    pwmConfig.reloadLogic = kPWM_ReloadPwmFullCycle;
	/* PWM A & PWM B form a complementary PWM pair */
	//pwmConfig.pairOperation	= kPWM_Independent;
    pwmConfig.enableDebugMode = true;

	/* Initialize submodule 0 */
	if (PWM_Init(PWM4, kPWM_Module_0, &pwmConfig) == kStatus_Fail)
	{
		PRINTF("PWM initialization failed\n");
		return;
	}
        

	
    g_lighting_ctx.light_onoff_st = 0;
    g_lighting_ctx.light_lightness = 100;
	
#if 0

	//PWM4->SM[0].DISMAP[1] = 0;
	/* Set the load okay bit for all submodules to load registers from their buffer */
	PWM_SetPwmLdok(PWM4, kPWM_Control_Module_0, true);

	/* Start the PWM generation from Submodules 0, 1 and 2 */
	PWM_StartTimer(PWM4, kPWM_Control_Module_0);
        PWM4->SM[0].DISMAP[0] = 0;

   delay = 0x0fffU;
	while (1U)
	{
		for (i = 0U; i < delay; i++)
		{
			__ASM volatile("nop");
		}
		pwmVal = pwmVal + 4;

		/* Reset the duty cycle percentage */
		if (pwmVal > 100)
		{
		  //while(1);
			pwmVal = 4;
		}

		/* Update duty cycles for all 3 PWM signals */
		PWM_UpdatePwmDutycycle(PWM4, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, pwmVal);
		/* Set the load okay bit for all submodules to load registers from their buffer */
		PWM_SetPwmLdok(PWM4, kPWM_Control_Module_0, true);
	}

#endif

}



void light_onoff_set_hdl(uint8_t onoff){
	
	if(!onoff){
		//PWM_UpdatePwmDutycycle(PWM4, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, 0);
		//PWM_SetPwmLdok(PWM4, kPWM_Control_Module_0, true);
		//PWM4->SM[0].DISMAP[0] = 0;
          PWM4->OUTEN = 0;
		PWM_StopTimer(PWM4, kPWM_Control_Module_0);
		
		if(lighting_change_ln_timer){

			xTimerDelete(lighting_change_ln_timer, 0);
			lighting_change_ln_timer = NULL;
			PRINTF("Delte local lightness change timer\r\n");
		}
	}else{
        pwm_internal_setup();        
       	PWM_SetPwmLdok(PWM4, kPWM_Control_Module_0, true);
		PWM_StartTimer(PWM4, kPWM_Control_Module_0);
		PWM4->SM[0].DISMAP[0] = 0;
	}
	g_lighting_ctx.light_onoff_st = onoff;
}

void light_rgb_set_hdl(uint8_t r, uint8_t g, uint8_t b){



}

void light_lightness_set_hdl(uint8_t lightness){
	if(lightness > 100 || !g_lighting_ctx.light_onoff_st){
		PRINTF("Invalid parameter, lightnness set %d, onoff status %d\r\n",lightness,g_lighting_ctx.light_onoff_st);
		return;
	}
	if(lightness == 0){
		lightness = 1;
	}

	current_lightness_level = g_lighting_ctx.light_lightness;
	g_lighting_ctx.light_lightness = lightness;
	uint8_t gap = 0;

	if(current_lightness_level > g_lighting_ctx.light_lightness){
		gap = current_lightness_level - g_lighting_ctx.light_lightness;
	}else{
		gap = g_lighting_ctx.light_lightness - current_lightness_level;
	}
	if(!gap){
		return;
	}
	if(lighting_change_ln_timer){
		xTimerStop(lighting_change_ln_timer, 0);
	}else{
		lighting_change_ln_timer = xTimerCreate("lightness_change", pdMS_TO_TICKS(10), pdTRUE, NULL, (TimerCallbackFunction_t)lightness_timeout_cb);
	}
	int setv = (int )(200/gap);
	xTimerChangePeriod(lighting_change_ln_timer, pdMS_TO_TICKS(setv), 0);
	xTimerStart(lighting_change_ln_timer, 0);

	uint8_t set_lightness = 0;
	if(current_lightness_level > g_lighting_ctx.light_lightness){
		
		set_lightness = current_lightness_level--;

	}else{
		set_lightness = current_lightness_level++;


	}
	PRINTF("Lightness set to %d, target lightness level %d\r\n",set_lightness,g_lighting_ctx.light_lightness);
	PWM_UpdatePwmDutycycle(PWM4, kPWM_Module_0, kPWM_PwmA, kPWM_SignedCenterAligned, set_lightness);
	PWM_SetPwmLdok(PWM4, kPWM_Control_Module_0, true);
	PWM4->SM[0].DISMAP[0] = 0;
	
}





/** cloud connected event callback */
static int lighting_connected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Connected");
    g_lighting_ctx.cloud_connected = 1;

    return 0;
}

/** cloud disconnected event callback */
static int lighting_disconnected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Disconnected");
    g_lighting_ctx.cloud_connected = 0;

    return 0;
}

/* device initialized event callback */
static int lighting_initialized(const int devid)
{
    EXAMPLE_TRACE("Device Initialized");
    g_lighting_ctx.master_initialized = 1;

    return 0;
}

/** recv property post response message from cloud **/
static int lighting_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
        const int reply_len)
{
    char *p_buffer;
    if (reply != NULL) {
        p_buffer = HAL_Malloc(reply_len + 1);
		if(p_buffer){
	        memcpy(p_buffer, reply, reply_len);
	        p_buffer[reply_len] = '\0';
		}
    }
    
    EXAMPLE_TRACE("Message Post Reply Received, Message ID: %d, Code: %d, Reply: %s", msgid, code,
                  (reply == NULL)? ("NULL") : (p_buffer));
    
    if (reply != NULL) {
        HAL_Free(p_buffer);
    }
    return 0;
}

/** recv event post response message from cloud **/
static int lighting_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
        const int eventid_len, const char *message, const int message_len)
{
    EXAMPLE_TRACE("Trigger Event Reply Received, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s",
                  msgid, code,
                  eventid_len,
                  eventid, message_len, message);

    return 0;
}

/** recv event post response message from cloud **/
static int lighting_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    EXAMPLE_TRACE("Property Set Received, Request: %s", request);
    
    cJSON *p_root, *light_c, *brigtness, *RGBColor;
    
    p_root = cJSON_Parse(request);
    if (p_root == NULL || !cJSON_IsObject(p_root)) {
        HAL_Printf("JSON Parse Error\r\n");
        return -1;
    }
	 
	light_c = cJSON_GetObjectItemCaseSensitive(p_root, "LightSwitch");
	
	if ((light_c != NULL) && (cJSON_IsNumber(light_c))) {
        
        light_onoff_set_hdl(light_c->valueint);
    }
	brigtness = cJSON_GetObjectItemCaseSensitive(p_root, "Brightness");
	if((brigtness != NULL) && (cJSON_IsNumber(brigtness))) {
        light_lightness_set_hdl(brigtness->valueint);
    }
    RGBColor = cJSON_GetObjectItemCaseSensitive(p_root, "RGBColor");
	if(RGBColor != NULL) {
        light_rgb_set_hdl(RGBColor->valueint,RGBColor->valueint,RGBColor->valueint);
    }
    cJSON_Delete(p_root);

    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_PROPERTY,
                             (unsigned char *)request, request_len);
    EXAMPLE_TRACE("Post Property Message ID: %d", res);

    return 0;
}


static int lighting_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
                                              const char *request, const int request_len,
                                              char **response, int *response_len)
{
    int add_result = 0;
    cJSON *root = NULL, *item_number_a = NULL, *item_number_b = NULL;
    const char *response_fmt = "{\"Result\": %d}";

    EXAMPLE_TRACE("Service Request Received, Service ID: %.*s, Payload: %s", serviceid_len, serviceid, request);

    /* Parse Root */
    root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root)) {
        EXAMPLE_TRACE("JSON Parse Error");
        return -1;
    }

    if (strlen("Operation_Service") == serviceid_len && memcmp("Operation_Service", serviceid, serviceid_len) == 0) {
        /* Parse NumberA */
        item_number_a = cJSON_GetObjectItem(root, "NumberA");
        if (item_number_a == NULL || !cJSON_IsNumber(item_number_a)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("NumberA = %d", item_number_a->valueint);

        /* Parse NumberB */
        item_number_b = cJSON_GetObjectItem(root, "NumberB");
        if (item_number_b == NULL || !cJSON_IsNumber(item_number_b)) {
            cJSON_Delete(root);
            return -1;
        }
        EXAMPLE_TRACE("NumberB = %d", item_number_b->valueint);

        add_result = item_number_a->valueint + item_number_b->valueint;

        /* Send Service Response To Cloud */
        *response_len = strlen(response_fmt) + 10 + 1;
        *response = (char *)HAL_Malloc(*response_len);
        if (*response == NULL) {
            EXAMPLE_TRACE("Memory Not Enough");
            return -1;
        }
        memset(*response, 0, *response_len);
        HAL_Snprintf(*response, *response_len, response_fmt, add_result);
        *response_len = strlen(*response);
    }

    cJSON_Delete(root);
    return 0;
}

static int lighting_timestamp_reply_event_handler(const char *timestamp)
{
    EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

/** fota event handler **/
static int lighting_fota_event_handler(int type, const char *version)
{
    char buffer[128] = {0};
    int buffer_length = 128;

    /* 0 - new firmware exist, query the new firmware */
    if (type == 0) {
        EXAMPLE_TRACE("New Firmware Version: %s", version);

        IOT_Linkkit_Query(EXAMPLE_MASTER_DEVID, ITM_MSG_QUERY_FOTA_DATA, (unsigned char *)buffer, buffer_length);
    }
	

    return 0;
}

/* cota event handler */
static int lighting_cota_event_handler(int type, const char *config_id, int config_size, const char *get_type,
                                   const char *sign, const char *sign_method, const char *url)
{
    char buffer[128] = {0};
    int buffer_length = 128;

    /* type = 0, new config exist, query the new config */
    if (type == 0) {
        EXAMPLE_TRACE("New Config ID: %s", config_id);
        EXAMPLE_TRACE("New Config Size: %d", config_size);
        EXAMPLE_TRACE("New Config Type: %s", get_type);
        EXAMPLE_TRACE("New Config Sign: %s", sign);
        EXAMPLE_TRACE("New Config Sign Method: %s", sign_method);
        EXAMPLE_TRACE("New Config URL: %s", url);

        IOT_Linkkit_Query(EXAMPLE_MASTER_DEVID, ITM_MSG_QUERY_COTA_DATA, (unsigned char *)buffer, buffer_length);
    }

    return 0;
}

void lighting_property_post(uint8_t is_light_switch){
	if(!g_lighting_ctx.cloud_connected){

		HAL_Printf("cloud not connected\r\n");
		return;

	}

	char property_payload[30] = {0};
	int offset = 0;
	if(is_light_switch){
		HAL_Snprintf(property_payload, sizeof(property_payload), "{\"LightSwitch\": %d}", g_lighting_ctx.light_onoff_st);
	}else{

		HAL_Snprintf(property_payload, sizeof(property_payload), "{\"Brightness\": %d}", g_lighting_ctx.light_lightness);

	}
	
	IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_PROPERTY,
									 (unsigned char *)property_payload, strlen(property_payload));

}

void lighting_post_event(void)
{
    int res = 0;
    char *event_id = "HardwareError";
    char *event_payload = "{\"ErrorCode\": 0}";

    res = IOT_Linkkit_TriggerEvent(EXAMPLE_MASTER_DEVID, event_id, strlen(event_id),
                                   event_payload, strlen(event_payload));
    EXAMPLE_TRACE("Post Event Message ID: %d", res);
}

void lighting_deviceinfo_update(void)
{
    int res = 0;
    char *device_info_update = "[{\"attrKey\":\"abc\",\"attrValue\":\"hello,world\"}]";

    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DEVICEINFO_UPDATE,
                             (unsigned char *)device_info_update, strlen(device_info_update));
    EXAMPLE_TRACE("Device Info Update Message ID: %d", res);
}

void lighting_deviceinfo_delete(void)
{
    int res = 0;
    char *device_info_delete = "[{\"attrKey\":\"abc\"}]";

    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DEVICEINFO_DELETE,
                             (unsigned char *)device_info_delete, strlen(device_info_delete));
    EXAMPLE_TRACE("Device Info Delete Message ID: %d", res);
}

void lighting_button_pressed(void ){
	uint8_t r = HAL_Random(0xff);
	uint8_t g = HAL_Random(0xff);
	uint8_t b = HAL_Random(0xff);
	pca_red_value_set(r);
	pca_green_value_set(r);
	pca_blue_value_set(r);

}

int lighting_run(int argc, char **argv)
{
    int res = 0;
    int cnt = 0;
    iotx_linkkit_dev_meta_info_t lighting_meta_info;
    int domain_type = 0, dynamic_register = 0, post_reply_need = 0;

#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0) {
        EXAMPLE_TRACE("IOT ATM init failed!\n");
        return -1;
    }
#endif
    pca9634_init();
#if 0
	int r = 100;
	int g = 30;
	int b = 50;
	while(1){
		
		pca_blue_value_set(b++&0xff);
		pca_green_value_set(g++&0xff);
		pca_red_value_set(r++&0xff);
		HAL_SleepMs(200);
	}
#endif
	app_wait_wifi_connect();
	

    memset(&g_lighting_ctx, 0, sizeof(lighting_example_ctx_t));
	
    memset(&lighting_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));
	
    lighting_init();
	
	
	HAL_GetProductKey(lighting_meta_info.product_key);
	HAL_GetProductSecret(lighting_meta_info.product_secret);
	HAL_GetDeviceName(lighting_meta_info.device_name);
	HAL_GetDeviceSecret(lighting_meta_info.device_secret);
   

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /* Register Callback */
    IOT_RegisterCallback(ITE_CONNECT_SUCC, lighting_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, lighting_disconnected_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, lighting_service_request_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, lighting_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, lighting_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, lighting_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, lighting_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, lighting_initialized);
    IOT_RegisterCallback(ITE_FOTA, lighting_fota_event_handler);
    IOT_RegisterCallback(ITE_COTA, lighting_cota_event_handler);

    domain_type = IOTX_CLOUD_REGION_SHANGHAI;
    IOT_Ioctl(IOTX_IOCTL_SET_DOMAIN, (void *)&
              domain_type);

    /* Choose Login Method */
    dynamic_register = 0;
    IOT_Ioctl(IOTX_IOCTL_SET_DYNAMIC_REGISTER, (void *)&dynamic_register);

    /* post reply doesn't need */
    post_reply_need = 1;
    IOT_Ioctl(IOTX_IOCTL_RECV_EVENT_REPLY, (void *)&post_reply_need);

    /* Create Master Device Resources */
    g_lighting_ctx.master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &lighting_meta_info);
    if (g_lighting_ctx.master_devid < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Connect(g_lighting_ctx.master_devid);
    if (res < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Connect Failed\n");
        return -1;
    }
	lighting_property_post(1);
	lighting_property_post(0);
    while (1) {
        
        IOT_Linkkit_Yield(EXAMPLE_YIELD_TIMEOUT_MS);

        /* Post Event Example */
        if ((cnt % 50) == 0) {
            lighting_post_event();
			//lighting_button_pressed();
        }
		cnt++;
    }
}



