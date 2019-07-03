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
	uint8_t red_value;
	uint8_t green_value;
	uint8_t blue_value;
} rgb_light_example_ctx_t;



static rgb_light_example_ctx_t g_rgb_light_ctx;

#define EXAMPLE_TRACE(...)                                          \
    do {                                                            \
        HAL_Printf("\033[1;32;40m%s.%d: ", __func__, __LINE__);     \
        HAL_Printf(__VA_ARGS__);                                    \
        HAL_Printf("\033[0m\r\n");                                  \
    } while (0)

#define EXAMPLE_MASTER_DEVID            (0)
#define EXAMPLE_YIELD_TIMEOUT_MS        (100)

#define RGB_LIGHT_DEFAULT_ON_R		255
#define RGB_LIGHT_DEFAULT_ON_G		255
#define RGB_LIGHT_DEFAULT_ON_B		255


unsigned char rgb_light_on(void ){
	unsigned char ret = 1;
	//uint8_t read_value = 0;
	//uint8_t pca_ret = pca_read_reg(0,&read_value);
	//PRINTF("Read reg 0 result %d, read value %d\r\n",pca_ret,read_value);
	//pca_ret = pca_read_reg(1,&read_value);
	//PRINTF("Read reg 1 result %d, read value %d\r\n",pca_ret,read_value);
	ret = pca_red_value_set(g_rgb_light_ctx.red_value);
	if(ret != 0){

		PRINTF("Red value set no ack\r\n");
		return ret;
	}
	ret = pca_blue_value_set(g_rgb_light_ctx.blue_value);
	if(ret != 0){

		PRINTF("Blue value set no ack\r\n");
		return ret;
	}
	ret = pca_green_value_set(g_rgb_light_ctx.green_value);
	return ret;



}


unsigned char rgb_light_off(void ){
	unsigned char ret = 1;
	ret = pca_red_value_set(0);
	if(ret != 0){

		PRINTF("Red value set no ack\r\n");
		return ret;
	}
	ret = pca_blue_value_set(0);
	if(ret != 0){

		PRINTF("Blue value set no ack\r\n");
		return ret;
	}
	ret = pca_green_value_set(0);
	return ret;



}


void rgb_light_onoff_set_hdl(uint8_t onoff){
	if(onoff){
		rgb_light_on();
	}else{
		rgb_light_off();
	}
	g_rgb_light_ctx.light_onoff_st = onoff;
}

void rgb_light_set_hdl(uint8_t r, uint8_t g, uint8_t b){
	g_rgb_light_ctx.red_value = r;
	g_rgb_light_ctx.green_value = g;
	g_rgb_light_ctx.blue_value = b;

	pca_red_value_set(r);
	pca_green_value_set(g);
	pca_blue_value_set(b);
}





/** cloud connected event callback */
static int rgb_light_connected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Connected");
    g_rgb_light_ctx.cloud_connected = 1;

    return 0;
}

/** cloud disconnected event callback */
static int rgb_light_disconnected_event_handler(void)
{
    EXAMPLE_TRACE("Cloud Disconnected");
    g_rgb_light_ctx.cloud_connected = 0;

    return 0;
}

/* device initialized event callback */
static int rgb_light_initialized(const int devid)
{
    EXAMPLE_TRACE("Device Initialized");
    g_rgb_light_ctx.master_initialized = 1;

    return 0;
}

