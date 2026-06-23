#include "main.h"
#include "uart.h"
/*
usart1_init
USART1 的配置 时钟总线是APB2
参数列表:
baudrate:串口波特率
Tx PA9
Rx PA10
*/
void usart1_init(int baudrate)
{
    //定义三个结构体 GPIO USART NVIC
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    //1.配置USART1对应的GPIO
    //时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    //配置结构体
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_9;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF; //复用模式
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL; //悬空输入
    GPIO_Init(GPIOA,&GPIO_InitStruct);
    //配置GPIO的复用功能
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource9,GPIO_AF_USART1);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource10,GPIO_AF_USART1);
    //2.配置USART
    //使能时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
    //初始化USART
    USART_InitStruct.USART_BaudRate = baudrate; // 波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b; //数据位8
    USART_InitStruct.USART_StopBits = USART_StopBits_1; //停止位1
    USART_InitStruct.USART_Parity = USART_Parity_No; //无校验
    USART_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx; //收发模式
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None; //不需要硬件流控制
    USART_Init(USART1,&USART_InitStruct);
    //3.使能中断 如果需要中断
    USART_ITConfig(USART1,USART_IT_RXNE,ENABLE);//接受数据寄存器不为空产生中断
    //4.配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = USART1_IRQn; //串口1的中断请求线
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    //开启串口
    USART_Cmd(USART1,ENABLE);
}
/*
usart1_send_byte 串口1发送1个字节
参数列表:
ch:要发送的那个字节
*/
void usart1_send_byte(char ch)
{
    while(USART_GetFlagStatus(USART1,USART_FLAG_TXE) != SET);
    //确保发送数据之前,TXE是被设置了的
    USART_SendData(USART1,ch);
}
/*
usart1_send_str 串口1发送字符串
参数列表:
s:要发送的字符串
len:要发送的字符串的长度 字节数
*/
void usart1_send_str(char *s,int len)
{
    int i = 0;
    for(i = 0;i < len;i++)
    {
        usart1_send_byte(s[i]);
    }
}
/*
串口1中断,主要就是用来接收数据
需要把接收的数据进行保存,最好自定义一个通信协议
比如:
发送过来的数据
以\r\n结尾
*/

// ---- USART1 环形缓冲区 ----
#define UART1_RX_BUF_SIZE  256
static volatile uint8_t  uart1_rx_buf[UART1_RX_BUF_SIZE];
static volatile uint16_t uart1_rx_head = 0;  // ISR 写入位置
static volatile uint16_t uart1_rx_tail = 0;  // 主循环读取位置

// ISR 中调用，存入一个字节
void USART1_IRQHandler(void)
{
    if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET) {
        uint8_t ch = USART_ReceiveData(USART1);
        uint16_t next = (uart1_rx_head + 1) % UART1_RX_BUF_SIZE;
        if (next != uart1_rx_tail) {       // 缓冲区未满
            uart1_rx_buf[uart1_rx_head] = ch;
            uart1_rx_head = next;
        }
        USART_ClearITPendingBit(USART1, USART_IT_RXNE);
    }
}

// 主循环中调用，返回实际读取的字节数
int uart1_read(uint8_t *dst, int maxlen)
{
    int cnt = 0;
    while (cnt < maxlen && uart1_rx_tail != uart1_rx_head) {
        dst[cnt++] = uart1_rx_buf[uart1_rx_tail];
        uart1_rx_tail = (uart1_rx_tail + 1) % UART1_RX_BUF_SIZE;
    }
    return cnt;
}

// 返回缓冲区中待读取的字节数
int uart1_available(void)
{
    return (uart1_rx_head - uart1_rx_tail + UART1_RX_BUF_SIZE) % UART1_RX_BUF_SIZE;
}

/*
    串口2的初始化
    usart2_init
    串口的
        RX  PA2
        TX  PA3
*/
void usart2_init(int baudrate)
{
    //定义三个结构体 GPIO USART NVIC
    GPIO_InitTypeDef GPIO_InitStruct;
    USART_InitTypeDef USART_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;

    //1.配置USART2对应的GPIO
    //时钟使能
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);
    //配置结构体
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF;   //复用模式
    GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;   //悬空输入
    GPIO_Init(GPIOA,&GPIO_InitStruct);

    //配置GPIO的复用功能
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2);
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2);

    //2.配置USART
    //使能时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
    USART_InitStruct.USART_BaudRate = baudrate;     //      波特率
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;    //数据位8
    USART_InitStruct.USART_StopBits = USART_StopBits_1;     //停止位1
    USART_InitStruct.USART_Parity = USART_Parity_No;        //无校验
    USART_InitStruct.USART_Mode = USART_Mode_Rx|USART_Mode_Tx;      //收发模式
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;    //不需要硬件流控制
    USART_Init(USART2,&USART_InitStruct);

    //3.使能中断 如果需要中断
    USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);//接受数据寄存器不为空产生中断


    //4.配置NVIC
    NVIC_InitStruct.NVIC_IRQChannel = USART2_IRQn;  //串口2的中断请求线
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);

    //开启串口
    USART_Cmd(USART2,ENABLE);
}


/*
    usart2_send_byte 串口2发送1个字节
    参数列表:
        ch:要发送的那个字节
*/
void usart2_send_byte(char ch)
{
    while(USART_GetFlagStatus(USART2,USART_FLAG_TXE) != SET);
    //确保发送数据之前,TXE是被设置了的
    USART_SendData(USART2,ch);
}


/*
    usart2_send_str 串口2发送字符串
    参数列表:
        s:要发送的字符串
        len:要发送的字符串的长度 字节数
*/
void usart2_send_str(char *s,int len)
{
    int i = 0;
    for(i = 0;i < len;i++)
    {
        usart2_send_byte(s[i]);
    }
}

// ---- USART2 环形缓冲区 (二进制协议帧, 主循环解析) ----
#define UART2_RX_BUF_SIZE  256
static volatile uint8_t  uart2_rx_buf[UART2_RX_BUF_SIZE];
static volatile uint16_t uart2_rx_head = 0;
static volatile uint16_t uart2_rx_tail = 0;

void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET) {
        uint8_t ch = USART_ReceiveData(USART2);
        uint16_t next = (uart2_rx_head + 1) % UART2_RX_BUF_SIZE;
        if (next != uart2_rx_tail) {
            uart2_rx_buf[uart2_rx_head] = ch;
            uart2_rx_head = next;
        }
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

int uart2_read(uint8_t *dst, int maxlen)
{
    int cnt = 0;
    while (cnt < maxlen && uart2_rx_tail != uart2_rx_head) {
        dst[cnt++] = uart2_rx_buf[uart2_rx_tail];
        uart2_rx_tail = (uart2_rx_tail + 1) % UART2_RX_BUF_SIZE;
    }
    return cnt;
}

int uart2_available(void)
{
    return (uart2_rx_head - uart2_rx_tail + UART2_RX_BUF_SIZE) % UART2_RX_BUF_SIZE;
}
