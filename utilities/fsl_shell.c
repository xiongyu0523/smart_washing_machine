/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * POSIX getopt for Windows
 * Code given out at the 1985 UNIFORUM conference in Dallas.
 *
 * From std-unix@ut-sally.UUCP (Moderator, John Quarterman) Sun Nov  3 14:34:15 1985
 * Relay-Version: version B 2.10.3 4.3bsd-beta 6/6/85; site gatech.CSNET
 * Posting-Version: version B 2.10.2 9/18/84; site ut-sally.UUCP
 * Path: gatech!akgua!mhuxv!mhuxt!mhuxr!ulysses!allegra!mit-eddie!genrad!panda!talcott!harvard!seismo!ut-sally!std-unix
 * From: std-unix@ut-sally.UUCP (Moderator, John Quarterman)
 * Newsgroups: mod.std.unix
 * Subject: public domain AT&T getopt source
 * Message-ID: <3352@ut-sally.UUCP>
 * Date: 3 Nov 85 19:34:15 GMT
 * Date-Received: 4 Nov 85 12:25:09 GMT
 * Organization: IEEE/P1003 Portable Operating System Environment Committee
 * Lines: 91
 * Approved: jsq@ut-sally.UUC
 * Here's something you've all been waiting for:  the AT&T public domain
 * source for getopt(3).  It is the code which was given out at the 1985
 * UNIFORUM conference in Dallas.  I obtained it by electronic mail
 * directly from AT&T.  The people there assure me that it is indeed
 * in the public domain
 * There is no manual page.  That is because the one they gave out at
 * UNIFORUM was slightly different from the current System V Release 2
 * manual page.  The difference apparently involved a note about the
 * famous rules 5 and 6, recommending using white space between an option
 * and its first argument, and not grouping options that have arguments.
 * Getopt itself is currently lenient about both of these things White
 * space is allowed, but not mandatory, and the last option in a group can
 * have an argument.  That particular version of the man page evidently
 * has no official existence, and my source at AT&T did not send a copy.
 * The current SVR2 man page reflects the actual behavor of this getopt.
 * However, I am not about to post a copy of anything licensed by AT&T.
 */

#include <assert.h>
#include "fsl_shell.h"
#include "FreeRTOS.h"

/*******************************************************************************
 * Definitions
 ******************************************************************************/
#define KEY_ESC (0x1BU)
#define KET_DEL (0x7FU)

/*******************************************************************************
 * Prototypes
 ******************************************************************************/
static int32_t HelpCommand(p_shell_context_t context, int32_t argc, char **argv); /*!< help command */

static int32_t ParseLine(const char *cmd, uint32_t len, char *argv[SHELL_MAX_ARGS]); /*!< parse line command */

static int32_t StrCompare(const char *str1, const char *str2, int32_t count); /*!< compare string command */

static int32_t ProcessCommand(p_shell_context_t context, const char *cmd); /*!< process a command */

static void GetHistoryCommand(p_shell_context_t context, uint8_t hist_pos); /*!< get commands history */

static void AutoComplete(p_shell_context_t context); /*!< auto complete command */

static uint8_t GetChar(p_shell_context_t context); /*!< get a char from communication interface */

static int32_t StrLen(const char *str); /*!< get string length */

static char *StrCopy(char *dest, const char *src, int32_t count); /*!< string copy */

/*******************************************************************************
 * Variables
 ******************************************************************************/
static const shell_command_context_t xHelpCommand = {"help", "\"help\":                 Lists all the registered commands\r\n",
                                                     HelpCommand, 0};

static shell_command_context_list_t g_RegisteredCommands;

static char g_paramBuffer[SHELL_BUFFER_SIZE];

/*******************************************************************************
 * Code
 ******************************************************************************/
void SHELL_Init(
    p_shell_context_t context, send_data_cb_t send_cb, recv_data_cb_t recv_cb, printf_data_t shell_printf, char *prompt)
{
    assert(send_cb != NULL);
    assert(recv_cb != NULL);
    assert(prompt != NULL);
    assert(shell_printf != NULL);

    /* Memset for context */
    memset(context, 0, sizeof(shell_context_struct));
    context->send_data_func = send_cb;
    context->recv_data_func = recv_cb;
    context->printf_data_func = shell_printf;
    context->prompt = prompt;

    SHELL_RegisterCommand(&xHelpCommand);
}

