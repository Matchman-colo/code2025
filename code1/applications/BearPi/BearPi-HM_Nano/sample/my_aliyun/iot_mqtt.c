/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - clarifications and/or documentation extension
 *******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifi_connect.h"
#include "MQTTClient.h"
#include <memory.h>
#include <signal.h>
#include <sys/time.h>
#include <cJSON.h>
#include "sensor_example.h"

#define EXAMPLE_PRODUCT_KEY			"k29870psNpB"
#define EXAMPLE_DEVICE_NAME			"ESP8266-dev"
#define EXAMPLE_DEVICE_SECRET       "cfdf213036aea759750deb07e90557bd"
#define MSGQUEUE_OBJECTS 16

const char *subTopicModeContorl = "/sys/"EXAMPLE_PRODUCT_KEY"/"EXAMPLE_DEVICE_NAME"/thing/service/property/set";
const char *pubTopic = "/sys/"EXAMPLE_PRODUCT_KEY"/"EXAMPLE_DEVICE_NAME"/thing/event/property/post";
const char *eventPubTopic = "/sys/"EXAMPLE_PRODUCT_KEY"/"EXAMPLE_DEVICE_NAME"/thing/event/ClearLog/post";
const char *userDefinePubTopic = "/sys/"EXAMPLE_PRODUCT_KEY"/"EXAMPLE_DEVICE_NAME"/thing/event/Define/post";

/* 枚举值：0代表命令下发，1代表数据上报 */
typedef enum
{
	en_msg_cmd = 0,
	en_msg_report,
} en_msg_type_t;

/* 命令下发的格式 */
typedef struct
{
	char *request_id;
	char *payload;
} cmd_t;

/* 数据上报的格式 */
typedef struct
{
	int lum;
	int temp;
	int hum;
} report_t;

/* 统一应用消息，要么命令，要么数据 */
typedef struct
{
	en_msg_type_t msg_type;
	union
	{
		cmd_t cmd;
		report_t report;
	} msg;
} app_msg_t;

/* 应用程序控制块，用于存储设备状态，如连接状态、LED 和电机开关 */
typedef struct
{
	int connected;
	int led;
	int motor;
} app_cb_t;
static app_cb_t g_app_cb;

char* clear_msg;
char json_msg[256];						//JSON数据
char* control_mode0 = "COM0";
char* control_mode1 = "COM1";
char* control_switch0 = "SWI0"; 
char* control_switch1 = "SWI1"; 
envir_data_t device_state = {0, 0, 0};  // 当前设备状态
osMessageQueueId_t mid_MsgQueue;		//传感器数据队列
osMessageQueueId_t clear_MsgQueue;		//清扫日志队列
osMutexId_t mqtt_Mutex;
extern osMutexId_t uart_mutex;			//避免与相机通过uart1向STM32发送消息起冲突
osEventFlagsId_t controlMode_Event;		//控制模式事件	1自动模式	2自定义

MQTTClient c;
app_msg_t app_msg;


/* declare the external function aiotMqttSign() */
extern int aiotMqttSign(const char *productKey, const char *deviceName, const char *deviceSecret, 
						char clientId[150], char username[65], char password[65]);

void cfinish(int sig)
{
	signal(SIGINT, NULL);
	printf("sig is %d\r\n",sig);
}


void userDefineArrived(MessageData* md)
{
	MQTTMessage* message = md->message;

	cJSON *root = cJSON_Parse((char *)message->payload);
    if (root == NULL) {
        printf("Failed to parse JSON!\n");
        return;
    }

    // 获取 value 对象
    cJSON *value = cJSON_GetObjectItem(root, "value");
    if (value == NULL) {
        printf("Failed to get 'items' object!\n");
        cJSON_Delete(root);
        return;
    }else{
		char *value_str = cJSON_Print(value); // 将对象转为格式化字符串
		printf("value content:\n%s\n", value_str);
		free(value_str); // 释放内存
	}
}

