/**
 * 软件延时函数 (168MHz) — 移植自实验19，确认可正常工作
 * 不依赖 SysTick，无需 delay_init()
 */
#include "delay.h"

void delay_us(u16 nus)
{
    volatile u16 i;
    while (nus--)
    {
        i = 31;  /* 168MHz下约1us */
        while (i--);
    }
}

void delay_ms(u16 nms)
{
    volatile u16 i;
    while (nms--)
    {
        i = 33800;  /* 约1ms (168MHz下校准) */
        while (i--);
    }
}
