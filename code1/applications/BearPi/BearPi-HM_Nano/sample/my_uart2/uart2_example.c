#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_adc.h"
#include "wifiiot_uart.h"

#define UART_TASK_STACK_SIZE 1024 * 6
#define UART_TASK_PRIO 25
#define UART_BUFF_SIZE 20

uint8_t clear_num = 0;
uint8_t dat_buff[6];
char* send_data1 = "LED_ON";
char* send_data2 = "LED_OFF";
char* clear_log = "";
char tx_buffer[48];  // 固定长度缓冲区（需与队列消息长度匹配）
extern volatile uint8_t flag_camera_ok;

osMutexId_t uart_mutex;
extern osEventFlagsId_t boardIndex_Event;		//太阳能板编号事件 0x1-->1号板子    0x2-->2号板子   0x8-->清扫完毕
extern osEventFlagsId_t controlMode_Event;      //控制模式事件	1自动模式	2自定义模式
extern osMessageQueueId_t clear_MsgQueue;       //清扫日志队列


static void UART2_Task(void)
{
    uint8_t uart_buff[UART_BUFF_SIZE] = {0};
    uint8_t *uart_buff_ptr = uart_buff;
    uint32_t ret;
    uint8_t control_mode = 0;

    WifiIotUartAttribute uart_attr = {
        .baudRate = 115200,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };


    osMutexAcquire(uart_mutex,osWaitForever);
    //Initialize uart driver
    ret = UartInit(WIFI_IOT_UART_IDX_2, &uart_attr, NULL);
    if (ret != WIFI_IOT_SUCCESS)
    {
        printf("Failed to init uart2! Err code = %d\n", ret);
        return;
    }else{
        printf("Success to init uart2! Err code = %d\n", ret);
    }
    osMutexRelease(uart_mutex);

    
    while (1)
    {
        control_mode = osEventFlagsGet(controlMode_Event);
        /* 自动模式 */
        if(control_mode == 0x1)
        {
            UartRead(WIFI_IOT_UART_IDX_2, uart_buff_ptr, UART_BUFF_SIZE);
            strncpy((char *)dat_buff, (const char *)uart_buff_ptr, 5);
            dat_buff[5] = '\0';
            //printf("Camera:%s\r\n",dat_buff);

            /* 来自相机(uart2) 识别结果:Drity/Clean   OK*/
            if(strstr((const char *)dat_buff,"Dir"))
            {
                /* 回复OK */
                UartWrite(WIFI_IOT_UART_IDX_2,(uint8_t*)"OK",2);
                /* 打开继电器 */
                GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 1);
                /* 发给STM32执行操作 */  
                osMutexAcquire(uart_mutex,osWaitForever);
                UartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)send_data1, strlen(send_data1));  
                osMutexRelease(uart_mutex);
            }else if(strstr((const char *)dat_buff,"Cle"))
            {
                UartWrite(WIFI_IOT_UART_IDX_2,(uint8_t*)"OK",2);
                 /* 关闭继电器 */
                 GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2, 0);

                osMutexAcquire(uart_mutex,osWaitForever);
                UartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)send_data2, strlen(send_data2));
                osMutexRelease(uart_mutex);
                
                clear_num = osEventFlagsGet(boardIndex_Event);
                if(clear_num == 0x1)
                    clear_log = "Board1 has been cleaned!";
                else
                    clear_log = "Board2 has been cleaned!";
                // 拷贝到缓冲区并确保终止符
                printf("clear_log: %s\r\n",clear_log);
                strncpy(tx_buffer, clear_log, sizeof(tx_buffer));
                tx_buffer[sizeof(tx_buffer) - 1] = '\0';
                osMessageQueuePut(clear_MsgQueue,&tx_buffer,0U,0U);
            }else if(strstr((const char *)dat_buff,"OK"))
            {
                flag_camera_ok = 1;
            }
        }else{
            /* 自定义模式 */

        }
        
        osDelay(2U);
    }
}

static void UART2_ExampleEntry(void)
{
    uart_mutex = osMutexNew(NULL);
    if(uart_mutex == NULL)
	{
		printf("Falied to create uart_mutex!\n");
	}
    
    osThreadAttr_t attr;

    attr.name = "UART2_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = UART_TASK_STACK_SIZE;
    attr.priority = UART_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)UART2_Task, NULL, &attr) == NULL)
    {
        printf("[ADCExample] Falied to create UART_Task!\n");
    }
}

SYS_RUN(UART2_ExampleEntry);