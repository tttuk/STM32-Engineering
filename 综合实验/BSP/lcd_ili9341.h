#ifndef __LCD_ILI9341_H__
#define    __LCD_ILI9341_H__
#include "stm32f4xx.h"
#include "lcd_fonts.h"

/******************************* 定义 ILI934 显示屏常用颜色 ********************************/
#define      WHITE                           0xFFFF
#define      BLACK                         0x0000
#define      GREY                          0xF7DE
#define      BLUE                          0x001F
#define      BLUE2                         0x051F
#define      RED                           0xF800
#define      MAGENTA                       0xF81F
#define      GREEN                         0x07E0
#define      CYAN                          0x7FFF
#define      YELLOW                        0xFFE0
#define      BRED                          0xF81F
#define      GRED                          0xFFE0
#define      GBLUE                         0x07FF

void LCD_Init(void);

uint16_t LCD_GetLenX(void);
uint16_t LCD_GetLenY(void);
void LCD_SetColors(uint16_t TextColor, uint16_t BackColor);
void LCD_GetColors(uint16_t *TextColor, uint16_t *BackColor);
void LCD_SetTextColor(uint16_t Color);
void LCD_SetBackColor(uint16_t Color);

void LCD_SetPixel(uint16_t usX, uint16_t usY);
uint16_t LCD_GetPixel(uint16_t usX, uint16_t usY);
void LCD_Clear(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);

void LCD_DrawLine(uint16_t usX1, uint16_t usY1, uint16_t usX2, uint16_t usY2);
void LCD_DrawRectangle(uint16_t usX_Start, uint16_t usY_Start, uint16_t usWidth, uint16_t usHeight, uint8_t ucFilled);
void LCD_DrawCircle(uint16_t usX_Center, uint16_t usY_Center, uint16_t usRadius, uint8_t ucFilled);

void LCD_DispStringEN(uint16_t usX, uint16_t usY, uint8_t uDir, char *pStr);

/* ILI9341 底层操作 */
void ILI9341_WriteCmd(uint16_t usCmd);
void ILI9341_WriteData(uint16_t usData);
void ILI9341_OpenWindow(uint16_t usX, uint16_t usY, uint16_t usWidth, uint16_t usHeight);

#define CMD_SetCoordinateX  0x2A
#define CMD_SetCoordinateY  0x2B
#define CMD_SetPixel        0x2C

#endif /* __LCD_ILI9341_H__ */
