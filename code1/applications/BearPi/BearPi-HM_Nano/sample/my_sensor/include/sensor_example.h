#ifndef __BH1750_H
#define __BH1750_H
 
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_errno.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"
#include "wifiiot_i2c.h"
#include "wifiiot_i2c_ex.h"    
#include "wifiiot_uart.h"


typedef struct {
	float temperature;
	float humidity;
	int light;
} envir_data_t;


void Read_SensorData(envir_data_t* dat);

#endif
