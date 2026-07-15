/**
  ******************************************************************************
  * @file    main.c
  * @brief   综合场景实验 —— 冷链物流温度核查系统
  * @date    2026-07-15
  ******************************************************************************
  *
  * 场景: 冷链物流温度核查 (Cold Chain Temperature Verification)
  *
  * 核查员刷卡(NFC)认证身份后, 系统自动采集 DHT11 温湿度数据,
  * 在 LCD 上显示核查结果, 判断冷链环境是否符合 2~8°C 存储标准。
  *
  * 硬件:
  *   NFC (PN532) → USART1 PA9/PA10   — 核查员身份卡读取
  *   DHT11        → PE6              — 环境温湿度采集
  *   LCD (ILI9341)→ FSMC 接口        — 核查结果显示终端
  *   USART2       → PA2/PA3          — printf 调试输出
  *
  * 代码复用来源 (已移植实验19的LCD驱动和延时方案):
  *   nfc_pn532.c  — 实验12 NFC读写卡实验
  *   dht11.c      — 实验1  DHT11传感器实验
  *   lcd_ili9341.c— 实验19 LCD液晶屏实验 (函数式FSMC驱动 + 软件延时)
  *   usart.c      — 实验4  串行通信
  *   delay.c      — 实验19 软件忙等延时 (不依赖SysTick)
  ******************************************************************************
  */

#include "stm32f4xx.h"
#include "delay.h"
#include "usart.h"
#include "nfc_pn532.h"
#include "dht11.h"
#include "lcd_ili9341.h"
#include <stdio.h>
#include <string.h>

/*==============================================================================
 * 冷链温度阈值定义 (单位: °C)
 *============================================================================*/
#define TEMP_MIN_COLD_CHAIN    2     // 冷链低温下限
#define TEMP_MAX_COLD_CHAIN    8     // 冷链高温上限

/*==============================================================================
 * LCD 显示布局定义 (横屏 320×240)
 *============================================================================*/
#define LCD_TITLE_X      50
#define LCD_TITLE_Y      20
#define LCD_CARD_UID_X   20
#define LCD_CARD_UID_Y   60
#define LCD_TEMP_X       20
#define LCD_TEMP_Y       110
#define LCD_HUMI_X       20
#define LCD_HUMI_Y       165
#define LCD_STATUS_X     20
#define LCD_STATUS_Y     220
#define LCD_PROMPT_X     50
#define LCD_PROMPT_Y     210

