#include <stdio.h>

#include <unistd.h>
#include <string.h>
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "wifiiot_pwm.h"
#include "wifiiot_gpio.h"
#include "wifiiot_gpio_ex.h"

#define BIG_OFFSET_INTERVAL_TIME    2         /* 偏移大时增速间隔时间：20ms */
#define LOW_SPEED_INTERVAL_TIME     3         /* 低速时增速间隔时间：30ms */
#define Distinguish_OFFSET_TIME     40       /* 区分偏移大小的时间：400ms */
#define SMALL_OFFSET_INTERVAL_SPEED 300       /* 偏移量小时每次增速200 */
#define BIG_OFFSET_INTERVAL_SPEED   500       /* 偏移量大时每次增速400 */
#define SMALL_OFFSET_MAX_SPEED      8000
#define BIG_OFFSET_MAX_SPEED        13000

extern osEventFlagsId_t motor_id;
extern osMutexId_t track_mutex;
extern osEventFlagsId_t track_id;
extern osMessageQueueId_t clear_MsgQueue;

uint32_t motor_control = 0;             /* 0代表循迹控制    1代表清扫设备控制 */

static void MotorTask(void)
{
    uint32_t motor_flag = 0;
    uint32_t motor_status = 0;
    uint32_t track_status = 0;
    uint32_t now_speed_time = 0;
    uint32_t last_speed_time = 0;
    uint32_t now_time = 0;
    uint32_t last_time = 0;
    uint32_t speed_left = 5000;
    uint32_t speed_right = 5000;
    uint32_t flag_synchronize_time = 1;     /* 刚偏移时需要同步now_time和last_time */
    uint32_t cnt = 0;
    char* clear_log = "";
    char tx_buffer[48];  // 固定长度缓冲区（需与队列消息长度匹配）
    //初始化GPIO
    GpioInit();

    //设置GPIO_10和GPIO_12引脚复用功能为PWM
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_IO_FUNC_GPIO_10_PWM1_OUT); /* 左轮 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_IO_FUNC_GPIO_12_PWM3_OUT); /* 右轮 */


    //设置GPIO_10和GPIO_12引脚为输出模式
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_10, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_12, WIFI_IOT_GPIO_DIR_OUT);

    //初始化PWM1和PWM3端口
    PwmInit(WIFI_IOT_PWM_PORT_PWM1);
    PwmInit(WIFI_IOT_PWM_PORT_PWM3);

    while (1)
    {
        //输出PWM波，不等待，以便根据循迹情况随时改变状态
        motor_flag = osEventFlagsWait(motor_id, 0x1, osFlagsWaitAny, 0);
        if(motor_flag & 0x1)
        {
            motor_status = !motor_status;
            //printf("motor_status: %d\n", motor_status);
        }
        /* 轮子是否转动 */
        if(motor_status)
        {
            /* 循迹控制 */
            if(!motor_control)
            {
                /* 通过循迹情况控制电机转动 */
                /* 获取互斥锁，防止和TrackTask竞争 */
                osMutexAcquire(track_mutex,osWaitForever);
                
                track_status = osEventFlagsGet(track_id);
                osMutexRelease(track_mutex);
                switch (track_status)
                {
                    case 2: /* 正常 */
                    {
                        /* 回到正轨时，重置数据 */
                        speed_left = 5000;
                        speed_right = 5000;
                        flag_synchronize_time = 1;
                        PwmStart(WIFI_IOT_PWM_PORT_PWM1, 5000, 40000);
                        PwmStart(WIFI_IOT_PWM_PORT_PWM3, 5000, 40000);
                    }break;
                    
                    case 3: /* 微微偏左 */
                    {
                        PwmStart(WIFI_IOT_PWM_PORT_PWM3, 5000, 40000);
                        PwmStart(WIFI_IOT_PWM_PORT_PWM1, 3000, 40000);
                    }break;
                    case 1:
                    {
                        now_speed_time = osKernelGetTickCount();
                        now_time = now_speed_time;
                        if(flag_synchronize_time)
                        {
                            flag_synchronize_time = 0;
                            last_time = now_time;           /* 下次同步就是回到正轨时 */
                        }
                        /* 微微偏左 */
                        if(now_time - last_time < Distinguish_OFFSET_TIME)
                        {
                            PwmStart(WIFI_IOT_PWM_PORT_PWM1, 4000, 40000);
                            /* 每间隔100ms增加一级小速度 */
                            if(now_speed_time - last_speed_time >= LOW_SPEED_INTERVAL_TIME)
                            {
                                speed_left += SMALL_OFFSET_INTERVAL_SPEED;
                                if(speed_left >= SMALL_OFFSET_MAX_SPEED)
                                {
                                    speed_left = SMALL_OFFSET_MAX_SPEED;
                                }
                                last_speed_time = now_speed_time;
                                //printf("small offset speed_left: %d\n",speed_left);
                            }
                        }else{
                            PwmStart(WIFI_IOT_PWM_PORT_PWM1, 3000, 40000);
                            /* 较多偏左  每间隔20ms增加一级大速度  */
                            if(now_speed_time - last_speed_time >= BIG_OFFSET_INTERVAL_TIME)
                            {
                                speed_left += BIG_OFFSET_INTERVAL_SPEED;
                                if(speed_left >= BIG_OFFSET_MAX_SPEED)
                                {
                                    speed_left = BIG_OFFSET_MAX_SPEED;
                                }
                                last_speed_time = now_speed_time;
                                //printf("big offset speed_left: %d\n",speed_left);
                            }
                        }
                        PwmStart(WIFI_IOT_PWM_PORT_PWM3, speed_left, 40000);
                    }break;
                    case 6: /* 微微偏右 */
                    {
                        PwmStart(WIFI_IOT_PWM_PORT_PWM3, 3000, 40000);
                        PwmStart(WIFI_IOT_PWM_PORT_PWM1, 5000, 40000);
                    }break;
                    case 4: /* 较多偏右 */
                    {
                        now_speed_time = osKernelGetTickCount();
                        now_time = now_speed_time;
                        if(flag_synchronize_time)
                        {
                            flag_synchronize_time = 0;
                            last_time = now_time;           /* 下次同步就是回到正轨时 */
                        }
                        /* 微微偏右 */
                        if(now_time - last_time < Distinguish_OFFSET_TIME)
                        {
                            PwmStart(WIFI_IOT_PWM_PORT_PWM3, 3000, 40000);
                            /* 每间隔20ms增加一级小速度 */
                            if(now_speed_time - last_speed_time >= LOW_SPEED_INTERVAL_TIME)
                            {
                                speed_right += SMALL_OFFSET_INTERVAL_SPEED;
                                if(speed_right >= SMALL_OFFSET_MAX_SPEED)
                                {
                                    speed_right = SMALL_OFFSET_MAX_SPEED;
                                }
                                last_speed_time = now_speed_time;
                                //printf("small offset speed_right: %d\n",speed_right);
                            }
                        }else{
                            PwmStart(WIFI_IOT_PWM_PORT_PWM3, 3000, 40000);
                            /* 较多偏右  每间隔20ms增加一级大速度  */
                            if(now_speed_time - last_speed_time >= BIG_OFFSET_INTERVAL_TIME)
                            {
                                speed_right += BIG_OFFSET_INTERVAL_SPEED;
                                if(speed_right >= BIG_OFFSET_MAX_SPEED)
                                {
                                    speed_right = BIG_OFFSET_MAX_SPEED;
                                }
                                last_speed_time = now_speed_time;
                                //printf("big offset speed_right: %d\n",speed_right);
                            }
                        }
                        PwmStart(WIFI_IOT_PWM_PORT_PWM1, speed_right, 40000);
                    }break;
                    case 7:/* 检测到太阳能板，停下，开始清理 */
                    {
                        ++cnt;
                        PwmStop(WIFI_IOT_PWM_PORT_PWM1);
                        PwmStop(WIFI_IOT_PWM_PORT_PWM3);
                        osDelay(100U);
                       
                        /* 模拟清扫-->向队列发消息上云 */
                        clear_log = (cnt % 2)?"Board 1 has been cleaned!":"Board 2 has been cleaned!";
                        // 拷贝到缓冲区并确保终止符
                        strncpy(tx_buffer, clear_log, sizeof(tx_buffer));
                        tx_buffer[sizeof(tx_buffer) - 1] = '\0';
                        osMessageQueuePut(clear_MsgQueue,&tx_buffer,0U,0U);
                        /* 切换到清扫设备控制 */
                        motor_control = 1;
                        
                    }break;
                default:
                    break;
                }
            }else{
                /* 清扫设备控制 */
                PwmStart(WIFI_IOT_PWM_PORT_PWM1, 5000, 40000);
                PwmStart(WIFI_IOT_PWM_PORT_PWM3, 5000, 40000);
                osDelay(100U);
                /* 切换到循迹控制 */
                motor_control = 0;
            }
        }else{
            PwmStop(WIFI_IOT_PWM_PORT_PWM1);
            PwmStop(WIFI_IOT_PWM_PORT_PWM3);
        }
        usleep(10);

        // PwmStart(WIFI_IOT_PWM_PORT_PWM4, 3000, 40000);

        // osDelay(300U);
        // PwmStart(WIFI_IOT_PWM_PORT_PWM4, 1000, 40000);
        // osDelay(100U);
        // PwmStart(WIFI_IOT_PWM_PORT_PWM4, 2000, 40000);
        // osDelay(100U);
        // PwmStart(WIFI_IOT_PWM_PORT_PWM4, 3000, 40000);
        // osDelay(100U);
        // PwmStart(WIFI_IOT_PWM_PORT_PWM4, 4000, 40000);
        // osDelay(100U);
        // PwmStart(WIFI_IOT_PWM_PORT_PWM4, 5000, 40000);
    }
}

static void MotorExampleEntry(void)
{
    osThreadAttr_t attr;

    attr.name = "MotorTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 1024;
    attr.priority = 24;

    if (osThreadNew((osThreadFunc_t)MotorTask, NULL, &attr) == NULL)
    {
        printf("Falied to create MotorTask!\n");
    }
}

APP_FEATURE_INIT(MotorExampleEntry);