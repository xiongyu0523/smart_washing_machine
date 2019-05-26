/**
 * NOTE:
 *
 * HAL_TCP_xxx API reference implementation: wrappers/os/ubuntu/HAL_TCP_linux.c
 *
 */
#include "infra_types.h"
#include "infra_defs.h"
#include "infra_compat.h"
#include "wrappers_defs.h"
#include "stdarg.h"

#include <stdlib.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "timers.h"
#include "fsl_debug_console.h"

#include "sockets.h"
#include "netdb.h"

#define HAL_SEM_MAX_COUNT           (10)
#define HAL_SEM_INIT_COUNT          (0)

#define UINT32_IPADDR_TO_CSV_BYTES(a) ((uint8_t)((a) >> 24) & 0xFF), (uint8_t)(((a) >> 16) & 0xFF), (uint8_t)(((a) >> 8) & 0xFF), (uint8_t)((a)&0xFF)
#define CSV_BYTES_TO_UINT32_IPADDR(a0, a1, a2, a3)  (((uint32_t)(a0)&0xFF) << 24) | (((uint32_t)(a1)&0xFF) << 16) | (((uint32_t)(a2)&0xFF) << 8) | ((uint32_t)(a3)&0xFF)

static char _product_key[IOTX_PRODUCT_KEY_LEN + 1]       = "a1XGwg2WTT3";
static char _product_secret[IOTX_PRODUCT_SECRET_LEN + 1] = "eqlHSVlE9arvbXYJ";
static char _device_name[IOTX_DEVICE_NAME_LEN + 1]       = "V1lKTHntfe7DAkcZZsat";
static char _device_secret[IOTX_DEVICE_SECRET_LEN + 1]   = "xVUIJqGw0VYx3d9p9z7EHQV0ZJgWu0ru";

uint64_t HAL_UptimeMs(void);

static uint64_t _linux_get_time_ms(void)
{
    return HAL_UptimeMs();
}

static uint64_t _linux_time_left(uint64_t t_end, uint64_t t_now)
{
    uint64_t t_left;

    if (t_end > t_now) {
        t_left = t_end - t_now;
    } else {
        t_left = 0;
    }

    return t_left;
}


/**
 * @brief Deallocate memory block
 *
 * @param[in] ptr @n Pointer to a memory block previously allocated with platform_malloc.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Free(void *ptr)
{
    vPortFree(ptr);
}


/**
 * @brief Get device name from user's system persistent storage
 *
 * @param [ou] device_name: array to store device name, max length is IOTX_DEVICE_NAME_LEN
 * @return the actual length of device name
 */
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN])
{
    int len = strlen(_device_name);
    memset(device_name, 0x0, IOTX_DEVICE_NAME_LEN + 1);

    strncpy(device_name, _device_name, len);

    return strlen(device_name);
}


/**
 * @brief Get device secret from user's system persistent storage
 *
 * @param [ou] device_secret: array to store device secret, max length is IOTX_DEVICE_SECRET_LEN
 * @return the actual length of device secret
 */
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN])
{
    int len = strlen(_device_secret);
    memset(device_secret, 0x0, IOTX_DEVICE_SECRET_LEN + 1);

    strncpy(device_secret, _device_secret, len);

    return len;
}


/**
 * @brief Get firmware version
 *
 * @param [ou] version: array to store firmware version, max length is IOTX_FIRMWARE_VER_LEN
 * @return the actual length of firmware version
 */
int HAL_GetFirmwareVersion(char *version)
{
	return (int)1;
}


/**
 * @brief Get product key from user's system persistent storage
 *
 * @param [ou] product_key: array to store product key, max length is IOTX_PRODUCT_KEY_LEN
 * @return  the actual length of product key
 */
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN])
{
    int len = strlen(_product_key);
    memset(product_key, 0x0, IOTX_PRODUCT_KEY_LEN + 1);

    strncpy(product_key, _product_key, len);

    return len;
}


int HAL_GetProductSecret(char *product_secret)
{
    int len = strlen(_product_secret);
    memset(product_secret, 0x0, IOTX_PRODUCT_SECRET_LEN + 1);

    strncpy(product_secret, _product_secret, len);

    return len;
}


int HAL_Kv_Get(const char *key, void *val, int *buffer_len)
{
	return (int)1;
}