int32_t SHELL_Main(p_shell_context_t context)
{
    uint8_t ch;
    int32_t i;

    if (!context)
    {
        return -1;
    }

    context->exit = false;
    context->printf_data_func("\r\nMCU IoT Gateway (build: %s)\r\n", __DATE__);
    context->printf_data_func("Copyright (c) 2018 NXP Semiconductor\r\n");
    context->printf_data_func(context->prompt);

    while (1)
    {
        if (context->exit)
        {
            break;
        }
        ch = GetChar(context);
        /* If error occured when getting a char, continue to receive a new char. */
        if ((uint8_t)(-1) == ch)
        {
            continue;
        }
        /* Special key */
        if (ch == KEY_ESC)
        {
            context->stat = kSHELL_Special;
            continue;
        }
        else if (context->stat == kSHELL_Special)
        {
            /* Function key */
            if (ch == '[')
            {
                context->stat = kSHELL_Function;
                continue;
            }
            context->stat = kSHELL_Normal;
        }
        else if (context->stat == kSHELL_Function)
        {
            context->stat = kSHELL_Normal;

            switch ((uint8_t)ch)
            {
                /* History operation here */
                case 'A': /* Up key */
                    GetHistoryCommand(context, context->hist_current);
                    if (context->hist_current < (context->hist_count - 1))
                    {
                        context->hist_current++;
                    }
                    break;
                case 'B': /* Down key */
                    GetHistoryCommand(context, context->hist_current);
                    if (context->hist_current > 0)
                    {
                        context->hist_current--;
                    }
                    break;
                case 'D': /* Left key */
                    if (context->c_pos)
                    {
                        context->printf_data_func("\b");
                        context->c_pos--;
                    }
                    break;
                case 'C': /* Right key */
                    if (context->c_pos < context->l_pos)
                    {
                        context->printf_data_func("%c", context->line[context->c_pos]);
                        context->c_pos++;
                    }
                    break;
                default:
                    break;
            }
            continue;
        }
        /* Handle tab key */
        else if (ch == '\t')
        {
#if SHELL_AUTO_COMPLETE
            /* Move the cursor to the beginning of line */
            for (i = 0; i < context->c_pos; i++)
            {
                context->printf_data_func("\b");
            }
            /* Do auto complete */
            AutoComplete(context);
            /* Move position to end */
            context->c_pos = context->l_pos = StrLen(context->line);
#endif
            continue;
        }
#if SHELL_SEARCH_IN_HIST
        /* Search command in history */
        else if ((ch == '`') && (context->l_pos == 0) && (context->line[0] == 0x00))
        {
        }
#endif
        /* Handle backspace key */
        else if ((ch == KET_DEL) || (ch == '\b'))
        {
            /* There must be at last one char */
            if (context->c_pos == 0)
            {
                continue;
            }

            context->l_pos--;
            context->c_pos--;

            if (context->l_pos > context->c_pos)
            {
                memmove(&context->line[context->c_pos], &context->line[context->c_pos + 1],
                        context->l_pos - context->c_pos);
                context->line[context->l_pos] = 0;
                context->printf_data_func("\b%s  \b", &context->line[context->c_pos]);

                /* Reset position */
                for (i = context->c_pos; i <= context->l_pos; i++)
                {
                    context->printf_data_func("\b");
                }
            }
            else /* Normal backspace operation */
            {
                context->printf_data_func("\b \b");
                context->line[context->l_pos] = 0;
            }
            continue;
        }
        else
        {
        }

        /* Input too long */
        if (context->l_pos >= (SHELL_BUFFER_SIZE - 1))
        {
            context->l_pos = 0;
        }

        /* Handle end of line, break */
        if ((ch == '\r') || (ch == '\n'))
        {
            static char endoflinechar = 0U;

            if ((endoflinechar != 0U) && (endoflinechar != ch))
            {
                continue;
            }
            else
            {
                endoflinechar = ch;
                context->printf_data_func("\r\n");
                /* If command line is NULL, will start a new transfer */
                if (0U == StrLen(context->line))
                {
                    context->printf_data_func(context->prompt);
                    continue;
                }

                if (ProcessCommand(context, context->line) == 0) {
                    
                    uint8_t tmpCommandLen = StrLen(context->line);
                    /* Compare with last command. Push back to history buffer if different */
                    if (tmpCommandLen != StrCompare(context->line, context->hist_buf[0], StrLen(context->line)))
                    {
                        for (uint32_t i = SHELL_HIST_MAX - 1; i > 0; i--)
                        {
                            memset(context->hist_buf[i], '\0', SHELL_BUFFER_SIZE);
                            tmpCommandLen = StrLen(context->hist_buf[i - 1]);
                            StrCopy(context->hist_buf[i], context->hist_buf[i - 1], tmpCommandLen);
                        }
                        memset(context->hist_buf[0], '\0', SHELL_BUFFER_SIZE);
                        tmpCommandLen = StrLen(context->line);
                        StrCopy(context->hist_buf[0], context->line, tmpCommandLen);
                        if (context->hist_count < SHELL_HIST_MAX)
                        {
                            context->hist_count++;
                        }
                    }
                }

                /* Reset all params */
                context->c_pos = context->l_pos = 0;
                context->hist_current = 0;
                context->printf_data_func(context->prompt);
                memset(context->line, 0, sizeof(context->line));
                continue;
            }
        }

        /* Normal character */
        if (context->c_pos < context->l_pos)
        {
            memmove(&context->line[context->c_pos + 1], &context->line[context->c_pos],
                    context->l_pos - context->c_pos);
            context->line[context->c_pos] = ch;
            context->printf_data_func("%s", &context->line[context->c_pos]);
            /* Move the cursor to new position */
            for (i = context->c_pos; i < context->l_pos; i++)
            {
                context->printf_data_func("\b");
            }
        }
        else
        {
            context->line[context->l_pos] = ch;
            context->printf_data_func("%c", ch);
        }

        ch = 0;
        context->l_pos++;
        context->c_pos++;
    }
    return 0;
}

