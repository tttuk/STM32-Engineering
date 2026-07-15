/**
 * 实验8：LCD液晶屏实验 (ILI9341, FSMC接口)
 * 开发板：QST-AU100 (STM32F407ZGT6)
 * 代码移植自实验19（确认可正常工作）
 */
#include <stdio.h>
#include "stm32f4xx.h"
#include "delay.h"
#include "usart.h"
#include "lcd_ili9341.h"
#include "lcd_fonts.h"

int main(void)
{
    /* ====== 第1步：让PB5红色LED先闪烁3次，确认MCU在跑 ====== */
    {
        RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
        GPIO_InitTypeDef g;
        g.GPIO_Pin   = GPIO_Pin_5;
        g.GPIO_Mode  = GPIO_Mode_OUT;
        g.GPIO_OType = GPIO_OType_PP;
        g.GPIO_Speed = GPIO_Speed_25MHz;
        g.GPIO_PuPd  = GPIO_PuPd_NOPULL;
        GPIO_Init(GPIOB, &g);
        int i;
        for (i = 0; i < 3; i++) {
            GPIO_ResetBits(GPIOB, GPIO_Pin_5);  /* 点亮 (低电平) */
            delay_ms(200);
            GPIO_SetBits(GPIOB, GPIO_Pin_5);    /* 熄灭 */
            delay_ms(200);
        }
    }

    /* ====== 第2步：初始化LCD ====== */
    LCD_Init();

    /* ====== 第3步：显示测试画面 ====== */
    LCD_SetBackColor(BLACK);
    LCD_Clear(0, 0, LCD_GetLenX(), LCD_GetLenY());

    /* 红色方块 */
    LCD_SetTextColor(RED);
    LCD_SetBackColor(RED);
    LCD_Clear(50, 50, 100, 100);

    /* 白色文字 */
    LCD_SetTextColor(WHITE);
    LCD_SetBackColor(BLACK);
    LCD_SetFontEN(&ASCII_8x16);
    LCD_DispStringEN(10, 10, 0, "LCD Test OK!");
    LCD_DispStringEN(10, 30, 0, "240x320 ILI9341");

    /* ====== 第4步：最后初始化UART并打印（避免semihosting阻塞LCD） ====== */
    UART2_Init(115200);
    printf("\r\nLCD TEST DONE\r\n");

    while(1) { }
}