int HAL_Kv_Set(const char *key, const void *val, int len, int sync)
{
	return (int)1;
}


/**
 * @brief Allocates a block of size bytes of memory, returning a pointer to the beginning of the block.
 *
 * @param [in] size @n specify block size in bytes.
 * @return A pointer to the beginning of the block.
 * @see None.
 * @note Block value is indeterminate.
 */
void *HAL_Malloc(uint32_t size)
{
	return (void *)pvPortMalloc(size);
}


/**
 * @brief Create a mutex.
 *
 * @retval NULL : Initialize mutex failed.
 * @retval NOT_NULL : The mutex handle.
 * @see None.
 * @note None.
 */
void *HAL_MutexCreate(void)
{
    QueueHandle_t sem;

    sem = xSemaphoreCreateMutex();
    if (0 == sem) {
        return NULL;
    }

    return sem;
}


/**
 * @brief Destroy the specified mutex object, it will release related resource.
 *
 * @param [in] mutex @n The specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexDestroy(void *mutex)
{
    QueueHandle_t sem;
    if (mutex == NULL) {
        return;
    }
    sem = (QueueHandle_t)mutex;
    vSemaphoreDelete(sem);
}


/**
 * @brief Waits until the specified mutex is in the signaled state.
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexLock(void *mutex)
{
    BaseType_t ret;
    QueueHandle_t sem;
    if (mutex == NULL) {
        return;
    }

    sem = (QueueHandle_t)mutex;
    ret = xSemaphoreTake(sem, 0xffffffff);
    while (pdPASS != ret) {
        ret = xSemaphoreTake(sem, 0xffffffff);
    }
}


/**
 * @brief Releases ownership of the specified mutex object..
 *
 * @param [in] mutex @n the specified mutex.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_MutexUnlock(void *mutex)
{
    QueueHandle_t sem;
    if (mutex == NULL) {
        return;
    }
    sem = (QueueHandle_t)mutex;
    (void)xSemaphoreGive(sem);
}


/**
 * @brief Writes formatted data to stream.
 *
 * @param [in] fmt: @n String that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_Printf(const char *fmt, ...)
{
    PRINTF(fmt);
}


uint32_t HAL_Random(uint32_t region)
{
    int output;
    
    output = rand();
	return (region > 0) ? (output % region) : 0;
}


void HAL_Reboot()
{
	NVIC_SystemReset();
    while(1);
}


/**
 * @brief   create a semaphore
 *
 * @return semaphore handle.
 * @see None.
 * @note The recommended value of maximum count of the semaphore is 255.
 */
void *HAL_SemaphoreCreate(void)
{
    QueueHandle_t sem = 0;
    sem = xSemaphoreCreateCounting(HAL_SEM_MAX_COUNT, HAL_SEM_INIT_COUNT);
    if (0 == sem) {
        return NULL;
    }

    return sem;
}


/**
 * @brief   destory a semaphore
 *
 * @param[in] sem @n the specified sem.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphoreDestroy(void *sem)
{
    QueueHandle_t queue;

    if (sem == NULL) {
        return;
    }
    queue = (QueueHandle_t)sem;

    vSemaphoreDelete(queue);
}


/**
 * @brief   signal thread wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SemaphorePost(void *sem)
{
    QueueHandle_t queue;
    if (sem == NULL) {
        return;
    }
    queue = (QueueHandle_t)sem;
    (void)xSemaphoreGive(queue);
}


/**
 * @brief   wait on a semaphore
 *
 * @param[in] sem @n the specified semaphore.
 * @param[in] timeout_ms @n timeout interval in millisecond.
     If timeout_ms is PLATFORM_WAIT_INFINITE, the function will return only when the semaphore is signaled.
 * @return
   @verbatim
   =  0: The state of the specified object is signaled.
   =  -1: The time-out interval elapsed, and the object's state is nonsignaled.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_SemaphoreWait(void *sem, uint32_t timeout_ms)
{
    BaseType_t ret = 0;
    QueueHandle_t queue;
    if (sem == NULL) {
        return -1;
    }

    queue = (QueueHandle_t)sem;
    ret = xSemaphoreTake(queue, timeout_ms);
    if (pdPASS != ret) {
        return -1;
    }
    return 0;
}


/**
 * @brief Sleep thread itself.
 *
 * @param [in] ms @n the time interval for which execution is to be suspended, in milliseconds.
 * @return None.
 * @see None.
 * @note None.
 */