void messageModeContorlArrived(MessageData* md)
{
	/* 命令下发，加入队列 */
	MQTTMessage* message = md->message;

    // 解析 JSON 字符串
    cJSON *root = cJSON_Parse((char *)message->payload);
    if (root == NULL) {
        printf("Failed to parse JSON!\n");
        return;
    }
    // 获取 items 对象
    cJSON *items = cJSON_GetObjectItem(root, "items");
    if (items == NULL) {
        printf("Failed to get 'items' object!\n");
        cJSON_Delete(root);
        return;
    }
    // 获取 mode 对象
    cJSON *mode = cJSON_GetObjectItem(items, "mode");
    if (mode == NULL) {
        printf("Failed to get 'mode' object!\n");
        cJSON_Delete(root);
        return;
    }
    // 获取 value 值
    cJSON *value = cJSON_GetObjectItem(mode, "value");
    if (value == NULL) {
        printf("Failed to get 'value' field!\n");
        cJSON_Delete(root);
        return;
    }
    // 检查 value 是否为数字
    if (cJSON_IsNumber(value)) {
        int value_int = value->valueint;  // 获取整数值
        printf("Value: %d\n", value_int);
	
		osMutexAcquire(uart_mutex,osWaitForever);
		switch(value_int)
		{
			case 0:{/* 自动模式 */
				LOS_TaskLock();
				osEventFlagsClear(controlMode_Event,0x2);
				osEventFlagsSet(controlMode_Event,0x1);
				LOS_TaskUnlock();
				UartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)control_mode0, strlen(control_mode0));
			}break;
			case 1:{/* 自定义模式 */
				LOS_TaskLock();
				osEventFlagsClear(controlMode_Event,0x1);
				osEventFlagsSet(controlMode_Event,0x2);
				printf("controlMode_Event: %d\r\n",osEventFlagsGet(controlMode_Event));
				LOS_TaskUnlock();
				UartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)control_mode1, strlen(control_mode1));
				UartWrite(WIFI_IOT_UART_IDX_2, (unsigned char *)control_mode1, strlen(control_mode1));
			}break;
			case 2:{/* 打开 */
				UartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)control_switch0, strlen(control_switch0));
			}break;
			case 3:{/* 关闭 */
				UartWrite(WIFI_IOT_UART_IDX_1, (unsigned char *)control_switch1, strlen(control_switch1));
			}break;
			default:
				break;
		}
		osMutexRelease(uart_mutex);
    } else {
        printf("'value' is invalid!\n");
    }

    // 释放 JSON 对象
    cJSON_Delete(root);
}

osStatus_t result = 0;
/* 将传感器数据封装为 MQTT 消息并发送 */
static void deal_report_msg(report_t *report)
{	
	snprintf(json_msg, sizeof(json_msg), 
	"{\"method\":\"thing.service.property.set\",\"id\":\"1241570014\",\"params\":{\"temp\":%d,\"humi\":%d,\"ligh\":%d},\"version\":\"1.0.0\"}", 
	report->temp, 
	report->hum,
	report->lum); 
	MQTTMessage msg = {
		QOS1, 
		0,
		0,
		0,
		json_msg,
		strlen(json_msg)
	};
	// 加锁保护
	osMutexAcquire(mqtt_Mutex, osWaitForever);
	result = MQTTPublish(&c, pubTopic, &msg);
	osMutexRelease(mqtt_Mutex);
	//printf("result: %d, JSON data: %s\n",result,json_msg);
    return;
}

static void deal_clean_msg(char* clear_message)
{	
	snprintf(json_msg, sizeof(json_msg), 
	"{\"id\":\"1740921591834\",\"version\":\"1.0\",\"params\":{\"value\":{\"ClearLog\":\"%s\"}},\"method\":\"thing.event.ClearLog.post\"}", 
	clear_message); 
	MQTTMessage msg = {
		QOS1, 
		0,
		0,
		0,
		json_msg,
		strlen(json_msg)
	};
	// 加锁保护
	osMutexAcquire(mqtt_Mutex, osWaitForever);
	result = MQTTPublish(&c, eventPubTopic, &msg);
	osMutexRelease(mqtt_Mutex);
	//printf("result: %d, JSON data: %s\n",result,json_msg);
    return;
}


 /* main function */