static int32_t HelpCommand(p_shell_context_t context, int32_t argc, char **argv)
{
    uint8_t i = 0;

    for (i = 0; i < g_RegisteredCommands.numberOfCommandInList; i++)
    {
        context->printf_data_func(g_RegisteredCommands.CommandList[i]->pcHelpString);
    }
    return 0;
}

static int32_t ProcessCommand(p_shell_context_t context, const char *cmd)
{
    const shell_command_context_t *tmpCommand = NULL;
    const char *tmpCommandString              = NULL;
    uint8_t tmpCommandLen                     = 0;
    bool is_param_valid                       = false;
    bool is_cmd_found                         = false;
    int32_t argc = 0;
    char *argv[SHELL_MAX_ARGS];
    int32_t rt = -1;

    tmpCommandLen = StrLen(cmd);
    argc = ParseLine(cmd, tmpCommandLen, argv);

    if (argc > 0)
    {
        for (uint32_t i = 0; i < g_RegisteredCommands.numberOfCommandInList; i++)
        {
            tmpCommand = g_RegisteredCommands.CommandList[i];
            tmpCommandString = tmpCommand->pcCommand;
            tmpCommandLen = StrLen(tmpCommandString);

            if (StrCompare(tmpCommandString, argv[0], tmpCommandLen) == 0)
            {
                is_cmd_found = true;
                
                /* support commands with optional number of parameters */
                if (tmpCommand->cExpectedNumberOfParameters == SHELL_OPTIONAL_PARAMS)
                {
                    is_param_valid = true;
                }
                else if ((tmpCommand->cExpectedNumberOfParameters == 0) && (argc == 1))
                {
                    is_param_valid = true;
                }
                else if (tmpCommand->cExpectedNumberOfParameters > 0)
                {
                    if ((argc - 1) == tmpCommand->cExpectedNumberOfParameters)
                    {
                        is_param_valid = true;
                    }
                }
                
                break;
            }
        }

        if (is_cmd_found) {
            if (is_param_valid) {
                
                tmpCommand->pFuncCallBack(context, argc, argv);
                rt = 0;

            } else {
                context->printf_data_func("Error: Incorrect command or parameters\r\n");
            }
        } else {
            context->printf_data_func("\r\nCommand not recognised.  Enter 'help' to view a list of available commands.\r\n\r\n");
        }
    }

    return rt;
}

static void GetHistoryCommand(p_shell_context_t context, uint8_t hist_pos)
{
    uint8_t i;
    uint32_t tmp;

    if (context->hist_buf[0][0] == '\0')
    {
        context->hist_current = 0;
        return;
    }
    if (hist_pos >= SHELL_HIST_MAX)
    {
        hist_pos = SHELL_HIST_MAX - 1;
    }
    tmp = StrLen(context->line);
    /* Clear current if have */
    if (tmp > 0)
    {
        memset(context->line, '\0', tmp);
        for (i = 0; i < tmp; i++)
        {
            context->printf_data_func("\b \b");
        }
    }

    context->l_pos = StrLen(context->hist_buf[hist_pos]);
    context->c_pos = context->l_pos;
    StrCopy(context->line, context->hist_buf[hist_pos], context->l_pos);
    context->printf_data_func(context->hist_buf[hist_pos]);
}

