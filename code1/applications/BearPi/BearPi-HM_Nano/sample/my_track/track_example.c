#include <stdio.h>
#include <unistd.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

osEventFlagsId_t track_id;
osMutexId_t track_mutex;

static void TrackTask(void)
{
    uint32_t R1_Value = 0;
    uint32_t M_Value = 0;
    uint32_t L1_Value = 0;
    //初始化GPIO
    GpioInit();

    /* R1 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_FUNC_GPIO_14_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_14, WIFI_IOT_IO_PULL_UP);
    /* M */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_FUNC_GPIO_7_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_7, WIFI_IOT_IO_PULL_UP);
    /* L1 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_FUNC_GPIO_8_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_GPIO_DIR_IN);
    IoSetPull(WIFI_IOT_IO_NAME_GPIO_8, WIFI_IOT_IO_PULL_UP);

    while (1)
    {
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_14, &R1_Value);
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_7, &M_Value);
        GpioGetInputVal(WIFI_IOT_IO_NAME_GPIO_8, &L1_Value);

        /* 获取互斥锁，防止和电机判断处竞争 */
        osMutexAcquire(track_mutex, osWaitForever);
        if(L1_Value)
        {
            osEventFlagsSet(track_id,0x4);
        }else
        {
            osEventFlagsClear(track_id, 0x4);
        }

        if(M_Value)
        {
            osEventFlagsSet(track_id,0x2);
        }else
        {
            osEventFlagsClear(track_id, 0x2); 
        }

        if(R1_Value)
        {
            osEventFlagsSet(track_id,0x1);
        }else
        {
            osEventFlagsClear(track_id, 0x1);
        }
        osMutexRelease(track_mutex);
        osDelay(2U);
    }
}

static void TrackExampleEntry(void)
{
    track_id = osEventFlagsNew(NULL);
    if(track_id == NULL)
    {
        printf("Falied to create track_id!\n");
    }
    track_mutex = osMutexNew(NULL);
    if (track_mutex == NULL)
    {
      printf("Falied to create track_mutex!\n");
    }

    osThreadAttr_t attr;

    attr.name = "TrackTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = 24;

    if (osThreadNew((osThreadFunc_t)TrackTask, NULL, &attr) == NULL)
    {
        printf("Falied to create TrackTask!\n");
    }
}

APP_FEATURE_INIT(TrackExampleEntry);