int main(void)
{
    DHT11_Data  dht11_data;
    uint8_t     nfc_buf[16];
    char        lcd_buf[40];

    /*======================================================================
     * 第1步: 启动闪烁 — 确认MCU在跑 (PB5 红色LED闪3次)
     *====================================================================*/
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

    /*======================================================================
     * 第2步: NVIC 优先级分组 (必须在任何中断使能前调用!)
     *====================================================================*/
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

    /*======================================================================
     * 第3步: LCD 初始化 (先于printf，避免semihosting阻塞)
     *====================================================================*/
    LCD_Init();

    /*--- LCD 显示待机标题界面 ---*/
    LCD_SetTextColor(WHITE);
    LCD_SetBackColor(BLACK);
    LCD_Clear(0, 0, LCD_GetLenX(), LCD_GetLenY());

    LCD_SetTextColor(CYAN);
    LCD_DispStringEN(LCD_TITLE_X, LCD_TITLE_Y, 0, "Cold Chain QC System");

    LCD_SetTextColor(WHITE);
    LCD_DispStringEN(LCD_PROMPT_X, LCD_PROMPT_Y, 0, ">> System Init <<");

    /*======================================================================
     * 第4步: 外设初始化
     *====================================================================*/
    UART2_Init(115200);         // 调试串口初始化
    NFC_Init(115200);           // NFC 模块初始化 (USART1)
    DHT11_Init();               // DHT11 传感器初始化 (PE6)

    printf("\r\n========================================\r\n");
    printf("  Cold Chain Temperature Verification\r\n");
    printf("========================================\r\n\r\n");

    /*======================================================================
     * 第5步: NFC 唤醒
     *====================================================================*/
    MAX_TRY = 0;
    printf("NFC: Waking up PN532 module...\r\n");

    /* LCD 显示唤醒进度 */
    LCD_SetTextColor(YELLOW);
    LCD_DispStringEN(LCD_PROMPT_X, LCD_PROMPT_Y, 0, "NFC waking up...  ");

    NFC_WakeUp();

    printf("NFC: Ready, waiting for card...\r\n\r\n");

    /*======================================================================
     * 主循环: 刷卡 → 读温湿度 → LCD显示 → 等待 → 循环
     *====================================================================*/
    while (1)
    {
        /*--- 显示待机提示 ---*/
        LCD_Clear(0, 60, LCD_GetLenX(), LCD_GetLenY() - 60);
        LCD_SetTextColor(WHITE);
        LCD_DispStringEN(LCD_PROMPT_X, LCD_PROMPT_Y, 0, ">> Tap Card <<");

        /*--- 阻塞等待 NFC 刷卡 ---*/
        printf("[WAIT] Awaiting inspector card...\r\n");
        if (NFC_Read(2, nfc_buf) < 0)
        {
            printf("[ERR] NFC Read failed\r\n");
            continue;
        }

        /*--- 刷卡成功, 显示核查员卡号 ---*/
        printf("[OK] Card detected! UID: %02X %02X %02X %02X\r\n",
               UID[0], UID[1], UID[2], UID[3]);

        LCD_Clear(0, 55, LCD_GetLenX(), LCD_GetLenY() - 55);

        LCD_SetTextColor(CYAN);
        LCD_DispStringEN(LCD_CARD_UID_X, LCD_CARD_UID_Y, 0, "Inspector Card:");

        sprintf(lcd_buf, "UID: %02X%02X%02X%02X",
                UID[0], UID[1], UID[2], UID[3]);
        LCD_SetTextColor(YELLOW);
        LCD_DispStringEN(LCD_CARD_UID_X + 20, LCD_CARD_UID_Y + 25, 0, lcd_buf);

        /*--- 读取 DHT11 温湿度 ---*/
        printf("[DHT11] Reading temperature & humidity...\r\n");
        if (DHT11_ReadData(&dht11_data) == 0)
        {
            printf("[DHT11] Temp: %d.%d C, Humi: %d.%d %%\r\n",
                   dht11_data.temp_int, dht11_data.temp_deci,
                   dht11_data.humi_int, dht11_data.humi_deci);

            /*--- LCD 显示温度 ---*/
            LCD_SetTextColor(WHITE);
            LCD_DispStringEN(LCD_TEMP_X, LCD_TEMP_Y, 0, "Temperature:");

            sprintf(lcd_buf, "%d.%d C",
                    dht11_data.temp_int, dht11_data.temp_deci);
            LCD_SetTextColor(YELLOW);
            LCD_DispStringEN(LCD_TEMP_X + 20, LCD_TEMP_Y + 25, 0, lcd_buf);

            /*--- LCD 显示湿度 ---*/
            LCD_SetTextColor(WHITE);
            LCD_DispStringEN(LCD_HUMI_X, LCD_HUMI_Y, 0, "Humidity:");

            sprintf(lcd_buf, "%d.%d %%",
                    dht11_data.humi_int, dht11_data.humi_deci);
            LCD_SetTextColor(YELLOW);
            LCD_DispStringEN(LCD_HUMI_X + 20, LCD_HUMI_Y + 25, 0, lcd_buf);

            /*--- 冷链合规判定 ---*/
            LCD_SetTextColor(WHITE);
            LCD_DispStringEN(LCD_STATUS_X, LCD_STATUS_Y, 0, "QC Result:");

            if (dht11_data.temp_int >= TEMP_MIN_COLD_CHAIN &&
                dht11_data.temp_int <= TEMP_MAX_COLD_CHAIN)
            {
                LCD_SetTextColor(GREEN);
                LCD_DispStringEN(LCD_STATUS_X + 20, LCD_STATUS_Y + 30, 0,
                                 "[ PASS ]  Cold Chain OK");
                printf("[QC] Result: PASS (temp in 2~8 C range)\r\n");
            }
            else
            {
                LCD_SetTextColor(RED);
                sprintf(lcd_buf, "[ FAIL ]  Out of %d~%d C!",
                        TEMP_MIN_COLD_CHAIN, TEMP_MAX_COLD_CHAIN);
                LCD_DispStringEN(LCD_STATUS_X + 20, LCD_STATUS_Y + 30, 0, lcd_buf);
                printf("[QC] Result: FAIL (temp out of 2~8 C range!)\r\n");
            }
        }
        else
        {
            printf("[ERR] DHT11 read failed!\r\n");
            LCD_SetTextColor(RED);
            LCD_DispStringEN(LCD_TEMP_X, LCD_TEMP_Y, 0, "Sensor Error!");
            LCD_DispStringEN(LCD_TEMP_X, LCD_TEMP_Y + 25, 0,
                             "Check DHT11 wiring");
        }

        /*--- 停留 5 秒后返回待机界面 ---*/
        printf("[INFO] Displaying result for 5 seconds...\r\n\r\n");
        delay_ms(5000);
    }
}
