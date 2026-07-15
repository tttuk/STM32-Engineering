/**
 * ILI9341 LCD 驱动 (FSMC 8080接口) — QST-AU100 开发板
 *
 * 引脚:
 *   FSMC NE1  PD7  → CS
 *   FSMC A16  PD11 → RS (命令/数据)
 *   FSMC NWE  PD5  → WR
 *   FSMC NOE  PD4  → RD
 *   FSMC D0~D15: PD14/PD15/PD0/PD1/PE7~PE15
 *   RST: PE1
 *   BL:  PD12 (低电平点亮)
 */
#include <stddef.h>
#include "lcd_ili9341.h"
#include "lcd_fonts.h"
#include "delay.h"

/* ===== 地址定义 ===== */
#define FSMC_Addr_ILI9341_CMD   ((uint32_t)0x60000000)
#define FSMC_Addr_ILI9341_DATA  ((uint32_t)0x60020000)

/* MCU写命令/数据/读数据 — 函数实现(供外部调用) */
void ILI9341_WriteCmd(uint16_t cmd)   { *(__IO uint16_t*)(FSMC_Addr_ILI9341_CMD) = cmd; }
void ILI9341_WriteData(uint16_t data) { *(__IO uint16_t*)(FSMC_Addr_ILI9341_DATA) = data; }
uint16_t ILI9341_ReadData(void)        { return *(__IO uint16_t*)(FSMC_Addr_ILI9341_DATA); }

/* ===== IO定义 ===== */
#define  ILI9341_RST_PORT    GPIOE
#define  ILI9341_RST_PIN     GPIO_Pin_1
#define  ILI9341_BK_PORT     GPIOD
#define  ILI9341_BK_PIN      GPIO_Pin_12

#define ILI9341_LESS_PIXEL   240
#define ILI9341_MORE_PIXEL   320
#define ILI9341_DELAY_MS(x)  delay_ms(x)

/* ===== 命令宏 ===== */
#define CMD_RESET         0x0001
#define CMD_SLEEP_OUT    0x0011
#define CMD_DISPLAY_OFF    0x0028
#define CMD_DISPLAY_ON    0x0029
#define CMD_PAGE_ADDR    0x002B
#define CMD_GRAM        0x002C
#define CMD_MAC            0x0036
#define CMD_PIXEL_FORMAT   0x003A

/* ===== 全局变量 ===== */
uint8_t g_LCD_ScanMode = 2;
static uint16_t g_LCD_TextColor = BLACK;
static uint16_t g_LCD_BackColor = WHITE;
uint16_t g_LCD_LenX = ILI9341_LESS_PIXEL;
uint16_t g_LCD_LenY = ILI9341_MORE_PIXEL;

/* ===== 函数声明 ===== */
static void ILI9341_GPIO_Config(void);
static void ILI9341_FSMC_Config(void);
static void ILI9341_Rst(void);
static void ILI9341_REG_Config(void);
static void ILI9341_FillColor(uint32_t ulAmout_Point, uint16_t usColor);
static void ILI9341_BackLight(uint8_t uOnOff);
static void ILI9341_GramScan(uint8_t ucOtion);

/* ===== GPIO初始化 ===== */
static void ILI9341_GPIO_Config(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD | RCC_AHB1Periph_GPIOE, ENABLE);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_4 | GPIO_Pin_5 |
                                  GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 |
                                  GPIO_Pin_10 |GPIO_Pin_11 | GPIO_Pin_14 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 |
                                  GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 |
                                  GPIO_Pin_15;
    GPIO_Init(GPIOE, &GPIO_InitStructure);

    GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource1, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource4, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource5, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource7, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource8, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource9, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource10, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource11, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource14, GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOD, GPIO_PinSource15, GPIO_AF_FSMC);

    GPIO_PinAFConfig(GPIOE, GPIO_PinSource7 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource8 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource9 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource10 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource11 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource12 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource13 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource14 , GPIO_AF_FSMC);
    GPIO_PinAFConfig(GPIOE, GPIO_PinSource15 , GPIO_AF_FSMC);

    /* 背光 PD12 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_12;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOD, &GPIO_InitStructure);

    /* RST PE1 */
    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_1;
    GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/* ===== 背光 (低电平点亮) ===== */
