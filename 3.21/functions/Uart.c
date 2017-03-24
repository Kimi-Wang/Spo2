#include "Uart.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "app_uart.h"
#include "app_error.h"
#include "bsp.h"
         
#define UART_TX_BUF_SIZE    256                       
#define UART_RX_BUF_SIZE    256

#define UART_RX_PIN     8
#define UART_TX_PIN     6
#define UART_CTS_PIN    7
#define UART_RTS_PIN    5

/***************************************************************************************************
* ��������:
*          UART�¼�����
* ����˵����
* ����ֵ��
**************************************************************************************************/
static void uartEventHandle(app_uart_evt_t * p_event)
{

}

/***************************************************************************************************
* ��������:
*          UART��ʼ��
* ����˵����
* ����ֵ��
**************************************************************************************************/
void uartInit(void)
{
    unsigned int err_code;
    
    const app_uart_comm_params_t comm_params =
    {
        UART_RX_PIN,
        UART_TX_PIN,
        UART_RTS_PIN,
        UART_CTS_PIN,
        APP_UART_FLOW_CONTROL_DISABLED,
        false,
        UART_BAUDRATE_BAUDRATE_Baud115200
    };

    APP_UART_FIFO_INIT( &comm_params,
                       UART_RX_BUF_SIZE,
                       UART_TX_BUF_SIZE,
                       uartEventHandle,
                       APP_IRQ_PRIORITY_MID,
                       err_code);
    APP_ERROR_CHECK(err_code);
}

/***************************************************************************************************
* ��������:
*          UART����һ���ֽ�
* ����˵����
*          ch������
* ����ֵ��
**************************************************************************************************/
static void sendChar(unsigned char ch)
{
    app_uart_put(ch);
} 

/**************************************************************************************************
 * ���ܣ�UART����һ�����ݡ�
 * ������
 * 		buff������
 * 		len�����ݳ��ȡ�
 *************************************************************************************************/
void send(const unsigned char *buff, short int len)
{
    if (buff == NULL || len == 0)
    {
        return;
    }

    while (len--)
    {
        sendChar(*buff++);
    }
}


