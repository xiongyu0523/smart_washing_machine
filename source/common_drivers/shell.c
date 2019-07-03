/* Copyright 2018 NXP */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "board.h"
#include "fsl_shell.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

#include "ip_cmd.h"

#include "virtual_com.h"

#include "shell.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define CFG_SHELL_PRINT_MAX_LEN                  512

#define COLOR_NONE                              "\033[0m"
#define HIGHLIGHT                               "\033[1;33m"

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static void SHELL_SendDataCallback(uint8_t *buf, uint32_t len);
static bool SHELL_ReceiveDataCallback(uint8_t *buf, uint32_t len, uint32_t to);
static int SHELL_Printf(const char *pcFormat, ...);

/*******************************************************************************
 * Variables
 ******************************************************************************/

/*******************************************************************************
 * Code
 ******************************************************************************/

static void SHELL_SendDataCallback(uint8_t *buf, uint32_t len)
{
    VCOM_Write(buf, len);
}

static bool SHELL_ReceiveDataCallback(uint8_t *buf, uint32_t len, uint32_t to)
{
    return VCOM_Read(buf, len, to);
}

static int SHELL_Printf(const char *pcFormat, ...)
{
    int32_t nbytes;
    va_list args;
    
    char *p_buffer = pvPortMalloc(CFG_SHELL_PRINT_MAX_LEN);

    va_start(args, pcFormat);

    nbytes = vsnprintf(p_buffer, CFG_SHELL_PRINT_MAX_LEN - 1, pcFormat, args);
    p_buffer[CFG_SHELL_PRINT_MAX_LEN - 1] = '\0';

    va_end(args);
    
    SHELL_SendDataCallback(p_buffer, nbytes);
    
    vPortFree(p_buffer);
    
    return 0;
}

void ShellTask(void *p_param)
{
    shell_context_struct user_context;  
    
    VCOM_Init();
    /* Delay 1s to wait enum done, so we can get the very first line of message on shell */
    vTaskDelay(pdMS_TO_TICKS(1000));

    SHELL_Init(&user_context, SHELL_SendDataCallback/* not used */, SHELL_ReceiveDataCallback, SHELL_Printf, HIGHLIGHT"[root@gateway"COLOR_NONE":~#] ");

    IpCmdRegister();
    
    while (1) {
        SHELL_Main(&user_context);
    }
}

void ShellInit(void)
{
    xTaskCreate(ShellTask,
                "ShellTask", 
                1024, 
                NULL, 
                tskIDLE_PRIORITY + 1,
                NULL);
}
