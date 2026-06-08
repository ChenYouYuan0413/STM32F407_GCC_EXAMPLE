#ifndef __UART_H__
#define __UART_H__




void usart1_init(int baudrate);
/*
usart1_send_byte 串口1发送1个字节
参数列表:
ch:要发送的那个字节
*/
void usart1_send_byte(char ch);

/*
usart1_send_str 串口1发送字符串
参数列表:
s:要发送的字符串
len:要发送的字符串的长度 字节数
*/
void usart1_send_str(char *s,int len);
/*
    串口2的初始化
    usart2_init
    串口的
        RX  PA2
        TX  PA3
*/
void usart2_init(int baudrate);

/*
    usart2_send_byte 串口1发送1个字节
    参数列表:
        ch:要发送的那个字节
*/
void usart2_send_byte(char ch);

/*
    usart2_send_str 串口1发送字符串
    参数列表:
        s:要发送的字符串 
        len:要发送的字符串的长度 字节数
*/
void usart2_send_str(char *s,int len);



#endif