static void ILI9341_BackLight(uint8_t uOnOff)
{
    if (uOnOff)
        GPIO_ResetBits(ILI9341_BK_PORT, ILI9341_BK_PIN);   /* 低电平亮 */
    else
        GPIO_SetBits(ILI9341_BK_PORT, ILI9341_BK_PIN);
}

/* ===== 复位 ===== */
static void ILI9341_Rst(void)
{
    GPIO_ResetBits(ILI9341_RST_PORT, ILI9341_RST_PIN);
    ILI9341_DELAY_MS(5);
    GPIO_SetBits(ILI9341_RST_PORT, ILI9341_RST_PIN);
    ILI9341_DELAY_MS(5);
}

/* ===== FSMC配置 ===== */
static void ILI9341_FSMC_Config(void)
{
    FSMC_NORSRAMInitTypeDef  FSMC_NORSRAMInitStructure;
    FSMC_NORSRAMTimingInitTypeDef  p;

    RCC_AHB3PeriphClockCmd(RCC_AHB3Periph_FSMC, ENABLE);

    p.FSMC_AddressSetupTime = 0x6;
    p.FSMC_AddressHoldTime  = 0;
    p.FSMC_DataSetupTime    = 0x6;
    p.FSMC_BusTurnAroundDuration = 0;
    p.FSMC_CLKDivision = 0;
    p.FSMC_DataLatency = 0;
    p.FSMC_AccessMode = FSMC_AccessMode_A;

    FSMC_NORSRAMInitStructure.FSMC_Bank = FSMC_Bank1_NORSRAM1;
    FSMC_NORSRAMInitStructure.FSMC_DataAddressMux = FSMC_DataAddressMux_Disable;
    FSMC_NORSRAMInitStructure.FSMC_MemoryType = FSMC_MemoryType_SRAM;
    FSMC_NORSRAMInitStructure.FSMC_MemoryDataWidth = FSMC_MemoryDataWidth_16b;
    FSMC_NORSRAMInitStructure.FSMC_BurstAccessMode = FSMC_BurstAccessMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_AsynchronousWait = FSMC_AsynchronousWait_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalPolarity = FSMC_WaitSignalPolarity_Low;
    FSMC_NORSRAMInitStructure.FSMC_WrapMode = FSMC_WrapMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignalActive = FSMC_WaitSignalActive_BeforeWaitState;
    FSMC_NORSRAMInitStructure.FSMC_WriteOperation = FSMC_WriteOperation_Enable;
    FSMC_NORSRAMInitStructure.FSMC_WaitSignal = FSMC_WaitSignal_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ExtendedMode = FSMC_ExtendedMode_Disable;
    FSMC_NORSRAMInitStructure.FSMC_WriteBurst   = FSMC_WriteBurst_Disable;
    FSMC_NORSRAMInitStructure.FSMC_ReadWriteTimingStruct = &p;
    FSMC_NORSRAMInitStructure.FSMC_WriteTimingStruct     = &p;
    FSMC_NORSRAMInit(&FSMC_NORSRAMInitStructure);

    FSMC_NORSRAMCmd(FSMC_Bank1_NORSRAM1, ENABLE);
}

/* ===== ILI9341寄存器配置 ===== */
static void ILI9341_REG_Config(void)
{
    ILI9341_WriteCmd(CMD_RESET);
    ILI9341_DELAY_MS(120);

    ILI9341_WriteCmd(CMD_DISPLAY_OFF);

    ILI9341_WriteCmd(CMD_PIXEL_FORMAT);
    ILI9341_WriteData(0x55);  /* 16bit/pixel */

    ILI9341_WriteCmd(CMD_SLEEP_OUT);
    ILI9341_DELAY_MS(100);
    ILI9341_WriteCmd(CMD_DISPLAY_ON);
    ILI9341_DELAY_MS(100);

    ILI9341_WriteCmd(CMD_GRAM);
    ILI9341_DELAY_MS(5);
}