void HAL_SleepMs(uint32_t ms)
{
	vTaskDelay(pdMS_TO_TICKS(ms));
}


/**
 * @brief Writes formatted data to string.
 *
 * @param [out] str: @n String that holds written text.
 * @param [in] len: @n Maximum length of character will be written
 * @param [in] fmt: @n Format that contains the text to be written, it can optionally contain embedded format specifiers
     that specifies how subsequent arguments are converted for output.
 * @param [in] ...: @n the variable argument list, for formatted and inserted in the resulting string replacing their respective specifiers.
 * @return bytes of character successfully written into string.
 * @see None.
 * @note None.
 */
int HAL_Snprintf(char *str, const int len, const char *fmt, ...)
{
    va_list args;
    int rc;

    va_start(args, fmt);
    rc = vsnprintf(str, len, fmt, args);
    va_end(args);
    return rc;
}


void HAL_Srandom(uint32_t seed)
{
	srand(seed);
}


int HAL_Sys_Net_Is_Ready()
{
	return (int)1;
}


/**
 * @brief Destroy the specific TCP connection.
 *
 * @param [in] fd: @n Specify the TCP connection by handle.
 *
 * @return The result of destroy TCP connection.
 * @retval < 0 : Fail.
 * @retval   0 : Success.
 */
int HAL_TCP_Destroy(uintptr_t fd)
{
	return (int)1;
}


/**
 * @brief Establish a TCP connection.
 *
 * @param [in] host: @n Specify the hostname(IP) of the TCP server
 * @param [in] port: @n Specify the TCP port of TCP server
 *
 * @return The handle of TCP connection.
   @retval   0 : Fail.
   @retval > 0 : Success, the value is handle of this TCP connection.
 */
uintptr_t HAL_TCP_Establish(const char *host, uint16_t port)
{
    struct addrinfo hints;
    struct addrinfo *addrInfoList = NULL;
    struct addrinfo *cur = NULL;
    int fd = 0;
    int rc = 0;
    char service[6];

    memset(&hints, 0, sizeof(hints));

    PRINTF("establish tcp connection with server(host='%s', port=[%u])\n", host, port);

    hints.ai_family = AF_INET; /* only IPv4 */
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    sprintf(service, "%u", port);

    if ((rc = getaddrinfo(host, service, &hints, &addrInfoList)) != 0) {
        PRINTF("getaddrinfo error(%d), host = '%s', port = [%d]\n", rc, host, port);
        return (uintptr_t)(-1);
    }

    for (cur = addrInfoList; cur != NULL; cur = cur->ai_next) {
        if (cur->ai_family != AF_INET) {
            PRINTF("socket type error\n");
            rc = -1;
            continue;
        }

        fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
        if (fd < 0) {
            PRINTF("create socket error\n");
            rc = -1;
            continue;
        }

        if (connect(fd, cur->ai_addr, cur->ai_addrlen) == 0) {
            rc = fd;
            break;
        }

        close(fd);
        PRINTF("connect error\n");
        rc = -1;
    }

    if (-1 == rc) {
        printf("fail to establish tcp\n");
    } else {
        printf("success to establish tcp, fd=%d\n", rc);
    }
    freeaddrinfo(addrInfoList);

    return (uintptr_t)rc;
}


/**
 * @brief Read data from the specific TCP connection with timeout parameter.
 *        The API will return immediately if 'len' be received from the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a TCP connection.
 * @param [out] buf @n A pointer to a buffer to receive incoming data.
 * @param [out] len @n The length, in bytes, of the data pointed to by the 'buf' parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block 'timeout_ms' millisecond maximumly.
 *
 * @retval       -2 : TCP connection error occur.
 * @retval       -1 : TCP connection be closed by remote server.
 * @retval        0 : No any data be received in 'timeout_ms' timeout period.
 * @retval (0, len] : The total number of bytes be received in 'timeout_ms' timeout period.

 * @see None.
 */
