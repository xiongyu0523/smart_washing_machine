#include <stdarg.h>
#include "board.h"
#include "fsl_lpuart.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"
#include "portmacro.h"
#define			HEX_OUTPUT			16
#define			DECIMAL_OUTPUT		10
#define			OCTAL_OUTPUT		8
TaskHandle_t log_task_handle = NULL;
#define HexToAscii(hex) (uint8_t)( ((hex) & 0x0F) + ((((hex) & 0x0F) <= 9) ? '0' : ('A'-10)) )
QueueHandle_t log_mutex;

#define PTR_MASK			0x1ff
static char printf_buffer[512];
static uint16_t	printf_w_ptr = 0;
static uint16_t	printf_r_ptr = 0;

static char rec_buffer[128];
static char rng_buffer[128];

static char rx_buffer[256];
static uint8_t rx_r_ptr = 0;
static uint8_t rx_w_ptr = 0;

static const char prefix[] = "";
static const char infix[] = "";
static const char changeLine[]="\r\n";
static void puts(char *s){

	unsigned int i=0;
	while((*s != '\0')){
		printf_buffer[printf_w_ptr++ & PTR_MASK] = *s++;
	}
}

static void putchar(char c){
	printf_buffer[printf_w_ptr++ & PTR_MASK] = c;
}

static void puti(int num, int base){
	char re[]="0123456789ABCDEF";

	char buf[50];

	char *addr = &buf[49];

	*addr = '\0';

	do{
		*--addr = re[num%base];
		num/=base;
	}while(num!=0);

	puts(addr);
}


static void DbgConsole_PrintCallback(char *buf, int32_t *indicator, char dbgVal, int len)
{
    int i = 0;
    for (i = 0; i < len; i++)
    {
        if (((uint32_t)*indicator + 1UL) >= 128)
        {
			int j=0;
			while(j<(uint32_t)(*indicator)){
				printf_buffer[printf_w_ptr++ & PTR_MASK] = buf[j++];
                                
			}
            *indicator = 0;
        }

        buf[*indicator] = dbgVal;
        (*indicator)++;
    }
}
#if 0
void HAL_Printf(const char *formatString, ...)
#else
int format_printf(const char *formatString, ...)
#endif
{
    va_list ap;
    int logLength = 0, dbgResult = 0;
    char printBuf[128] = {0};


    va_start(ap, formatString);
	xSemaphoreTake(log_mutex, 0xffffffff);
    /* format print log first */
    logLength = StrFormatPrintf(formatString, ap, printBuf, DbgConsole_PrintCallback);
    /* print log */
	int i=0;
	while(i<logLength){
		printf_buffer[printf_w_ptr++ & PTR_MASK] = printBuf[i++];
	}

	xSemaphoreGive(log_mutex);
    va_end(ap);
	vTaskResume(log_task_handle);
#if 0
    return dbgResult;
#endif
}

void log_send(char *str,...){
	char span;
	long j;
	char *s;
	//char *msg;
	va_list arg_ptr;
	va_start(arg_ptr, str);

	while((span = *(str++))){
		if(span != '%'){
			putchar(span);
		}else{
			span = *(str++);
			if(span == 'c'){
				j = va_arg(arg_ptr,int);//get value of char
				putchar(j);
			}else if(span == 'd'){
				j = va_arg(arg_ptr,int);//get value of char
				if(j<0){
					putchar('-');
					j = -j;
				}
				puti(j,DECIMAL_OUTPUT);
			}else if(span == 's'){
				s = va_arg(arg_ptr,char *);//get string value
				puts(s);
			}else if(span == 'o'){
				j = va_arg(arg_ptr,unsigned int);//get octal value
				puti(j,OCTAL_OUTPUT);
			}else if(span == 'x'){
					j = va_arg(arg_ptr,unsigned int);//get hex value
					puti(j,HEX_OUTPUT);
			}else if(span == 0){
				break;
			}else{
				putchar(span);
			}
		}

	}
	va_end(arg_ptr);
	vTaskResume(log_task_handle);;
}


void log_write_hex(uint8_t *ptr, uint16_t len){
	uint16_t tl = 0;
	char re[]="0123456789ABCDEF";
	char buf[50];
	while(tl < len){
		char *addr = &buf[49];
		*addr = '\0';
		int num = ptr[tl++];
		do{
			*--addr = re[num%HEX_OUTPUT];
			num/=HEX_OUTPUT;
		}while(num!=0);
		puts(addr);
	}
	puts((char *)changeLine);
}


