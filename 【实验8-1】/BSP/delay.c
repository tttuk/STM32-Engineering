#include "delay.h"

static uint32_t fac_us = 0;  /* us延时倍乘数 */

/**
  * @brief  延时初始化，使用SysTick
  * @param  无
  * @retval 无
  */
void delay_init(void)
{
    /* SystemCoreClock 单位为 Hz，除以 1000000 得到 1us 的计数值 */
    fac_us = SystemCoreClock / 1000000;
}

/**
  * @brief  微秒级延时
  * @param  nus: 延时的微秒数 (0 ~ 798915)
  * @retval 无
  */
void delay_us(uint32_t nus)
{
    uint32_t temp;
    SysTick->LOAD = nus * fac_us;            /* 装载计数值 */
    SysTick->VAL  = 0x00;                     /* 清空计数器 */
    SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk; /* 使能SysTick */

    do {
        temp = SysTick->CTRL;
    } while ((temp & 0x01) && !(temp & (1 << 16))); /* 等待计数到0 */

    SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk; /* 关闭SysTick */
    SysTick->VAL  = 0x00;                      /* 清空计数器 */
}

/**
  * @brief  毫秒级延时
  * @param  nms: 延时的毫秒数
  * @retval 无
  */
void delay_ms(uint32_t nms)
{
    uint32_t i;
    for (i = 0; i < nms; i++) {
        delay_us(1000);
    }
}