int32_t HAL_TCP_Read(uintptr_t fd, char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret, err_code, tcp_fd;
    uint32_t len_recv;
    uint64_t t_end, t_left;
    fd_set sets;
    struct timeval timeout;

    t_end = _linux_get_time_ms() + timeout_ms;
    len_recv = 0;
    err_code = 0;

    if (fd >= FD_SETSIZE) {
        return -1;
    }
    tcp_fd = (int)fd;

    do {
        t_left = _linux_time_left(t_end, _linux_get_time_ms());
        if (0 == t_left) {
            break;
        }
        FD_ZERO(&sets);
        FD_SET(tcp_fd, &sets);

        timeout.tv_sec = t_left / 1000;
        timeout.tv_usec = (t_left % 1000) * 1000;

        ret = select(tcp_fd + 1, &sets, NULL, NULL, &timeout);
        if (ret > 0) {
            ret = recv(tcp_fd, buf + len_recv, len - len_recv, 0);
            if (ret > 0) {
                len_recv += ret;
            } else if (0 == ret) {
                printf("connection is closed\n");
                err_code = -1;
                break;
            } else {
                if (EINTR == errno) {
                    continue;
                }
                printf("recv fail\n");
                err_code = -2;
                break;
            }
        } else if (0 == ret) {
            break;
        } else {
            if (EINTR == errno) {
                continue;
            }
            printf("select-recv fail\n");
            err_code = -2;
            break;
        }
    } while ((len_recv < len));

    /* priority to return data bytes if any data be received from TCP connection. */
    /* It will get error code on next calling */
    return (0 != len_recv) ? len_recv : err_code;
}


/**
 * @brief Write data into the specific TCP connection.
 *        The API will return immediately if 'len' be written into the specific TCP connection.
 *
 * @param [in] fd @n A descriptor identifying a connection.
 * @param [in] buf @n A pointer to a buffer containing the data to be transmitted.
 * @param [in] len @n The length, in bytes, of the data pointed to by the 'buf' parameter.
 * @param [in] timeout_ms @n Specify the timeout value in millisecond. In other words, the API block 'timeout_ms' millisecond maximumly.
 *
 * @retval      < 0 : TCP connection error occur..
 * @retval        0 : No any data be write into the TCP connection in 'timeout_ms' timeout period.
 * @retval (0, len] : The total number of bytes be written in 'timeout_ms' timeout period.

 * @see None.
 */
int32_t HAL_TCP_Write(uintptr_t fd, const char *buf, uint32_t len, uint32_t timeout_ms)
{
    int ret,tcp_fd;
    uint32_t len_sent;
    uint64_t t_end, t_left;
    fd_set sets;
    int net_err = 0;

    t_end = _linux_get_time_ms() + timeout_ms;
    len_sent = 0;
    ret = 1; /* send one time if timeout_ms is value 0 */

    if (fd >= FD_SETSIZE) {
        return -1;
    }
    tcp_fd = (int)fd;

    do {
        t_left = _linux_time_left(t_end, _linux_get_time_ms());

        if (0 != t_left) {
            struct timeval timeout;

            FD_ZERO(&sets);
            FD_SET(tcp_fd, &sets);

            timeout.tv_sec = t_left / 1000;
            timeout.tv_usec = (t_left % 1000) * 1000;

            ret = select(tcp_fd + 1, NULL, &sets, NULL, &timeout);
            if (ret > 0) {
                if (0 == FD_ISSET(tcp_fd, &sets)) {
                    printf("Should NOT arrive\n");
                    /* If timeout in next loop, it will not sent any data */
                    ret = 0;
                    continue;
                }
            } else if (0 == ret) {
                printf("select-write timeout %d\n", tcp_fd);
                break;
            } else {
                if (EINTR == errno) {
                    printf("EINTR be caught\n");
                    continue;
                }

                printf("select-write fail, ret = select() = %d\n", ret);
                net_err = 1;
                break;
            }
        }

        if (ret > 0) {
            ret = send(tcp_fd, buf + len_sent, len - len_sent, 0);
            if (ret > 0) {
                len_sent += ret;
            } else if (0 == ret) {
                printf("No data be sent\n");
            } else {
                if (EINTR == errno) {
                    printf("EINTR be caught\n");
                    continue;
                }

                printf("send fail, ret = send() = %d\n", ret);
                net_err = 1;
                break;
            }
        }
    } while (!net_err && (len_sent < len) && (_linux_time_left(t_end, _linux_get_time_ms()) > 0));

    if (net_err) {
        return -1;
    } else {
        return len_sent;
    }
}


