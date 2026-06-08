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

void USART1_IRQHandler(void)
{
    unsigned char ch; //接受数据的变量
    if(USART_GetITStatus(USART1,USART_IT_RXNE) == SET)
    {
        ch = USART_ReceiveData(USART1); //从串口1接受数据
        if(ch == '1')
        {
            //亮灯
        }
        if(ch == '2')
        {
            //灭灯
        }
        USART_ClearITPendingBit(USART1,USART_IT_RXNE);
    }
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

    //1.配置USART1对应的GPIO
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
    usart2_send_byte 串口1发送1个字节
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
    usart2_send_str 串口1发送字符串
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


/*
    串口2中断,主要就是用来接受数据
    需要把接受的数据进行保存,最好自定义一个通信协议
    比如:
        发送过来的数据
        以\r\n结尾
*/
char r_buf[512] = {0};   //保存接受的字符串

int cnt = 0;   //接受的数量
int smoke = 0;
void USART2_IRQHandler(void)
{
    unsigned char ch;   //接受数据的变量

    if(USART_GetITStatus(USART2,USART_IT_RXNE) == SET)//获取串口中断标志
    {   
        //处理GY39发送回来的数据
        USART_ClearITPendingBit(USART2,USART_IT_RXNE);
        ch  = USART_ReceiveData(USART2);//从串口2接受数据
        r_buf[cnt] = ch;//把数据存入到数组中
        cnt++;
        //什么时候数据接受完毕,数据接受完成了 几首温度 气压海拔 湿度 总共需要接受15个数据
        if(cnt == 9)
        {

						smoke = r_buf[2]<<8|r_buf[3];
					cnt = 0;

        }

    }
}