static void AutoComplete(p_shell_context_t context)
{
    int32_t len;
    int32_t minLen;
    uint8_t i = 0;
    const shell_command_context_t *tmpCommand = NULL;
    const char *namePtr;
    const char *cmdName;

    minLen = 0;
    namePtr = NULL;

    if (!StrLen(context->line))
    {
        return;
    }
    context->printf_data_func("\r\n");
    /* Empty tab, list all commands */
    if (context->line[0] == '\0')
    {
        HelpCommand(context, 0, NULL);
        return;
    }
    /* Do auto complete */
    for (i = 0; i < g_RegisteredCommands.numberOfCommandInList; i++)
    {
        tmpCommand = g_RegisteredCommands.CommandList[i];
        cmdName = tmpCommand->pcCommand;
        if (StrCompare(context->line, cmdName, StrLen(context->line)) == 0)
        {
            if (minLen == 0)
            {
                namePtr = cmdName;
                minLen = StrLen(namePtr);
                /* Show possible matches */
                context->printf_data_func("%s\r\n", cmdName);
                continue;
            }
            len = StrCompare(namePtr, cmdName, StrLen(namePtr));
            if (len < 0)
            {
                len = len * (-1);
            }
            if (len < minLen)
            {
                minLen = len;
            }
        }
    }
    /* Auto complete string */
    if (namePtr)
    {
        StrCopy(context->line, namePtr, minLen);
    }
    context->printf_data_func("%s%s", context->prompt, context->line);
    return;
}

static char *StrCopy(char *dest, const char *src, int32_t count)
{
    char *ret = dest;
    int32_t i = 0;

    for (i = 0; i < count; i++)
    {
        dest[i] = src[i];
    }

    return ret;
}

static int32_t StrLen(const char *str)
{
    int32_t i = 0;

    while (*str)
    {
        str++;
        i++;
    }
    return i;
}

static int32_t StrCompare(const char *str1, const char *str2, int32_t count)
{
    while (count--)
    {
        if (*str1++ != *str2++)
        {
            return *(unsigned char *)(str1 - 1) - *(unsigned char *)(str2 - 1);
        }
    }
    return 0;
}

static int32_t ParseLine(const char *cmd, uint32_t len, char *argv[SHELL_MAX_ARGS])
{
    uint32_t argc;
    char *p;
    uint32_t position;

    /* Init params */
    memset(g_paramBuffer, '\0', len + 1);
    StrCopy(g_paramBuffer, cmd, len);

    p = g_paramBuffer;
    position = 0;
    argc = 0;

    while (position < len)
    {
        /* Skip all blanks */
        while (((char)(*p) == ' ') && (position < len))
        {
            *p = '\0';
            p++;
            position++;
        }
        /* Process begin of a string */
        if (*p == '"')
        {
            p++;
            position++;
            argv[argc] = p;
            argc++;
            /* Skip this string */
            while ((*p != '"') && (position < len))
            {
                p++;
                position++;
            }
            /* Skip '"' */
            *p = '\0';
            p++;
            position++;
        }
        else /* Normal char */
        {
            argv[argc] = p;
            argc++;
            while (((char)*p != ' ') && ((char)*p != '\t') && (position < len))
            {
                p++;
                position++;
            }
        }
    }
    return argc;
}

int32_t SHELL_RegisterCommand(const shell_command_context_t *command_context)
{
    int32_t result = 0;

    /* If have room  in command list */
    if (g_RegisteredCommands.numberOfCommandInList < SHELL_MAX_CMD)
    {
        g_RegisteredCommands.CommandList[g_RegisteredCommands.numberOfCommandInList++] = command_context;
    }
    else
    {
        result = -1;
    }
    return result;
}

static uint8_t GetChar(p_shell_context_t context)
{
    uint8_t ch;

#if SHELL_USE_FILE_STREAM
    ch = fgetc(context->STDIN);
#else
    (void)context->recv_data_func(&ch, 1U, portMAX_DELAY);
#endif
    return ch;
}