/**
 * @brief  create a thread
 *
 * @param[out] thread_handle @n The new thread handle, memory allocated before thread created and return it, free it after thread joined or exit.
 * @param[in] start_routine @n A pointer to the application-defined function to be executed by the thread.
        This pointer represents the starting address of the thread.
 * @param[in] arg @n A pointer to a variable to be passed to the start_routine.
 * @param[in] hal_os_thread_param @n A pointer to stack params.
 * @param[out] stack_used @n if platform used stack buffer, set stack_used to 1, otherwise set it to 0.
 * @return
   @verbatim
     = 0: on success.
     = -1: error occur.
   @endverbatim
 * @see None.
 * @note None.
 */
int HAL_ThreadCreate(
            void **thread_handle,
            void *(*work_routine)(void *),
            void *arg,
            hal_os_thread_param_t *hal_os_thread_param,
            int *stack_used)
{
    BaseType_t xResult;
    TaskHandle_t handle;

    if (thread_handle == NULL) {
        return -1;
    }

    if (work_routine == NULL) {
        return -1;
    }

    if (hal_os_thread_param == NULL) {
        return -1;
    }
    
    if (hal_os_thread_param->stack_size == 0) {
        return -1;
    }
    
    if (stack_used == NULL) {
        return -1;
    }

    *stack_used = 0;

	xResult = xTaskCreate((TaskFunction_t)work_routine,
                          hal_os_thread_param->name,
                          hal_os_thread_param->stack_size,
                          arg,
                          hal_os_thread_param->priority,
                          &handle);
    
    if (xResult == pdFALSE) {
        return -1;
    }
    
    *thread_handle = handle;
    
    return 0;
}

void HAL_ThreadDelete(void *thread_handle)
{
	vTaskDelete(*(TaskHandle_t *)thread_handle);
}

void *HAL_Timer_Create(const char *name, void (*func)(void *), void *user_data)
{
    return xTimerCreate(name, pdMS_TO_TICKS(10), pdFALSE, NULL, (TimerCallbackFunction_t)func);
}

int HAL_Timer_Delete(void *timer)
{ 
	return (xTimerDelete((TimerHandle_t)timer, 0) == pdPASS) ? 0 : -1;
}

int HAL_Timer_Start(void *timer, int ms)
{
    BaseType_t xResult; 

    xResult = xTimerChangePeriod((TimerHandle_t)timer, pdMS_TO_TICKS(ms), 0);
    if (xResult != pdPASS) {
        return -1;
    }
    
    return (xTimerStart((TimerHandle_t)timer, 0) == pdPASS) ? 0 : -1;
}


int HAL_Timer_Stop(void *timer)
{
	return (xTimerStop((TimerHandle_t)timer, 0) == pdPASS) ? 0 : -1;
}


int HAL_UDP_close_without_connect(intptr_t sockfd)
{
	return close((int)sockfd);
}


intptr_t HAL_UDP_create_without_connect(const char *host, unsigned short port)
{
    struct sockaddr_in addr;
    long sockfd;
    int opt_val = 1;
    struct hostent *hp;
    struct in_addr in;
    uint32_t ip;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        printf("socket");
        return -1;
    }
    if (0 == port) {
        return (intptr_t)sockfd;
    }

    memset(&addr, 0, sizeof(struct sockaddr_in));

    if (0 != setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR | SO_BROADCAST, &opt_val, sizeof(opt_val))) {
        printf("setsockopt");
        close(sockfd);
        return -1;
    }

    if (NULL == host) {
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
    } else {
        if (inet_aton(host, &in)) {
            ip = *(uint32_t *)&in;
        } else {
            hp = gethostbyname(host);
            if (!hp) {
                printf("can't resolute the host address \n");
                close(sockfd);
                return -1;
            }
            ip = *(uint32_t *)(hp->h_addr);
        }
        addr.sin_addr.s_addr = ip;
    }
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);

    if (-1 == bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in))) {
        close(sockfd);
        return -1;
    }
    printf("success to establish udp, fd=%d", (int)sockfd);

    return (intptr_t)sockfd;
}


