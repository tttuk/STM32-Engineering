#include <stdio.h>
#include "stm32f4xx.h"
#include "dht11.h"
#include "usart.h"
#include "delay.h"

/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    DHT11_Data Data;

    delay_init();               /* 初始化延时函数 */
    UART2_Init(115200);         /* 初始化调试串口 */
    DHT11_Init();               /* 初始化DHT11 */
    printf("DHT11 Init OK\r\n");

    delay_ms(1000);

    while (1) {
        if (DHT11_ReadData(&Data) == 0) {
            printf("HUMI:%d.%d_TEMP:%d.%d\r\n",
                   Data.humi_int, Data.humi_deci,
                   Data.temp_int, Data.temp_deci);
        } else {
            printf("Read DHT11 ERROR!\r\n");
        }
        delay_ms(10000);        /* 每10秒读取一次 */
    }
}
