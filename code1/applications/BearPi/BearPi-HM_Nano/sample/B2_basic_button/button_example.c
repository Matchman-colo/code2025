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
#include <unistd.h>
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_uart.h"

char* exit_camera = "exit";

static void F1_Pressed(char *arg)
{
    (void)arg;
    //static uint32_t last_time = 0;
    //uint32_t now = osKernelGetTickCount(); // 获取当前系统时间

    // 防抖：20ms 内只处理一次
    //  if (now - last_time > 2) {
    //     last_time = now;
    // }
    UartWrite(WIFI_IOT_UART_IDX_2, (unsigned char *)exit_camera, strlen(exit_camera));   
    printf("F1 pressed."); 
}

static void ButtonExampleEntry(void)
{
    // motor_id = osEventFlagsNew(NULL);
    // if(motor_id == NULL)
    // {
    //     printf("Falied to create motor_id!\n");
    // }
    /* 整个项目设置一次即可 */
    GpioInit();

    /* 与串口2引脚冲突，用不了 */
    //初始化F1按键，设置为下降沿触发中断
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_FUNC_GPIO_11_GPIO);

    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_IO_PULL_UP);
    GpioRegisterIsrFunc(WIFI_IOT_IO_NAME_GPIO_11, WIFI_IOT_INT_TYPE_EDGE, WIFI_IOT_GPIO_EDGE_FALL_LEVEL_LOW, F1_Pressed, NULL);

    //初始化F2按键，设置为下降沿触发中断
    // IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_GPIO);
}

SYS_RUN(ButtonExampleEntry);