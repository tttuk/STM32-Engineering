#include "stm32f4xx.h"
#include "delay.h"     //����ʱ

/*����һ֡���ݣ�׼�����ͣ�\r\n�����س����У��������ֽ�*/
u8 SendBuf[] = "Hello Everyone!\r\n"; //ȫ��17���ֽ�

void USART2_Init( u32 baudrate );

int main(void)
{
	DMA_InitTypeDef DMA_InitStruct;//���� DMA ��ʼ���ṹ��
USART2_Init(115200); //����2 ��ʼ��������115200
	USART_DMACmd(USART2,USART_DMAReq_Tx,ENABLE); //ʹ�ܴ���2��DMA����
USART_DMACmd(USART2,USART_DMAReq_Rx,ENABLE); //ʹ�ܴ���2��DMA����

RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_DMA1,ENABLE); //����DMA1ʱ��

/* ���� DMA Stream */
/*���� DMA1 ��ͨ���������ַ���洢����ַ�����ݷ��������͵���Ҫ��*/
DMA_InitStruct.DMA_Channel = DMA_Channel_4; //ͨ��ѡ��
DMA_InitStruct.DMA_PeripheralBaseAddr = (u32)&USART2->DR;//DMA�����ַ��Ϊ����2
DMA_InitStruct.DMA_Memory0BaseAddr = (u32)SendBuf;//DMA �洢����ַ��Ҫ���͵ĵ�ַ
DMA_InitStruct.DMA_DIR = DMA_DIR_MemoryToPeripheral;//�洢��-->����ģʽ
DMA_InitStruct.DMA_BufferSize = 17;//���ݴ�����
DMA_InitStruct.DMA_PeripheralInc = DMA_PeripheralInc_Disable;//�����ַ������
DMA_InitStruct.DMA_MemoryInc = DMA_MemoryInc_Enable;//�洢����ַ����

/* ���� DMA1 �������� */
DMA_InitStruct.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;//�������ݳ���:8λ
DMA_InitStruct.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;//�洢�����ݳ���:8λ
DMA_InitStruct.DMA_Mode = DMA_Mode_Normal;// ʹ����ͨģʽ
DMA_InitStruct.DMA_Priority = DMA_Priority_Medium;//�е����ȼ�
DMA_InitStruct.DMA_FIFOMode = DMA_FIFOMode_Disable; //��ʹ��FIFO
DMA_InitStruct.DMA_FIFOThreshold = DMA_FIFOThreshold_Full;
DMA_InitStruct.DMA_MemoryBurst = DMA_MemoryBurst_Single;//�洢��ͻ�����δ���
DMA_InitStruct.DMA_PeripheralBurst = DMA_PeripheralBurst_Single;//����ͻ�����δ���
DMA_Init(DMA1_Stream6, &DMA_InitStruct);//ʹ���������� ��ʼ��DMA Stream6
	DMA_Cmd(DMA1_Stream6, ENABLE); //������� DMA �������ݵ�����2�����Զ˽��յ�����
while(1)
{

}

}

void USART2_Init( u32 baudrate )
{
GPIO_InitTypeDef GPIO_InitStructure; //����һ��GPIO��ʼ���õĽṹ��
USART_InitTypeDef USART_InitStructure; //����һ�����ڳ�ʼ���õĽṹ��

/*ʹ��GPIOA��USART2ʱ��*/
RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE); //GPIOA��AHB1����������
RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE); //USART2��APB1Ӳ��������

/*USART2 GPIO����*/
GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3; //USART2��IO
GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF; //����ģʽ
GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;//�ٶ�50MHz
GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //���츴�����
GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //��������
GPIO_Init(GPIOA,&GPIO_InitStructure);

/*USART2 ��Ӧ���Ÿ���ӳ��*/
GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //ӳ��PA2��USART2��ʹ��
GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //ӳ��PA3��USART2��ʹ��

/*USART2 �˿�����*/
USART_InitStructure.USART_BaudRate = baudrate; //����������
USART_InitStructure.USART_WordLength = USART_WordLength_8b; //�ֳ�8bit
USART_InitStructure.USART_StopBits = USART_StopBits_1; //1ֹͣλ
USART_InitStructure.USART_Parity = USART_Parity_No; //��У��
USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx; //�շ�
USART_Init(USART2, &USART_InitStructure); //ʹ������Ĳ�����ʼ��USART2

USART_Cmd(USART2, ENABLE); //ʹ�ܴ���2
}