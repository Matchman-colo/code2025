#include "sensor_example.h"

#define I2C_TASK_STACK_SIZE 1024 * 8
#define I2C_TASK_PRIO 25

#define WRITE_BIT           0x00
#define READ_BIT            0x01

#define BH1750_SLAVE_ADDR   0x23 // 从机地址
#define BH1750_PWR_DOWN     0x00 // 关闭模块
#define BH1750_PWR_ON       0x01 // 打开模块等待测量指令
#define BH1750_RST          0x07 // 重置数据寄存器值在PowerOn模式下有效
#define BH1750_CON_H        0x10 // 连续高分辨率模式，1lx，120ms
#define BH1750_CON_H2       0x11 // 连续高分辨率模式，0.5lx，120ms
#define BH1750_CON_L        0x13 // 连续低分辨率模式，4lx，16ms
#define BH1750_ONE_H        0x20 // 一次高分辨率模式，1lx，120ms，测量后模块转到PowerDown模式
#define BH1750_ONE_H2       0x21 // 一次高分辨率模式，0.5lx，120ms，测量后模块转到PowerDown模式
#define BH1750_ONE_L        0x23 // 一次低分辨率模式，4lx，16ms，测量后模块转到PowerDown模式

#define AHT20_ADDRESS 0x38

#define RELAY_PIN                   WIFI_IOT_IO_NAME_GPIO_2
#define RELAY_PIN_FUNC              WIFI_IOT_IO_FUNC_GPIO_2_GPIO

osMutexId_t data_mutex;
envir_data_t data_buffer;

/**
 @brief I2C写数据函数
 @param slaveAddr -[in] 从设备地址
 @param regAddr -[in] 寄存器地址
 @param pData -[in] 写入数据
 @param dataLen -[in] 写入数据长度
 @return 错误码
*/
int I2C_WriteData(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pData, uint16_t dataLen)
{
    int ret;
    WifiIotI2cData i2c_data = {0};

    if(0 != regAddr)
    {
        i2c_data.sendBuf = &regAddr;
        i2c_data.sendLen = 1;
        ret = I2cWrite(WIFI_IOT_I2C_IDX_1, (slaveAddr << 1) | WRITE_BIT, &i2c_data);
        if(ret != 0)
        {
            //printf("===== Error: I2C write status1 = 0x%x! =====\r\n", ret);
            return 0;
        }
    }

    i2c_data.sendBuf = pData;
    i2c_data.sendLen = dataLen;
    ret = I2cWrite(WIFI_IOT_I2C_IDX_1, (slaveAddr << 1) | WRITE_BIT, &i2c_data);
    if(ret != 0)
    {
        //printf("===== Error: I2C write status1 = 0x%x! =====\r\n", ret);
        return 0;
    }

    return 1;
}

/**
 @brief I2C读数据函数
 @param slaveAddr -[in] 从设备地址
 @param regAddr -[in] 寄存器地址
 @param pData -[in] 读出数据
 @param dataLen -[in] 读出数据长度
 @return 错误码
*/
int I2C_ReadData(uint8_t slaveAddr, uint8_t regAddr, uint8_t *pData, uint16_t dataLen)
{
    int ret;
    WifiIotI2cData i2c_data = {0};

    if(0 != regAddr)
    {
        i2c_data.sendBuf = &regAddr;
        i2c_data.sendLen = 1;
        ret = I2cWrite(WIFI_IOT_I2C_IDX_1, (slaveAddr << 1) | WRITE_BIT, &i2c_data);
        if(ret != 0)
        {
            //printf("===== Error: I2C write status = 0x%x! =====\r\n", ret);
            return 0;
        }
    }

    i2c_data.receiveBuf = pData;
    i2c_data.receiveLen = dataLen;
    ret = I2cRead(WIFI_IOT_I2C_IDX_1, (slaveAddr << 1) | READ_BIT, &i2c_data);
    if(ret != 0)
    {
        //printf("===== Error: I2C read status = 0x%x! =====\r\n", ret);
        return 0;
    }

    return 1;
}

void AHT20_Init(void) //初始化
{
	 uint8_t readBuffer;
	 osDelay(4U);
	 I2C_ReadData(AHT20_ADDRESS, 0, &readBuffer, 1);
	 if ((readBuffer & 0x08) == 0x00){
		 uint8_t sendBuffer[3] = {0xBE,0x08,0x00};
		 //HAL_I2C_Master_Transmit(&hi2c2, AHT20_ADDRESS, sendBuffer, 3, HAL_MAX_DELAY);
         I2C_WriteData(AHT20_ADDRESS, 0, sendBuffer, 3);
	 }
}
 
