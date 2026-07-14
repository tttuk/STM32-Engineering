#include <stddef.h>
#include "dht11.h"
#include "delay.h"

#define DHT11_PORT    GPIOE          /* DHT11端口 */
#define DHT11_PIN     GPIO_Pin_6     /* DHT11引脚号 */

static void DHT11_Mode_IN(void);
static void DHT11_Mode_OUT(void);

#define DHT11_DELAY_US(us)  delay_us(us)
#define DHT11_DELAY_MS(ms)  delay_ms(ms)

/**
  * @brief  使DHT11-DATA引脚变为输入模式
  * @param  无
  * @retval 无
  */
static void DHT11_Mode_IN(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin   = DHT11_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_IN;       /* 输入模式 */
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  使DHT11-DATA引脚变为推挽输出模式
  * @param  无
  * @retval 无
  */
static void DHT11_Mode_OUT(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.GPIO_Pin   = DHT11_PIN;
    GPIO_InitStruct.GPIO_Mode  = GPIO_Mode_OUT;      /* 输出模式 */
    GPIO_InitStruct.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStruct.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_25MHz;
    GPIO_Init(DHT11_PORT, &GPIO_InitStruct);
}

/**
  * @brief  DHT11 初始化函数
  * @param  无
  * @retval 无
  */
void DHT11_Init(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOE, ENABLE); /* 开启GPIO相应时钟 */

    DHT11_Mode_OUT();                       /* 设置为输出模式 */
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);    /* 初始时拉高GPIO */
}

/**
  * @brief  循环等待指定电平信号
  * @param  bitValue: 等待的电平值 (Bit_RESET 或 Bit_SET)
  * @param  max_us:   最大等待时间(us)
  * @retval 0:成功, -1:超时
  */
static int8_t DHT11_WaitFor(BitAction bitValue, uint32_t max_us)
{
    uint32_t try_num = 0;
    while ((GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) != bitValue) && try_num < max_us) {
        try_num++;
        DHT11_DELAY_US(1);
    }
    if (try_num >= max_us) {    /* 超时未收到预期信号，出错返回 */
        return -1;
    }
    return 0;
}

/**
  * @brief  从DHT11读取一个bit
  * @param  无
  * @retval 0/1:数据位, -1:超时错误
  * @note   数据位0与1的区别在于高电平的时长，
  *         数据0高电平约28us，数据1高电平约70us。
  *         先过滤低电平前导，再延时40us后检测电平即可区分。
  */
static int8_t DHT11_ReadBit(void)
{
    /* 等待电平变低，表示数据位开始传输 */
    if (DHT11_WaitFor(Bit_RESET, 100) < 0) { return -1; }

    /* 等待电平变高，表示进入0与1的区分时段 */
    if (DHT11_WaitFor(Bit_SET, 100) < 0) { return -1; }

    /* 等待40us，以便区分数据位0或1 */
    DHT11_DELAY_US(40);  /* 数据0在28us后会变低 */

    if (GPIO_ReadInputDataBit(DHT11_PORT, DHT11_PIN) == Bit_SET) { /* 仍为高电平表示1 */
        return 1;
    } else {
        return 0;
    }
}

/**
  * @brief  从DHT11读取一个字节，MSB先行
  * @param  无
  * @retval 读取到的字节
  */
static uint8_t DHT11_ReadByte(void)
{
    uint8_t byte = 0;
    uint8_t i;

    for (i = 0; i < 8; i++) {
        byte <<= 1;
        byte |= DHT11_ReadBit();
    }
    return byte;
}

/**
  * @brief  从DHT11读取一次温湿度数据
  * @param  pData: 存放读取结果的结构体指针
  * @retval 0:成功, -1:失败(超时或校验错误)
  * @note   一次完整的数据传输为40bit，高位先出:
  *         8bit湿度整数 + 8bit湿度小数 + 8bit温度整数
  *         + 8bit温度小数 + 8bit校验和
  */
int8_t DHT11_ReadData(DHT11_Data *pData)
{
    if (pData == NULL) { return -1; }

    DHT11_Mode_OUT();                          /* 未开始时输出模式 */
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);       /* 未开始时电平拉高 */
    GPIO_ResetBits(DHT11_PORT, DHT11_PIN);     /* 开始信号，拉低保持18ms */
    DHT11_DELAY_MS(18);
    GPIO_SetBits(DHT11_PORT, DHT11_PIN);       /* 拉高保持30us，开始信号完成 */
    DHT11_DELAY_US(30);

    DHT11_Mode_IN();                           /* 转为输入模式，等待DHT11响应 */

    /* 等待GPIO拉低的响应信号 */
    if (DHT11_WaitFor(Bit_RESET, 200) < 0) { return -1; }

    /* 等待GPIO拉高的响应信号 */
    if (DHT11_WaitFor(Bit_SET, 100) < 0) { return -1; }

    /* 延时80us，响应信号会在80us内结束 */
    DHT11_DELAY_US(80);

    /* 开始接收数据 */
    pData->humi_int  = DHT11_ReadByte();
    pData->humi_deci = DHT11_ReadByte();
    pData->temp_int  = DHT11_ReadByte();
    pData->temp_deci = DHT11_ReadByte();
    pData->check_sum = DHT11_ReadByte();

    /* 检查读取的数据是否正确 */
    if (pData->check_sum != pData->humi_int + pData->humi_deci +
                           pData->temp_int + pData->temp_deci) {
        return -1;
    }

    return 0;
}