/* ===== 扫描方向 ===== */
static void ILI9341_GramScan(uint8_t ucOption)
{
    if(ucOption > 7) return;
    g_LCD_ScanMode = ucOption;

    if(ucOption%2 == 0) {
        g_LCD_LenX = ILI9341_LESS_PIXEL;
        g_LCD_LenY = ILI9341_MORE_PIXEL;
    } else {
        g_LCD_LenX = ILI9341_MORE_PIXEL;
        g_LCD_LenY = ILI9341_LESS_PIXEL;
    }

    ILI9341_WriteCmd(CMD_MAC);
    ILI9341_WriteData(0x08 | (ucOption<<5));

    ILI9341_WriteCmd(CMD_SetCoordinateX);
    ILI9341_WriteData(0); ILI9341_WriteData(0);
    ILI9341_WriteData((g_LCD_LenX-1)>>8); ILI9341_WriteData((g_LCD_LenX-1)&0xFF);

    ILI9341_WriteCmd(CMD_SetCoordinateY);
    ILI9341_WriteData(0); ILI9341_WriteData(0);
    ILI9341_WriteData((g_LCD_LenY-1)>>8); ILI9341_WriteData((g_LCD_LenY-1)&0xFF);

    ILI9341_WriteCmd(CMD_SetPixel);
}

/* ===== 总初始化 ===== */
void LCD_Init(void)
{
    ILI9341_GPIO_Config();
    ILI9341_FSMC_Config();
    ILI9341_Rst();
    ILI9341_BackLight(ENABLE);
    ILI9341_REG_Config();
    ILI9341_GramScan(g_LCD_ScanMode);
}

/* ===== 窗口设置 ===== */
void ILI9341_OpenWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
    ILI9341_WriteCmd(CMD_SetCoordinateX);
    ILI9341_WriteData(usX >> 8);
    ILI9341_WriteData(usX & 0xff);
    ILI9341_WriteData((usX + usWidth - 1) >> 8);
    ILI9341_WriteData((usX + usWidth - 1) & 0xff);

    ILI9341_WriteCmd(CMD_SetCoordinateY);
    ILI9341_WriteData(usY >> 8);
    ILI9341_WriteData(usY & 0xff);
    ILI9341_WriteData((usY + usHeight - 1) >> 8);
    ILI9341_WriteData((usY + usHeight - 1) & 0xff);
}

/* ===== 批量填充像素 ===== */
static void ILI9341_FillColor(uint32_t ulAmout_Point, uint16_t usColor)
{
    uint32_t i;
    ILI9341_WriteCmd(CMD_SetPixel);
    for(i = 0; i < ulAmout_Point; i++)
        ILI9341_WriteData(usColor);
}

/* ===== 获取尺寸 ===== */
uint16_t LCD_GetLenX(void) { return g_LCD_LenX; }
uint16_t LCD_GetLenY(void) { return g_LCD_LenY; }

/* ===== 颜色设置 ===== */
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor) {
    g_LCD_TextColor = TextColor;
    g_LCD_BackColor = BackColor;
}
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor) {
    *TextColor = g_LCD_TextColor;
    *BackColor = g_LCD_BackColor;
}
void LCD_SetTextColor(uint16_t Color) { g_LCD_TextColor = Color; }
void LCD_SetBackColor(uint16_t Color) { g_LCD_BackColor = Color; }

/* ===== 清屏 ===== */
void LCD_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight)
{
    ILI9341_OpenWindow(usX, usY, usWidth, usHeight);
    ILI9341_FillColor(usWidth * usHeight, g_LCD_BackColor);
}

/* ===== 画点 ===== */
void LCD_SetPixel(uint16_t uX, uint16_t uY)
{
    if((uX < g_LCD_LenX) && (uY < g_LCD_LenY)) {
        ILI9341_OpenWindow(uX, uY, 1, 1);
        ILI9341_FillColor(1, g_LCD_TextColor);
    }
}

uint16_t LCD_GetPixel(uint16_t usX, uint16_t usY)
{
    uint16_t val;
    ILI9341_OpenWindow(usX, usY, 1, 1);
    ILI9341_WriteCmd(CMD_GRAM);
    val = ILI9341_ReadData();  /* 第一次读无效(假读) */
    val = ILI9341_ReadData();
    return val;
}