void log_task(void *arg){
	xSemaphoreTake(log_mutex, 0xffffffff);
	while(printf_w_ptr != printf_r_ptr){
#if 1
		 
         LPUART_WriteByte(LPUART1, printf_buffer[printf_r_ptr++ & PTR_MASK]);
                
#else
          LPUART_RTOS_Send(&handle, (uint8_t *)printf_buffer + printf_r_ptr,printf_w_ptr - printf_r_ptr);
          printf_r_ptr = printf_w_ptr;
#endif
          while(!(kLPUART_TxDataRegEmptyFlag & LPUART_GetStatusFlags(LPUART1)));
          
	}
	xSemaphoreGive(log_mutex);
}

void log_rx(void *args){
	if(rx_r_ptr != rx_w_ptr){
		app_process_recive_cmd(rx_buffer + rx_r_ptr, (rx_w_ptr - rx_r_ptr));
		rx_r_ptr = rx_w_ptr;
	}


}


static void log_tx_rx_task(void *args){
   	//init_done();
	while(1){
#if 1
		
		log_rx(NULL);
#else
                int n = 0;
                LPUART_RTOS_Receive(&handle, rec_buffer, sizeof(rec_buffer), &n);
                int i = 0;
                while(n>0){
                  rx_buffer[rx_w_ptr++] = rec_buffer[i++];
                  n--;
                }
                if(rx_buffer[rx_w_ptr] == '\n'){
                  log_rx(NULL);
                }
#endif
		log_task(NULL);
		vTaskSuspend(log_task_handle);
                
	}
}
#if 1
void LPUART1_IRQHandler(void)
{
    char data;

    /* If new data arrived. */
    if ((kLPUART_RxDataRegFullFlag)&LPUART_GetStatusFlags(LPUART1))
    {
        data = LPUART_ReadByte(LPUART1);
		rx_buffer[rx_w_ptr++] = data;
		if( (data == '\n') ) {

                      BaseType_t xHigherPriorityTaskWoken = xTaskResumeFromISR( log_task_handle );
                      portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
                    
		}
    }
}
#endif


void log_init(void ){
  BaseType_t xReturned;
  log_mutex = xSemaphoreCreateMutex();
  
  /* Create the task, storing the handle. */
  xReturned = xTaskCreate(
                  log_tx_rx_task,       /* Function that implements the task. */
                  "log_txrx",          /* Text name for the task. */
                  512,      /* Stack size in words, not bytes. */
                  NULL,    /* Parameter passed into the task. */
                  (3),/* Priority at which the task is created. */
                  &log_task_handle);      /* Used to pass out the created task's handle. */

  if( xReturned != pdPASS )
  {
      /* The task was created.  Use the task's handle to delete the task. */
      while(1);
  }
#if 0
  NVIC_SetPriority(LPUART1_IRQn, 5);
  LPUART_RTOS_Init(&handle, &t_handle, &lpuart_config);
#else
  lpuart_config_t config;
  /*
     * config.baudRate_Bps = 115200U;
     * config.parityMode = kLPUART_ParityDisabled;
     * config.stopBitCount = kLPUART_OneStopBit;
     * config.txFifoWatermark = 0;
     * config.rxFifoWatermark = 0;
     * config.enableTx = false;
     * config.enableRx = false;
     */
    LPUART_GetDefaultConfig(&config);
    config.baudRate_Bps = 115200U;
    config.enableTx     = true;
    config.enableRx     = true;
    LPUART_Init(LPUART1, &config, BOARD_DebugConsoleSrcFreq());
    /* Enable RX interrupt. */
    LPUART_EnableInterrupts(LPUART1, kLPUART_RxDataRegFullInterruptEnable);
    NVIC_SetPriority(LPUART1_IRQn, (1<<__NVIC_PRIO_BITS) - 2);
    EnableIRQ(LPUART1_IRQn);

	
#endif

}
//void app_send_debug_byte(uint8_t ch){
//  while(!(USART_FIFOSTAT_TXEMPTY_MASK & USART_GetStatusFlags(USART0)));
//  USART_WriteByte(USART0,ch); 
//}