static int task_main_entry(void)
{
	/* setup the buffer, it must big enough for aliyun IoT platform */
	int rc = 0;
	unsigned char buf[2048];
	unsigned char readbuf[2048];

	Network n;
	char *host = EXAMPLE_PRODUCT_KEY".iot-as-mqtt.cn-shanghai.aliyuncs.com";
	short port = 443;

	/* invoke aiotMqttSign to generate mqtt connect parameters */
	char clientId[150] = {0};
	char username[65] = {0};
	char password[65] = {0};

	WifiConnect("cdut-yb","cdutyb218");
	printf("Starting ...\n");

	if ((rc = aiotMqttSign(EXAMPLE_PRODUCT_KEY, EXAMPLE_DEVICE_NAME, EXAMPLE_DEVICE_SECRET, clientId, username, password) < 0)) {
		printf("aiotMqttSign -%0x4x\n", -rc);
		return -1;
	}

	signal(SIGINT, cfinish);
	signal(SIGTERM, cfinish);

	/* network init and establish network to aliyun IoT platform */
	NetworkInit(&n);
	rc = NetworkConnect(&n, host, port);
	printf("NetworkConnect %d\n", rc);

	/* init mqtt client */
	MQTTClientInit(&c, &n, 1000, buf, sizeof(buf), readbuf, sizeof(readbuf));

	/* set the default message handler */
	//c.defaultMessageHandler = messageArrived;

	/* set mqtt connect parameter */
	MQTTPacket_connectData data = MQTTPacket_connectData_initializer;       
	data.willFlag = 0;
	data.MQTTVersion = 3;
	data.clientID.cstring = clientId;
	data.username.cstring = username;
	data.password.cstring = password;
	data.keepAliveInterval = 60;
	data.cleansession = 1;
	printf("Connecting to %s %d\n", host, port);

	rc = MQTTConnect(&c, &data);
	if(!rc)
	{
		printf("Connect aliyun IoT Cloud Success!\n");
		GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 0);
	}else{
		printf("Connect aliyun IoT Cloud Fail!\n");
	}

	rc = MQTTSubscribe(&c, subTopicModeContorl, 1, messageModeContorlArrived);
	printf("Subscribe to subTopicModeContorl:%s------%d\n", subTopicModeContorl,rc);
	
	rc = MQTTSubscribe(&c, userDefinePubTopic, 1, userDefineArrived);
	printf("Subscribe to userDefinePubTopic:%s------%d\n", userDefinePubTopic,rc);
	char rx_buffer[48];  // 与发送端缓冲区大小一致
	while (1)
	{
		/* 上报属性值和监控下发指令 */
		result = osMessageQueueGet(mid_MsgQueue, &app_msg, NULL, 0);
		if (result == osOK)
		{
			switch (app_msg.msg_type)
			{
				case en_msg_cmd:
					//deal_cmd_msg(&app_msg->msg.cmd);
					break;
				case en_msg_report:
				{
					deal_report_msg(&(app_msg.msg.report));
					break;
				}
				default:
					break;
			}
		}

		/* 上报清扫日志 */
		result = osMessageQueueGet(clear_MsgQueue, &rx_buffer, NULL, 0);
		if (result == osOK)
		{
			deal_clean_msg(rx_buffer);
		}
		MQTTYield(&c, 1000);
	}
	return 0;
}

static int task_sensor_entry(void)
{
	app_msg_t app_msg;
	device_state.humidity = 63;
	device_state.light = 132;
	device_state.temperature = 21;
	while (1)
	{
		Read_SensorData(&device_state);
		app_msg.msg_type = en_msg_report;
		app_msg.msg.report.hum = (int)device_state.humidity;
		app_msg.msg.report.lum = (int)device_state.light;
		app_msg.msg.report.temp = (int)device_state.temperature;
		osMessageQueuePut(mid_MsgQueue, &app_msg, 0U, 0U);
		osDelay(100U);
	}
	return 0;
}

static void MQTT_Demo(void)
{
	mid_MsgQueue = osMessageQueueNew(MSGQUEUE_OBJECTS, 20, NULL);
	clear_MsgQueue = osMessageQueueNew(10, 48, NULL);
	mqtt_Mutex = osMutexNew(NULL);
	LOS_TaskLock();  // 锁定任务调度，创建controlMode_Event并设置初值期间，不允许任务切换
	controlMode_Event = osEventFlagsNew(NULL);
	osEventFlagsSet(controlMode_Event,0x1);					/* 默认模式是自动模式 */
	LOS_TaskUnlock();  // 解锁任务调度

	if (controlMode_Event == NULL)
    {
        printf("Falied to create controlMode_Event!\n");
    }
    if (mid_MsgQueue == NULL)
    {
        printf("Falied to create mid_MsgQueue!\n");
    }
	if (clear_MsgQueue == NULL)
    {
        printf("Falied to create clear_MsgQueue!\n");
    }
	if(mqtt_Mutex == NULL)
	{
		printf("Falied to create mqtt_Mutex!\n");
	}

    osThreadAttr_t attr;
    attr.name = "task_main_entry";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024 * 12;
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)task_main_entry, NULL, &attr) == NULL) {
        printf("[MQTT_Demo] Falied to create task_main_entry!\n");
    }

	attr.stack_size = 2048;
    attr.priority = 25;
    attr.name = "task_sensor_entry";
    if (osThreadNew((osThreadFunc_t)task_sensor_entry, NULL, &attr) == NULL)
    {
        printf("Falied to create task_sensor_entry!\n");
    }
}

SYS_RUN(MQTT_Demo);
