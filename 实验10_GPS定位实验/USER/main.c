#include <stdio.h>
#include "stm32f4xx.h"
#include "gps.h"
#include "usart.h"


/**
  * @brief  主函数
  * @param  无
  * @retval 无
  */
int main(void)
{
    UART2_Init(115200);
    printf("=== GPS Experiment Start ===\r\n");
    printf("UART2 Debug OK, baud=115200\r\n");

    GPS_Init(9600);
    printf("GPS UART4 Init OK, baud=9600\r\n");
    printf("Waiting for GPS data...\r\n");

    while(1){
        GPS_ReadAndParse();
    }
}
