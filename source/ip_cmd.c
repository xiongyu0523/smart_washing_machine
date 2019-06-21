/* Copyright 2018 NXP */

#include <stdint.h>
#include <string.h>

#include "fsl_shell.h"

#include "FreeRTOS.h"

#include "ip_cmd.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/

#define MAX_AP_NUMBERS                      20

/*******************************************************************************
 * Prototypes
 ******************************************************************************/

static int32_t ifconfig(p_shell_context_t context, int32_t argc, char **argv);
static int32_t dhclient(p_shell_context_t context, int32_t argc, char **argv);
static int32_t iwlist(p_shell_context_t context, int32_t argc, char **argv);
static int32_t iwconfig(p_shell_context_t context, int32_t argc, char **argv);

/*******************************************************************************
 * Variables
 ******************************************************************************/

static const char ifconfigHelp[] = "Usage:\r\n"
                                   "  ifconfig\r\n"
                                   "  ifconfig [wlan0|eth0]\r\n"
                                   "  ifconfig [wlan0|eth0] [up|down]\r\n";
static const char dhclientHelp[] = "Usage:\r\n"
                                   "  dhclient [wlan0|eth0]\r\n";
static const char iwlistHelp[] = "Usage:\r\n"
                                  "  iwlist [wlan0] scan\r\n";
static const char iwconfigHelp[] = "Usage:\r\n"
                                  "  iwconfig [wlan0] ssid <ssid>\r\n"
                                  "  iwconfig [wlan0] ssid <ssid> key <key> sec [wep|wpa|wpa2]\r\n";

static const shell_command_context_t s_ip_cmds[] = {
    {"ifconfig",     "\"ifconfig\":             Config network interface\r\n",          ifconfig,       SHELL_OPTIONAL_PARAMS},
    {"dhclient",     "\"dhclient\":             Get IP address from DCHP server\r\n",   dhclient,       1},
    {"iwlist",       "\"iwlist\":               Scan wireless network\r\n",             iwlist,         SHELL_OPTIONAL_PARAMS},
    {"iwconfig",     "\"iwconfig\":             Config wireless network\r\n",           iwconfig,       SHELL_OPTIONAL_PARAMS}
};

/*******************************************************************************
 * Code
 ******************************************************************************/

static int32_t ifconfig(p_shell_context_t context, int32_t argc, char **argv)
{
    BaseType_t ret = pdFAIL;
    
    switch (argc)
    {
        case 1:
            // TODO: show all available network interface status
            break;

        case 2:
            if ((strcmp(argv[1], HELP_STRING) == 0) || (strcmp(argv[1], HELP_STRING) == 0)) {
                context->printf_data_func("%s", ifconfigHelp);
            } else if (strcmp(argv[1], "wlan0") == 0) {
                // TODO: show wlan0 network interface status
            } {
                goto err;
            }
            break;

        case 3:
            if (strcmp(argv[1], "wlan0") == 0) {

                if (strcmp(argv[2], "up") == 0) {
                    
                    // if up

                } else if (strcmp(argv[2], "down") == 0) {
                    
                    // if down

                } else {
                	goto err;
                }
            } else {
                goto err;
            }
            break;

        default:
        	goto err;
            break;
    }

    return 0;

err:
    context->printf_data_func("Error: Incorrect command or parameters\r\n");
    return -1;      
}

static int32_t dhclient(p_shell_context_t context, int32_t argc, char **argv)
{
    int32_t rt = -1;
    uint8_t tmp_ip[4] = {0}, dhcp_ip[4] = {0}, ret = 0;

    if (strcmp(argv[1], HELP_STRING) == 0) {
        context->printf_data_func("%s", dhclientHelp);
    } else if (strcmp(argv[1], "wlan0") == 0) {
        
        // dhcp

    } else {
    	context->printf_data_func("Error: Incorrect command or parameters\r\n");
    }

    return rt;
}


static int32_t iwlist(p_shell_context_t context, int32_t argc, char **argv)
{
    switch (argc)
    {
        case 2:
            if (strcmp(argv[1], HELP_STRING) == 0) {
                context->printf_data_func("%s", iwlistHelp);
            }
            break;

        case 3:
            if (strcmp(argv[1], "wlan0") == 0) {
                if (strcmp(argv[2], "scan") == 0) {
                    
                    // scan ap and print
                    
                } else {
                    goto err;
                }
            } else {
                goto err;
            }
            break;

        default:
            goto err;
            break;
    }

    return 0;

err:
    context->printf_data_func("Error: Incorrect command or parameters\r\n");
    return -1;    
}

static int32_t iwconfig(p_shell_context_t context, int32_t argc, char **argv)
{
    int32_t rt = -1;

    switch (argc)
    {
        case 2:
            if (strcmp(argv[1], HELP_STRING) == 0) {
                context->printf_data_func("%s", iwconfigHelp);
            } else {
                goto err;
            }
            break;
        
        case 4:
            if (strcmp(argv[1], "wlan0") == 0) {
                if (strcmp(argv[2], "ssid") == 0) {
                    // connect to AP without password
                } else {
                    goto err;
                }
            } else {
                goto err;
            }
            break;

        case 6:
            if (strcmp(argv[1], "wlan0") == 0) {
                if (strcmp(argv[2], "ssid") == 0) {
                    if (strcmp(argv[4], "key") == 0) {
                        
                        // connect to AP with password
                        app_process_wifi_config(argv[3],argv[5]);
                    } else {
                        goto err;
                    }
                } else {
                    goto err;
                }
            } else {
                goto err;
            }
            break;

        default:
            goto err;
            break;
    }
    
    return rt;

err:
    context->printf_data_func("Error: Incorrect command or parameters\r\n");
    return -1;
}

void IpCmdRegister(void)
{
    for (uint32_t i = 0; i < (sizeof(s_ip_cmds) / sizeof(s_ip_cmds[0])); i++) {
        SHELL_RegisterCommand(&s_ip_cmds[i]);
    }
}