/** recv property post response message from cloud **/
static int rgb_light_report_reply_event_handler(const int devid, const int msgid, const int code, const char *reply,
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
static int rgb_light_trigger_event_reply_event_handler(const int devid, const int msgid, const int code, const char *eventid,
        const int eventid_len, const char *message, const int message_len)
{
    EXAMPLE_TRACE("Trigger Event Reply Received, Message ID: %d, Code: %d, EventID: %.*s, Message: %.*s",
                  msgid, code,
                  eventid_len,
                  eventid, message_len, message);

    return 0;
}

/** recv event post response message from cloud **/
static int rgb_light_property_set_event_handler(const int devid, const char *request, const int request_len)
{
    int res = 0;
    EXAMPLE_TRACE("Property Set Received, Request: %s", request);
    
    cJSON *p_root, *light_c, *RGBColor;
    
    p_root = cJSON_Parse(request);
    if (p_root == NULL || !cJSON_IsObject(p_root)) {
        HAL_Printf("JSON Parse Error\r\n");
        return -1;
    }
	 
	light_c = cJSON_GetObjectItemCaseSensitive(p_root, "LightSwitch");
	
	if ((light_c != NULL) && (cJSON_IsNumber(light_c))) {
        
        rgb_light_onoff_set_hdl(light_c->valueint);
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


static int rgb_light_service_request_event_handler(const int devid, const char *serviceid, const int serviceid_len,
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

static int rgb_light_timestamp_reply_event_handler(const char *timestamp)
{
    EXAMPLE_TRACE("Current Timestamp: %s", timestamp);

    return 0;
}

/** fota event handler **/
static int rgb_light_fota_event_handler(int type, const char *version)
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
static int rgb_light_cota_event_handler(int type, const char *config_id, int config_size, const char *get_type,
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

void rgb_light_property_post(uint8_t is_light_switch){
	if(!g_rgb_light_ctx.cloud_connected){

		HAL_Printf("cloud not connected\r\n");
		return;

	}

	char property_payload[30] = {0};
	int offset = 0;
	
	HAL_Snprintf(property_payload, sizeof(property_payload), "{\"LightSwitch\": %d}", g_rgb_light_ctx.light_onoff_st);
	
	
	IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_POST_PROPERTY,
									 (unsigned char *)property_payload, strlen(property_payload));

}

void rgb_light_post_event(void)
{
    int res = 0;
    char *event_id = "HardwareError";
    char *event_payload = "{\"ErrorCode\": 0}";

    res = IOT_Linkkit_TriggerEvent(EXAMPLE_MASTER_DEVID, event_id, strlen(event_id),
                                   event_payload, strlen(event_payload));
    EXAMPLE_TRACE("Post Event Message ID: %d", res);
}

void rgb_light_deviceinfo_update(void)
{
    int res = 0;
    char *device_info_update = "[{\"attrKey\":\"abc\",\"attrValue\":\"hello,world\"}]";

    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DEVICEINFO_UPDATE,
                             (unsigned char *)device_info_update, strlen(device_info_update));
    EXAMPLE_TRACE("Device Info Update Message ID: %d", res);
}

void rgb_light_deviceinfo_delete(void)
{
    int res = 0;
    char *device_info_delete = "[{\"attrKey\":\"abc\"}]";

    res = IOT_Linkkit_Report(EXAMPLE_MASTER_DEVID, ITM_MSG_DEVICEINFO_DELETE,
                             (unsigned char *)device_info_delete, strlen(device_info_delete));
    EXAMPLE_TRACE("Device Info Delete Message ID: %d", res);
}

void rgb_light_button_pressed(void ){
	uint8_t r = HAL_Random(0xff);
	uint8_t g = HAL_Random(0xff);
	uint8_t b = HAL_Random(0xff);
	pca_red_value_set(r);
	pca_green_value_set(r);
	pca_blue_value_set(r);

}

int rgb_light_run(int argc, char **argv)
{
    int res = 0;
    int cnt = 0;
    iotx_linkkit_dev_meta_info_t rgb_light_meta_info;
    int domain_type = 0, dynamic_register = 0, post_reply_need = 0;

#ifdef ATM_ENABLED
    if (IOT_ATM_Init() < 0) {
        EXAMPLE_TRACE("IOT ATM init failed!\n");
        return -1;
    }
#endif
	memset(&g_rgb_light_ctx, 0, sizeof(rgb_light_example_ctx_t));
		
	memset(&rgb_light_meta_info, 0, sizeof(iotx_linkkit_dev_meta_info_t));

    pca9634_init();
	g_rgb_light_ctx.red_value = RGB_LIGHT_DEFAULT_ON_R;
	g_rgb_light_ctx.green_value = RGB_LIGHT_DEFAULT_ON_G;
	g_rgb_light_ctx.blue_value = RGB_LIGHT_DEFAULT_ON_B;
	rgb_light_onoff_set_hdl(g_rgb_light_ctx.light_onoff_st);

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
	

    

	
	
	HAL_GetProductKey(rgb_light_meta_info.product_key);
	HAL_GetProductSecret(rgb_light_meta_info.product_secret);
	HAL_GetDeviceName(rgb_light_meta_info.device_name);
	HAL_GetDeviceSecret(rgb_light_meta_info.device_secret);
   

    IOT_SetLogLevel(IOT_LOG_DEBUG);

    /* Register Callback */
    IOT_RegisterCallback(ITE_CONNECT_SUCC, rgb_light_connected_event_handler);
    IOT_RegisterCallback(ITE_DISCONNECTED, rgb_light_disconnected_event_handler);
    IOT_RegisterCallback(ITE_SERVICE_REQUEST, rgb_light_service_request_event_handler);
    IOT_RegisterCallback(ITE_PROPERTY_SET, rgb_light_property_set_event_handler);
    IOT_RegisterCallback(ITE_REPORT_REPLY, rgb_light_report_reply_event_handler);
    IOT_RegisterCallback(ITE_TRIGGER_EVENT_REPLY, rgb_light_trigger_event_reply_event_handler);
    IOT_RegisterCallback(ITE_TIMESTAMP_REPLY, rgb_light_timestamp_reply_event_handler);
    IOT_RegisterCallback(ITE_INITIALIZE_COMPLETED, rgb_light_initialized);
    IOT_RegisterCallback(ITE_FOTA, rgb_light_fota_event_handler);
    IOT_RegisterCallback(ITE_COTA, rgb_light_cota_event_handler);

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
    g_rgb_light_ctx.master_devid = IOT_Linkkit_Open(IOTX_LINKKIT_DEV_TYPE_MASTER, &rgb_light_meta_info);
    if (g_rgb_light_ctx.master_devid < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Open Failed\n");
        return -1;
    }

    /* Start Connect Aliyun Server */
    res = IOT_Linkkit_Connect(g_rgb_light_ctx.master_devid);
    if (res < 0) {
        EXAMPLE_TRACE("IOT_Linkkit_Connect Failed\n");
        return -1;
    }
	
    while (1) {
        
        IOT_Linkkit_Yield(EXAMPLE_YIELD_TIMEOUT_MS);

        /* Post Event Example */
        if ((cnt % 100) == 0) {
            rgb_light_property_post(1);
	//		rgb_light_property_post(0);
        }
		cnt++;
    }
}




