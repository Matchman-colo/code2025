#include "ohos_init.h"
#include "unistd.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

void led_example(void)
{
    GpioInit();
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_IO_FUNC_GPIO_2_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_2,WIFI_IOT_GPIO_DIR_OUT);
    for (unsigned char i = 0; i < 10; i++)
    {
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,1);
        usleep(500000);
        GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,0);
        usleep(500000);
    }
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_2,1);

}
APP_FEATURE_INIT(led_example);