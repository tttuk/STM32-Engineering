#include "stm32f4xx.h"

void my_delay(int ms)
{
    int i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 10000; j++);
}

// ===== 串口发送一个字符 =====
void UART_SendByte(USART_TypeDef* USARTx, uint8_t ch)
{
    while(USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
    USART_SendData(USARTx, ch);
}

// ===== 串口发送字符串 =====
void UART_SendString(USART_TypeDef* USARTx, char* str)
{
    while(*str)
    {
        UART_SendByte(USARTx, *str++);
    }
}

int main(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    
    // ===== 1. 初始化LED1 =====
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOB, ENABLE);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // LED快闪3次，表示启动
    for(int i=0; i<3; i++) { GPIO_ResetBits(GPIOB, GPIO_Pin_5); my_delay(200); GPIO_SetBits(GPIOB, GPIO_Pin_5); my_delay(200); }
    
    // ===== 2. 初始化串口 USART2 =====
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2);
    
    USART_InitStructure.USART_BaudRate = 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_Init(USART2, &USART_InitStructure);
    USART_Cmd(USART2, ENABLE);
    
    // LED慢闪2次，表示串口初始化完成
    for(int i=0; i<2; i++) { GPIO_ResetBits(GPIOB, GPIO_Pin_5); my_delay(300); GPIO_SetBits(GPIOB, GPIO_Pin_5); my_delay(300); }
    
    // ===== 3. 主循环：LED闪烁 + 串口发送 =====
    while(1)
    {
        GPIO_ResetBits(GPIOB, GPIO_Pin_5);       // LED亮
        UART_SendString(USART2, "Hello NFC!\r\n");
        my_delay(500);
        
        GPIO_SetBits(GPIOB, GPIO_Pin_5);         // LED灭
        my_delay(500);
    }
}