/* ===== 画线 (Bresenham) ===== */
void LCD_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2)
{
    int dx  = (usX2 > usX1) ? (usX2 - usX1) : (usX1 - usX2);
    int dy  = (usY2 > usY1) ? (usY2 - usY1) : (usY1 - usY2);
    int sx  = (usX1 < usX2) ? 1 : -1;
    int sy  = (usY1 < usY2) ? 1 : -1;
    int err = dx - dy;

    while(1) {
        LCD_SetPixel(usX1, usY1);
        if(usX1 == usX2 && usY1 == usY2) break;
        int e2 = err * 2;
        if(e2 > -dy) { err -= dy; usX1 += sx; }
        if(e2 < dx)  { err += dx; usY1 += sy; }
    }
}

/* ===== 画矩形 ===== */
void LCD_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled)
{
    if(ucFilled) {
        ILI9341_OpenWindow(usX_Start, usY_Start, usWidth, usHeight);
        ILI9341_FillColor((uint32_t)usWidth * usHeight, g_LCD_TextColor);
    } else {
        uint16_t i;
        LCD_SetPixel(usX_Start, usY_Start);
        for(i=0; i<usWidth; i++)  { LCD_SetPixel(usX_Start+i, usY_Start); LCD_SetPixel(usX_Start+i, usY_Start+usHeight-1); }
        for(i=0; i<usHeight; i++) { LCD_SetPixel(usX_Start, usY_Start+i); LCD_SetPixel(usX_Start+usWidth-1, usY_Start+i); }
    }
}

/* ===== 画圆 ===== */
void LCD_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled)
{
    int x = usRadius, y = 0, err = 1 - usRadius;

    while(x >= y) {
        if(ucFilled) {
            uint16_t i;
            for(i=usX_Center-x; i<=usX_Center+x; i++) LCD_SetPixel(i, usY_Center+y);
            for(i=usX_Center-x; i<=usX_Center+x; i++) LCD_SetPixel(i, usY_Center-y);
            for(i=usX_Center-y; i<=usX_Center+y; i++) LCD_SetPixel(i, usY_Center+x);
            for(i=usX_Center-y; i<=usX_Center+y; i++) LCD_SetPixel(i, usY_Center-x);
        } else {
            LCD_SetPixel(usX_Center+x, usY_Center+y); LCD_SetPixel(usX_Center+y, usY_Center+x);
            LCD_SetPixel(usX_Center-y, usY_Center+x); LCD_SetPixel(usX_Center-x, usY_Center+y);
            LCD_SetPixel(usX_Center-x, usY_Center-y); LCD_SetPixel(usX_Center-y, usY_Center-x);
            LCD_SetPixel(usX_Center+y, usY_Center-x); LCD_SetPixel(usX_Center+x, usY_Center-y);
        }
        y++;
        if(err <= 0) err += 2*y+1;
        else { x--; err += 2*(y-x)+1; }
    }
}

/* ===== 字模显示 ===== */
static void LCD_DispMask(uint16_t uX, uint16_t uY, uint16_t uWidth, int16_t uHeight, const uint8_t *pMask)
{
    uint16_t i, j;
    uint16_t uFontLenght = uWidth * uHeight / 8;
    ILI9341_OpenWindow(uX, uY, uWidth, uHeight);
    ILI9341_WriteCmd(CMD_SetPixel);
    for(i=0; i<uFontLenght; i++) {
        for(j=0; j<8; j++) {
            if(pMask[i] & (0x80>>j))
                ILI9341_WriteData(g_LCD_TextColor);
            else
                ILI9341_WriteData(g_LCD_BackColor);
        }
    }
}

/* ===== 英文字符串显示 ===== */
void LCD_DispStringEN(uint16_t uX, uint16_t uY, uint8_t uDir, char *pStr)
{
    uint8_t *pMask = NULL;
    FONT *font = LCD_GetFontEN();
    uint16_t uCharWidth  = font->Width;
    uint16_t uCharHeight = font->Height;

    while(*pStr != '\0') {
        pMask = LCD_GetMaskEN(*pStr);
        pStr++;

        if(g_LCD_LenX - uX < uCharWidth) { uX = 0; uY += uCharHeight; }
        if(g_LCD_LenY - uY < uCharHeight) { uY = 0; }

        LCD_DispMask(uX, uY, uCharWidth, uCharHeight, pMask);
        uX += (uDir == 0) ? uCharWidth : uCharHeight;
    }
}
