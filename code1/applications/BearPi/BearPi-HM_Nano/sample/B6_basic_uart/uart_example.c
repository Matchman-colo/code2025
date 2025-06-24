/*
 * Copyright (c) 2020 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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

#define UART_TASK_STACK_SIZE 1024 * 8
#define UART_TASK_PRIO 25
#define UART_BUFF_SIZE 20

uint8_t dat_buffer[8];
uint8_t rec_camera_buffer[3];
uint8_t* rec_camera_ptr = rec_camera_buffer;
volatile uint8_t flag_camera_ok = 0;

char* send_data3 = "location";
extern osMutexId_t uart_mutex;
osEventFlagsId_t boardIndex_Event;		//太阳能板编号事件 0x1-->1号板子    0x2-->2号板子  

static void UART_Task(void)
{
    uint8_t uart_buff[UART_BUFF_SIZE] = {0};
    uint8_t *uart_buff_ptr = uart_buff;
    uint32_t ret;

    WifiIotUartAttribute uart_attr = {
        .baudRate = 115200,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };

    //Initialize uart driver
    osMutexAcquire(uart_mutex,osWaitForever);
    ret = UartInit(WIFI_IOT_UART_IDX_1, &uart_attr, NULL);
    if (ret != WIFI_IOT_SUCCESS)
    {
        printf("Failed to init uart1! Err code = %d\n", ret);
        return;
    }else{
        printf("Success to init uart1!", ret);
    }
    osMutexRelease(uart_mutex);

    uint16_t uart1_count = 0;
    while (1)
    {

        uart1_count = UartRead(WIFI_IOT_UART_IDX_1, uart_buff_ptr, UART_BUFF_SIZE);
        if(uart1_count >= 7)
        {
            strncpy((char *)dat_buffer, (const char *)uart_buff_ptr, 7);
            dat_buffer[7] = '\0';
            printf("STM32:%s\r\n",dat_buffer);

            if(strstr((const char *)dat_buffer,"ONE"))   /* 来自STM32(uart1) 到达地点 或 清扫查询*/
            {
                osEventFlagsClear(boardIndex_Event, 0x2);
                osEventFlagsSet(boardIndex_Event,0x1);
            }else if(strstr((const char *)dat_buffer,"TWO"))
            {
                osEventFlagsClear(boardIndex_Event, 0x1);
                osEventFlagsSet(boardIndex_Event,0x2);
            }

            if(strstr((const char *)dat_buffer,"Que"))
            {
                do
                {
                    UartWrite(WIFI_IOT_UART_IDX_2, (unsigned char *)send_data3, strlen(send_data3));
                    osDelay(10U);
                }while(!flag_camera_ok);
            }
        }
        osDelay(2U);
    }
}

static void UART_ExampleEntry(void)
{
	boardIndex_Event = osEventFlagsNew(NULL);
    if(boardIndex_Event == NULL)
    {
        printf("Falied to create boardIndex_Event!\n");
    }
    osThreadAttr_t attr;

    attr.name = "UART_Task";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = UART_TASK_STACK_SIZE;
    attr.priority = UART_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)UART_Task, NULL, &attr) == NULL)
    {
        printf("[ADCExample] Falied to create UART_Task!\n");
    }
}

SYS_RUN(UART_ExampleEntry);