int HAL_UDP_joinmulticast(intptr_t sockfd,
                          char *p_group)
{
    int err = -1;
    int socket_id = -1;
    int loop = 0;
    struct ip_mreq mreq;

    if (NULL == p_group) {
        return -1;
    }

    /*set loopback*/
    socket_id = (int)sockfd;
    err = setsockopt(socket_id, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));
    if (err < 0) {
        printf("setsockopt");
        return err;
    }

    mreq.imr_multiaddr.s_addr = inet_addr(p_group);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY); /*default networt interface*/

    /*join to the multicast group*/
    err = setsockopt(socket_id, IPPROTO_IP, IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq));
    if (err < 0) {
        printf("setsockopt");
        return err;
    }

    return 0;
}


int HAL_UDP_recvfrom(intptr_t sockfd,
                     NetworkAddr *p_remote,
                     unsigned char *p_data,
                     unsigned int datalen,
                     unsigned int timeout_ms)
{
    int ret;
    struct sockaddr_in addr;
    socklen_t addr_len = sizeof(addr);
    fd_set read_fds;
    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    FD_ZERO(&read_fds);
    FD_SET(sockfd, &read_fds);

    ret = select(sockfd + 1, &read_fds, NULL, NULL, &timeout);
    if (ret == 0) {
        return 0;    /* receive timeout */
    }

    if (ret < 0) {
        if (errno == EINTR) {
            return -3;    /* want read */
        }
        return -4; /* receive failed */
    }

    ret = recvfrom(sockfd, p_data, datalen, 0, (struct sockaddr *)&addr, &addr_len);
    if (ret > 0) {
        if (NULL != p_remote) {
            p_remote->port = ntohs(addr.sin_port);

            strcpy((char *)p_remote->addr, inet_ntoa(addr.sin_addr));
        }

        return ret;
    }

    return -1;
}


int HAL_UDP_sendto(intptr_t sockfd,
                   const NetworkAddr *p_remote,
                   const unsigned char *p_data,
                   unsigned int datalen,
                   unsigned int timeout_ms)
{
    int ret;
    uint32_t ip;
    struct in_addr in;
    struct hostent *hp;
    struct sockaddr_in addr;
    fd_set write_fds;
    struct timeval timeout = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};

    if (inet_aton((char *)p_remote->addr, &in)) {
        ip = *(uint32_t *)&in;
    } else {
        hp = gethostbyname((char *)p_remote->addr);
        if (!hp) {
            printf("can't resolute the host address \n");
            return -1;
        }
        ip = *(uint32_t *)(hp->h_addr);
    }

    FD_ZERO(&write_fds);
    FD_SET(sockfd, &write_fds);

    ret = select(sockfd + 1, NULL, &write_fds, NULL, &timeout);
    if (ret == 0) {
        return 0;    /* write timeout */
    }

    if (ret < 0) {
        if (errno == EINTR) {
            return -3;    /* want write */
        }
        return -4; /* write failed */
    }

    addr.sin_addr.s_addr = ip;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(p_remote->port);

    ret = sendto(sockfd, p_data, datalen, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));

    if (ret < 0) {
        printf("sendto");
    }

    return (ret) > 0 ? ret : -1;
}


/**
 * @brief Retrieves the number of milliseconds that have elapsed since the system was boot.
 *
 * @return the number of milliseconds.
 * @see None.
 * @note None.
 */
uint64_t HAL_UptimeMs(void)
{
	return (uint64_t)xTaskGetTickCount();
}

int HAL_Vsnprintf(char *str, const int len, const char *format, va_list ap)
{
	return vsnprintf(str, len, format, ap);
}

extern struct netif fsl_netif0;

uint32_t HAL_Wifi_Get_IP(char ip_str[NETWORK_ADDR_LEN], const char *ifname)
{
    strncpy(ip_str, inet_ntoa(fsl_netif0.ip_addr.addr), NETWORK_ADDR_LEN);
    return fsl_netif0.ip_addr.addr;
}


char *HAL_Wifi_Get_Mac(char mac_str[HAL_MAC_LEN])
{
    snprintf(mac_str, HAL_MAC_LEN, "%02X:%02X:%02X:%02X:%02X:%02X", 
             fsl_netif0.hwaddr[0],
             fsl_netif0.hwaddr[1],
             fsl_netif0.hwaddr[2],
             fsl_netif0.hwaddr[3],
             fsl_netif0.hwaddr[4],
             fsl_netif0.hwaddr[5]);
    
    return mac_str;
}