void AHT20_Read(float *Temperature, float *Humidity)//读取数据并进行转换
{
	 uint8_t sendBuffer[3] = {0xAC,0x33,0x00};
	 uint8_t readBuffer[6];
    
     I2C_WriteData(AHT20_ADDRESS,0,sendBuffer,3);
     osDelay(7U);
     I2C_ReadData(AHT20_ADDRESS,0,readBuffer,6);
 
	 if((readBuffer[0] & 0x80) == 0x00){
		 uint32_t data = 0;
		 data = ((uint32_t)readBuffer[3] >> 4) + ((uint32_t)readBuffer[2] << 4) + ((uint32_t)readBuffer[1] << 12);
		 *Humidity = data *100.0f / (1 << 20);
 
		 data = (((uint32_t)readBuffer[3] & 0x0F) << 16) + ((uint32_t)readBuffer[4] << 8) + (uint32_t)readBuffer[5];
		 *Temperature = data * 200.0f /(1<<20) - 50;
	 }
}


static void SensorTask(void)
{
    int ret;
    uint8_t sensor_data[2] = {0};
    uint8_t data;
    uint32_t Light = 0;
    float Temperature = 0;
    float Humidity = 0;

    GpioInit();
    //GPIO_0复用为I2C1_SDA
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_0, WIFI_IOT_IO_FUNC_GPIO_0_I2C1_SDA);
    //GPIO_1复用为I2C1_SCL
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_1, WIFI_IOT_IO_FUNC_GPIO_1_I2C1_SCL);
    //baudrate: 400kbps
    I2cInit(WIFI_IOT_I2C_IDX_1, 400000);
    I2cSetBaudrate(WIFI_IOT_I2C_IDX_1, 400000);

    /* 设置继电器IO口 */
    IoSetFunc(RELAY_PIN, RELAY_PIN_FUNC);
    GpioSetDir(RELAY_PIN, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(RELAY_PIN, 0);

    /* 阿里云连接成功 */
    IoSetFunc(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_IO_FUNC_GPIO_9_GPIO);
    GpioSetDir(WIFI_IOT_IO_NAME_GPIO_9, WIFI_IOT_GPIO_DIR_OUT);
    GpioSetOutputVal(WIFI_IOT_IO_NAME_GPIO_9, 1);

    while (1)
    {
        data = BH1750_PWR_ON;              // 发送启动命令
        I2C_WriteData(BH1750_SLAVE_ADDR, 0, &data, 1);
        data = BH1750_ONE_L;               // 设置一次低分辨率模式，测量后模块转到PowerDown模式
        I2C_WriteData(BH1750_SLAVE_ADDR, 0, &data, 1);
        usleep(30000);                     // 设置完成后要有一段延迟
        ret = I2C_ReadData(BH1750_SLAVE_ADDR, 0, sensor_data, 2);
        Light = (sensor_data[0] << 8 | sensor_data[1]) / 1.2;
        if(ret == 0) 
        {
            printf("I2C Error");
        } 

        AHT20_Init();
        AHT20_Read(&Temperature, &Humidity);
        //printf("Temperature: %.1f°C      Humidity: %.2f%\n      Light: %dLux\n",Temperature,Humidity,Light);

        osMutexAcquire(data_mutex,osWaitForever);
        data_buffer.temperature = Temperature;
        data_buffer.humidity = Humidity;
        data_buffer.light = Light;
        osMutexRelease(data_mutex);
        osDelay(1000);
    }
}

void Read_SensorData(envir_data_t* dat)
{
    osMutexAcquire(data_mutex,osWaitForever);
    dat->temperature = data_buffer.temperature;
    dat->humidity = data_buffer.humidity;
    dat->light = data_buffer.light;
    osMutexRelease(data_mutex);
}

static void SensorExampleEntry(void)
{
    data_mutex = osMutexNew(NULL);
    if(data_mutex == NULL)
	{
		printf("Falied to create data_mutex!\n");
	}

    osThreadAttr_t attr;
    attr.name = "SensorTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = I2C_TASK_STACK_SIZE;
    attr.priority = I2C_TASK_PRIO;

    if (osThreadNew((osThreadFunc_t)SensorTask, NULL, &attr) == NULL)
    {
        printf("Falied to create SensorTask!\n");
    }
}

SYS_RUN(SensorExampleEntry);
