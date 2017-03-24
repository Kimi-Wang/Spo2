#include <stdint.h>
#include <string.h>

#include "app_scheduler.h"
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "softdevice_handler.h"
#include "app_timer.h"
#include "app_twi.h"
#include "app_button.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp.h"
#include "bsp_btn_ble.h"
#include "fstorage.h"
#include "SEGGER_RTT.h"
#include "nrf_delay.h"
#include "app_mailbox.h"

#include "commandHandle.h"
#include "flash.h"
//uint8_t Bus_info_res_flag = 0;//ҵ����Ϣ��λ��־����1ʱ�������ҵ����Ϣ��0x21~0x24 

extern const app_mailbox_t  ble_notify_queue ; 
extern struct device_id  device_id_set;

extern  ble_nus_t    m_nus;                                   
bool notify_is_sched = false;

static  notify_free_handler_t notify_free_handler = NULL;

uint32_t sample_info_buffer[10][4]={0};//�洢flash�����ݶ˵���Ϣ����N������+������+���ݶ��ڵĲ�����+���ݵ㿪ʼ��ַ��

static uint32_t tmp_time[3];       //��ǰʱ��Ļ���
static uint8_t rd_size = 0;               //���ڲɼ���buffer��С,��û���ļ��浽flash�С�
static uint32_t ch_rd_data_tm = 5;                                 //�ı�Ĳ�����
static uint32_t cur_rd_data_tm = 5;                                //��ǰʵ�ʲ�����
static uint32_t dt_num = 0;                                 //���ݶ��д洢�����ݸ���

/***************************************************************************************************
 * ��������:
 *          ֪ͨ��Ϣ�����¼�������
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
static void notify_msg_scheduler_event_handler(void *p_event_data, uint16_t event_size)
{
    ret_code_t ret;

    static ble_notify_msg_t msg;
    static int leftone = 0;

    do {
        if(leftone == 1 || (ret = app_mailbox_get(&ble_notify_queue, &msg)) == NRF_SUCCESS)
        {
            if(m_nus.is_notification_enabled != true)
            {
                leftone = 0;
                continue;
            }

            ret = ble_nus_string_send(&m_nus, msg.msg, msg.len);
            if(ret != NRF_SUCCESS)
            {
                leftone = 1; // Send buffer full, next time send it.
            }
            else
                leftone = 0;
        }
        else
        {
            notify_free_handler_t handler = notify_free_handler ;
            if(handler)
                handler();
        }

    } while (ret == NRF_SUCCESS);

    notify_is_sched = false;

}

/***************************************************************************************************
 * ��������:
 *          ֪ͨ��Ϣ���к���
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
static void notify_msg_evt_schedule(int force) //button 
{
    uint32_t ret = 0;

    if(force || (app_mailbox_length_get(&ble_notify_queue) && notify_is_sched == false))
    {
        notify_is_sched = true;		
		//SEGGER_RTT_printf(0,"-->here notify_msg_evt_schedule\r\n");
        ret = app_sched_event_put(&ret, 4, notify_msg_scheduler_event_handler);
        if(ret != NRF_SUCCESS)
            SEGGER_RTT_printf(0,"app_sched_event_put error RETURN %u\r\n", ret);
    }
}

/***************************************************************************************************
 * ��������:
 *          ֪ͨ��Ϣ���ͺ���
 * ����˵����data Ҫ���͵����� len ��Ҫ���͵����ݳ���
 * ����ֵ��  
 **************************************************************************************************/
ret_code_t notify_msg_send(uint8_t *data, int len)
{
    uint32_t err_code;

    ble_notify_msg_t msg;

    if(m_nus.is_notification_enabled != true)
    {
        /* discard it */
        
		//SEGGER_RTT_printf(0,"m_nus.is_notification_enabled = %d\r\n",m_nus.is_notification_enabled);
		return NRF_SUCCESS;
    }

    msg.len = len;
    memcpy(msg.msg, data, len);

    err_code = app_mailbox_put(&ble_notify_queue, &msg);
    if(err_code != NRF_SUCCESS)
    {
        SEGGER_RTT_printf(0,"app_mailbox_put return %d\r\n", err_code);
    }

    notify_msg_evt_schedule(0);
	
	SEGGER_RTT_printf(0,"\r\n--->here notify_msg_send \r\n");
	
    return err_code;
}

/***************************************************************************************************
 * ��������:
 *          ���������������һ���ĸ�ʽ���ͳ�ȥ
 * ����˵����msg_dat  ����������ʼ��ʽ   sample_start_point ������������ʼ������  length ��������������ĳ���
 * ����ֵ��  
 **************************************************************************************************/
void data_transmit_format(uint8_t * msg_dat,uint32_t sample_start_point,uint32_t length)
{
	uint8_t * start_data ;
	char data_segment=0;
	if(sample_start_point !=0 && length != 0 )
	{
		
		while(sample_info_buffer[data_segment][0] != 0)
		{
			if(sample_start_point>sample_info_buffer[data_segment][2])
			{
				sample_start_point = sample_start_point - sample_info_buffer[data_segment][2]; //�ж���ʼ�����ĸ����ݶ���
			}
			else
				break;
			
			data_segment++;
		}
		start_data = (uint8_t *)(((sample_start_point-1)*5)+sample_info_buffer[data_segment][3]);//ָ��Ҫ��������ݵ�		
	}
	else
	{ //������������
		start_data = (uint8_t *)(sample_info_buffer[0][3]);//ָ��Ҫ��������ݵ㣬�׵�ַ
		length = count_total_sample_num((uint32_t  *)0x4b000);//������������
	}
	
	SEGGER_RTT_printf(0,"start addr %p : length :%d\r\n",start_data,length);
	int add_num = 1;
	
	int tmp_Divisible = length/3;
	int tmp_remainder = length%3;
	
	SEGGER_RTT_printf(0,"add_num = %d tmp_Divisible = %d tmp_remainder = %d\r\n",add_num,tmp_Divisible,tmp_remainder);
	
	nrf_delay_ms(3000);
	int a=0;
	
	do{
		SEGGER_RTT_printf(0,"a = %d\r\n",a++);
		char tmp_num =0;
		msg_dat[1] = 0x03;
		char buffer[20] = {0};
		memcpy(buffer,(const char *)msg_dat,4);		
				
		buffer[4] = sample_info_buffer[data_segment][1];//������
		
		if(tmp_Divisible > 0)
		{
			for(tmp_num =0;tmp_num<15;tmp_num++)
			{			
				buffer[5+tmp_num] = *start_data;
				SEGGER_RTT_printf(0,"start_data tmp_Divisible = %d\r\n",*start_data);				
				start_data++;
				if(start_data >= (uint8_t *)((sample_info_buffer[data_segment][2]*5)+sample_info_buffer[data_segment][3]+5))//���ݶ˽�β���´����ݵ�����һ�������ϡ�
				{
					data_segment++;
					start_data = (uint8_t *)sample_info_buffer[data_segment][3];
					if(tmp_num<5)
						add_num +=1;
					else if(tmp_num>4 && tmp_num<10)
						add_num +=2;
					else if(tmp_num>9 &&tmp_num <15)
						add_num +=3;
					
					break;
				}								
			}
			add_num +=3;
			SEGGER_RTT_printf(0,"in tmp_Divisible \r\n");
		
			tmp_Divisible --;
		}
		else
		{
			for(tmp_num =0;tmp_num<tmp_remainder*5;tmp_num++)
			{			
				buffer[5+tmp_num] = *start_data;
				SEGGER_RTT_printf(0,"start_data  tmp_remainder = %d\r\n",*start_data);	
				start_data++;
				if(start_data >= (uint8_t *)((sample_info_buffer[data_segment][2]*5)+sample_info_buffer[data_segment][3]+5))//���ݶ˽�β���´����ݵ�����һ�������ϡ�
				{
					data_segment++;
					start_data = (uint8_t *)sample_info_buffer[data_segment][3];
					if(tmp_num<5)
						add_num +=1;
					else if(tmp_num>4 && tmp_num<10)
						add_num +=2;
					else if(tmp_num>9 &&tmp_num <15)
						add_num +=3;
					
					break;
				}				
			}
			add_num +=3;
			SEGGER_RTT_printf(0,"in tmp_remainder \r\n");
		}
		notify_msg_send((uint8_t*)buffer,20);		
	}while(add_num < length);
}


/***************************************************************************************************
 * ��������:
 *          ͨ�����������͵�ǰflash�е��������ݣ�ָ�����Ȼ���ȫ������
 * ����˵����p_value  ��ָ������   num ����������
 * ����ֵ��  
 **************************************************************************************************/
void data_transmit_continue(void * p_value,uint16_t num) //ͨ�����������͵�ǰflash�е��������ݡ�
{
	uint8_t *msg_dat = (uint8_t *)p_value;
	uint32_t total_sample_num = count_total_sample_num((uint32_t *)0x4b000);//flash�洢���ݿ�ʼ�ĵ�ַ	
	
	SEGGER_RTT_printf(0,"\r\ntotal_sample_num =%d\r\n",total_sample_num);
	
	if(total_sample_num <= 0 || msg_dat[4]>= total_sample_num||((msg_dat[4] +msg_dat[5])>total_sample_num ||  num == 4))//����Ĳ����㲻���ڣ��򳤶Ȳ�����,����û�д��ݲ�����
	{
		msg_dat[1] = 0x02;
		char buffer[20] = {0};
		memcpy(buffer,(const char *)msg_dat,4);
		
		buffer[4] = -3;//��������
		
		notify_msg_send((uint8_t*)buffer,5);
		
		return ;
	}
	else
	{
		SEGGER_RTT_printf(0,"in sent_info\r\n ");
		data_transmit_format(msg_dat,msg_dat[4],msg_dat[5]);
	}
	
}

/***************************************************************************************************
 * ��������:
 *          ��Ϣ���д�������������յ�������
 * ����˵����
 * ����ֵ��  
 **************************************************************************************************/
static void msg_scheduler_event_handler(void *p_event_data, uint16_t event_size)
{
	th_ble_msg_evt_hdr_t *msg = (th_ble_msg_evt_hdr_t *)p_event_data;
    uint8_t msg_len;
    uint8_t * msg_dat;
	
	if(event_size != sizeof(th_ble_msg_evt_hdr_t))
	{
		SEGGER_RTT_printf(0,"Event size error !!! \r\n");
    }
	msg_len = msg->len;
    msg_dat = msg->msg;
	int i =0;
	for(i=0 ;i<msg_len;i++)
		SEGGER_RTT_printf(0,"0x%02x - ",msg_dat[i]);
	SEGGER_RTT_printf(0,"\r\nrecive_command_length : %d\r\n",msg_len);
    /* Do msg */
	
    if(msg_len >= 4 && msg_dat[0] == 0x5a )
	{
		device_id_set = *(struct device_id *)0x4a010;
		
		switch(msg_dat[1])
		{
			case 0x00 :
				
				msg_dat[1] = 0x01;//��Ӧ��
			
				switch(msg_dat[2])
				{
					case 0x01: // �豸���� RO 0x4a010 16Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a010);
						
						//SEGGER_RTT_printf(0,"0x01\r\n");
						
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a010)+4);
							
							
						break;
					}
					case 0x02://���к� RO 0x4a020 16Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a020);
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a020)+4);
						break;
					}
					case 0x03://���� RO 0x4a030 16Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a030);
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a030)+4);
						break;
					}
					case 0x04: //HW�汾�� R0 0x4a040 16Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a040);
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a040)+4);
						break;
					}
					case 0x05://FW�汾�� R0 0x4a050 16Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a050);
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a050)+4);
						break;
					}
					case 0x06://BT�汾�� R0 0x4a060 16Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a060);
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a060)+4);
						break;
					}
					case 0x0A://�洢��Ϣ R0 0x4a070 8Byte 2*U32
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						buffer[4] = device_id_set.device_info.storage[0];
						buffer[5] = device_id_set.device_info.storage[1];						
						notify_msg_send((uint8_t*)buffer,6);
						break;
					}
					case 0x0B://������Ϣ R0 0x4a078 1B u8
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						buffer[4] = *(uint32_t *)0x4a078&0xff;						
						
						notify_msg_send((uint8_t*)buffer,5);
						break;
					}
					case 0x0C: //�豸MAC  R0  0x4a07c 6Byte �ַ���
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						memcpy((char *)buffer+4,(const char *)0x4a07C,6);						
						
						notify_msg_send((uint8_t*)buffer,10);						
						break;
					}
					case 0x0D: //�豸״̬���ݶ�16Byte�ַ���������ĳ�0xff��
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a084);						
						
						notify_msg_send((uint8_t*)buffer,10);						
						break;
					}
					case 0x21: //����ҵ��ID  0x4a094 RW 16Byte U32
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a094);						
						
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a094)+4);						
						break;
					}
					case 0x22://��������  0x4a098 RW 16Byte UTF-8
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a098);						
						
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a098)+4);						
						break;
					}
					case 0x23: //������Ϣ  0x4a0a8  RW 7Byte 
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						memcpy((char *)buffer+4,(const char *)0x4a0a8+1,7);						
						
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a0a8)+4);						
						break;
					}
					case 0x24://������  0x4a0b0 RW 16Byte UTF-8
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						strcpy((char *)buffer+4,(const char *)0x4a0b0);						
						
						notify_msg_send((uint8_t*)buffer,strlen((const char *)0x4a0b0)+4);						
						break;
					}
					case 0x30://ҵ��״̬ 0x4a0c0  RW 1Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						buffer[4] = *(uint32_t *)0x4a0c0&0xff;
						notify_msg_send((uint8_t*)buffer,5);						
						break;
					}
					case 0x31: //����ģʽ 0x4a0c4 RW 1Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						buffer[4] = *(uint32_t *)0x4a0c4&0xff;
						notify_msg_send((uint8_t*)buffer,5);						
						break;
					}
					case 0x32://���ݲɼ�ģʽ 0x4a0c8 RW 1Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						buffer[4] = *(uint32_t *)0x4a0c8&0xff;
						notify_msg_send((uint8_t*)buffer,5);						
						break;
					}
					case 0x33: //���ܿ���  0x4a0e8  RW 1Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						buffer[4] = *(uint32_t *)0x4a0e8&0xff;
						notify_msg_send((uint8_t*)buffer,5);						
						break;
					}
					case 0x34:// ���ݲ����� 0x4a0e4  RW 2Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						buffer[4] = *(uint32_t *)0x4a0e4&0xff00;
						buffer[5] = *(uint32_t *)0x4a0e4&0xff;
						
						notify_msg_send((uint8_t*)buffer,6);						
						break;
					}
					case 0x37://�豸����ʱ�� 0x4a0cc Ro 9Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						memcpy((char *)buffer+4,(const char *)0x4a0cc,9);
						
						notify_msg_send((uint8_t*)buffer,13);						
						break;
					}
					case 0x38:// ��ǰʱ�� 0x4a0d8 RW 9Byte
					{
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);					
											
						memcpy((char *)buffer+4,(const char *)0x4a0d8,9);
						
						notify_msg_send((uint8_t*)buffer,13);						
						break;
					}
					
					case 0x20:
					case 0x50:
					case 0x51:
					case 0xC0:
					case 0xC1:
					case 0xC2:
					case 0xC3:
					case 0xC4: //д����
					{	
						msg_dat[1] = 0x00;
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						buffer[4] = -3;//��������
						
						notify_msg_send((uint8_t*)buffer,5);
					}
						
					break;
					
					default:
					{
						msg_dat[1] = 0x00;
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						buffer[4] = -2;//��������
						
						notify_msg_send((uint8_t*)buffer,5);
					}
						break;
						
				}
			break; // read command
				
			case 0x02:
			{		
				//2
				msg_dat[1] = 0x03;//дӦ��
//				memset(&device_id_set,0,sizeof(device_id_set));	
				switch(msg_dat[2]) 
				{
					
					case 0x20 ://ҵ��λ
						
						memset(&device_id_set.bus_mode,0,sizeof(device_id_set.bus_mode));
						device_id_set.bus_mode.bus_comm.running_status = 0x00; //U8 0 -- IDLE ,1 -- Run ,2-- Stop
						device_id_set.bus_mode.bus_comm.running_mode = 0x00;
						device_id_set.bus_mode.bus_comm.sample_mode = 0x00;					
						device_id_set.bus_mode.bus_comm.function_switch = 0x00;
						device_id_set.bus_mode.bus_comm.sample_rate = 0x80005;						
						
						device_id_change(&device_id_set.bus_mode,2);
						SEGGER_RTT_printf(0,"clean\r\n");
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break;
					
					case 0x21://ҵ��ID
						
						memset(&device_id_set.bus_mode.bus_info.command_id,0,sizeof(device_id_set.bus_mode.bus_info.command_id));
						memcpy((char *)&device_id_set.bus_mode.bus_info.command_id,(const char *)msg_dat+4,msg_len-4);
					
						device_id_change(&device_id_set.bus_mode,2);

						notify_msg_send((uint8_t*)msg_dat,4);
						break ;
					
					case 0x22://��������
						memset(&device_id_set.bus_mode.bus_info.user_name,0,sizeof(device_id_set.bus_mode.bus_info.user_name));
						memcpy((char *)&device_id_set.bus_mode.bus_info.user_name,(const char *)msg_dat+4,msg_len-4);
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
						break ;
					
					case 0x23://������Ϣ
						memset(&device_id_set.bus_mode.bus_info.user_info,0,sizeof(device_id_set.bus_mode.bus_info.user_info));						
						memcpy((char *)&device_id_set.bus_mode.bus_info.user_info,(const char *)msg_dat+4,msg_len-4);
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
						break ;
					
					case 0x24://������
						memset(&device_id_set.bus_mode.bus_info.user_number,0,sizeof(device_id_set.bus_mode.bus_info.user_number));						
						memcpy((char *)&device_id_set.bus_mode.bus_info.user_number,(const char *)msg_dat+4,msg_len-4);
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break ;
					
					case 0x30://ҵ��״��
						memset(&device_id_set.bus_mode.bus_comm.running_status,0,sizeof(device_id_set.bus_mode.bus_comm.running_status));						
						device_id_set.bus_mode.bus_comm.running_mode = msg_dat[4];
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break ;
					
					case 0x31://����ģʽ
						memset(&device_id_set.bus_mode.bus_comm.running_mode,0,sizeof(device_id_set.bus_mode.bus_comm.running_mode));						
						device_id_set.bus_mode.bus_comm.running_mode = msg_dat[4];
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break ;
					case 0x33://���ܿ���
						memset(&device_id_set.bus_mode.bus_comm.function_switch,0,sizeof(device_id_set.bus_mode.bus_comm.function_switch));						
						device_id_set.bus_mode.bus_comm.function_switch = msg_dat[4];	
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break;
					
					case 0x38://��ʱ
						memset(&device_id_set.bus_mode.bus_comm.time_calibration,0,sizeof(device_id_set.bus_mode.bus_comm.time_calibration));						
						memcpy((char *)&device_id_set.bus_mode.bus_comm.time_calibration,(const char *)msg_dat+4,msg_len-4);
						
						device_id_change(&device_id_set.bus_mode,2);
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break ;
					
					case 0x50://������������  ����дflash					
						
						data_transmit_continue(msg_dat,msg_len);
						//app_sched_event_put(&msg_dat,sizeof(msg_dat),data_transmit_continue);					
						
						break ;
					
					case 0x51://�������  ����дflash
						erase_all_data_flag = 1;
						device_id_change(NULL,5);
						notify_msg_send((uint8_t*)msg_dat,4);
					
						break ;
					
					case 0xC0://����ģʽ����  ����дflash
						
					
						break ;
					
					case 0xC1://����ģʽ�˳�  ����дflash
						
					
						break ;
					
					case 0xC2://д���豸SN 	����дflash
						
					
						break ;
					
					case 0xC3://д���豸����  ����дflash
						
					
						break ;
					
					case 0xC4://����Firmware  ����дflash
						
					
						break ;
					
					case 0x01:
					case 0x02:
					case 0x03:
					case 0x04:
					case 0x05:
					case 0x06:
					case 0x07:
					case 0x08:
					case 0x09:
					case 0x0A:
					case 0x0B:
					case 0x0C:
					case 0x0D:
					case 0x32:
					case 0x34:
					case 0x37:
					{
						msg_dat[1] = 0x02;
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						buffer[4] = -3;//��������
						
						notify_msg_send((uint8_t*)buffer,5);
					}
					
					break;
					
					default :
					{
						msg_dat[1] = 0x02;
						char buffer[20] = {0};
						memcpy(buffer,(const char *)msg_dat,4);
						
						buffer[4] = -2;//��������
						
						notify_msg_send((uint8_t*)buffer,5);
					}
					break;
				}
					
			}
			break;
			
			default :
			{
				msg_dat[1] = 0xff;
				char buffer[20] = {0};
				memcpy(buffer,(const char *)msg_dat,4);
				
				buffer[4] = -2;//��������
				
				notify_msg_send((uint8_t*)buffer,5);
			}
			break;
				
		}
	}
	else
	{
		SEGGER_RTT_printf(0,"Invalid command !!!\r\n");
		char buffer[20] = {0};
		memcpy(buffer,(const char *)msg_dat,4);
		
		buffer[4] = -4;//��Ч���
		
		notify_msg_send((uint8_t*)buffer,5);
	}
}

/***************************************************************************************************
 * ��������:
 *          ��ȡ��ǰflash�д洢�����ݸ���,���ݸ���ͨ������ֵ���أ����ݶ�ʱ��ͨ�����εõ�
 * ����˵����total_second  ���ܵ�ʱ��
 * ����ֵ��  total_samples_num ���ܵĲ������� 
 **************************************************************************************************/
static uint32_t get_samples_num_sec(uint32_t *total_second)
{
		uint16_t *samp_start_addr = (uint16_t *)0x4b003; //���ݵĴ洢��ַ  �洢��ʽ����ʼ��ַ��0x4B000����ʱ�� 12bit��+���������� 32bit��+�������� 8bit��+��spo2 8bit��+��pr 8bit��+��pvi 16bit��+��pi 8bit��
	
		uint32_t cur_num = 0;
		uint32_t total_samples_num = 0;
		uint16_t rd_data_rate = 0;
		
		cur_num = (uint32_t)(*(((uint32_t *)samp_start_addr)));
		
		while(cur_num != 0xffffffff)
		{
				rd_data_rate = *(samp_start_addr+2);													//��ȡ��������
			
				total_samples_num += cur_num;															//�ܳ��ȼ��ϴ˶����ݶεĳ���
				//samp_start_addr += 3 + cur_num;															//��ַָ����һ�����ݶ�
				samp_start_addr += 3 + cur_num*5;					//��ַָ����һ�����ݶ� 
			
				*total_second +=  rd_data_rate*cur_num;													//��ʱ����ϴ˶������õ�ʱ��
			
				cur_num = (uint32_t)(*(((uint32_t *)samp_start_addr)));		//��ȡ������
		}//���������ݶ�
		
		//��������Ե�ǰ���������ڲ���
		
		rd_data_rate = *(samp_start_addr+2);
		samp_start_addr += 3;
		cur_num = 0;
																									//��ֵΪ0��������¼���һ�����ݵĵ�ǰ����
		uint8_t * tmpaddr  = (uint8_t *)samp_start_addr;
		while(*tmpaddr != 0xff)
		{
				cur_num++;
				total_samples_num++;
				tmpaddr = tmpaddr + 5;
		}
		
		*total_second +=  rd_data_rate*(cur_num+rd_size/5);  //����ʱ��ʱ�������ڼ��������Ҫ�������ڲ����ĵ�rd_size
		return total_samples_num;
}


/***************************************************************************************************
 * ��������:
 *          ��ȡ��ʼʱ��
 * ����˵����cur_time  �����ݵĵ�ǰʱ��
 * ����ֵ��  
 **************************************************************************************************/
static void get_start_time(uint8_t *cur_time)
{
		uint8_t  sub_day;							 //�����ݴ��ɺ���ת���ɵ�������Сʱ�����ӡ��롢����
		uint8_t sub_hour;
		uint8_t sub_minute;
		uint8_t sub_second;
		uint16_t sub_msec;
	
		uint16_t year = (cur_time[0] << 8) | cur_time[1];		//�����������ֻ���������ʱ��
		uint8_t month = cur_time[2];
		uint8_t  day = cur_time[3];
		uint8_t hour = cur_time[4];
		uint8_t minute = cur_time[5];
		uint8_t second = cur_time[6];
		uint16_t msec = (cur_time[7] << 8) | cur_time[8];
	
		uint32_t total_second = 0;								
		get_samples_num_sec(&total_second);						//����豸���ʱ��
		
		SEGGER_RTT_printf(0,"rd_size = %d------\r\n",total_second);
		total_second = (total_second)*1000;			//��ת���ɺ���
		sub_day = total_second / (1000*60*60*24);
		sub_hour = total_second % (1000*60*60*24) / (1000*60*60);
		sub_minute = total_second % (1000*60*60*24) % (1000*60*60) / (60*1000);
		sub_second = total_second % (1000*60*60*24) % (1000*60*60) % (60*1000) / 1000;
		sub_msec = total_second % (1000*60*60*24) % (1000*60*60) % (60*1000) % 1000;
		
		if(msec < sub_msec)
		{
				msec = msec+1000-sub_msec;
				sub_second++;
		}
		else
				msec = msec- sub_msec;
		if(second < sub_second)
		{
				second = second+60-sub_second;
				sub_minute++;
		}
		else
		{
				second = second-sub_second;
		}
		if(minute < sub_minute)
		{
				minute = minute+60-sub_minute;
				sub_hour++;
		}
		else
		{
				minute = minute-sub_minute;
		}
		if(hour < sub_hour)
		{
				hour = hour+24-sub_hour;
				sub_day++;
		}
		else
		{
				hour = hour-sub_hour;
		}
		if(day <= sub_day)
		{
				if(month-1 ==0 || month-1 == 3 || month-1 == 5|| month-1 == 7 || month-1 == 8 || month-1 == 10 || month-1 == 1)
				{
								day = day+31-sub_day;
								if(month == 1)
								{
										month = 12;
										year--;
								}
								else
										month--;
				}
				else if(month-1 ==4 || month-1 == 6 || month-1 == 9|| month-1 == 11)
				{
							 day = day+30-sub_day;
							 month--;
				}
				else
				{
								if((year%4 == 0 && year%100 != 0) || year%400 == 0)
								{
												day = day+29-sub_day;
												month--;
								}
								else
								{
												day = day+28-sub_day;
												month--;;
								}
				}
		}
		else
				day = day-sub_day;
	
		tmp_time[0] = (year << 16) | (month << 8) | day;
		tmp_time[1] = (hour<<24) | (minute<<16) | (second<< 8) | (msec>>8);
		tmp_time[2] = msec & 0xff;
		SEGGER_RTT_printf(0,"Set date: %04u-%02u-%02u %02u:%02u:%02u.%03u\r\n",year, month, day,hour, minute, second, msec);
}



/***************************************************************************************************
 * ��������:
 *          ����һ�����ݶ��ڵĲ�������һ���ǵ�һ�����һ�ˣ�
 * ����˵����data_addr  �����ݶ˵���ʼ��ַ
 * ����ֵ��  total_sample_num ���ܵĲ������� 
 **************************************************************************************************/
int get_smaple_num(uint8_t * data_addr)//����һ�����ݶ��ڵĲ�������һ���ǵ�һ�����һ�ˣ�
{
	uint32_t total_sample_num = 0;
		//SEGGER_RTT_printf(0,"\r\n Cross the border%p \r\n",data_addr);
	if(data_addr >= (uint8_t *)0x80000)
	{
		SEGGER_RTT_printf(0,"\r\n Cross the border \r\n");		
		return -1;
	}
	SEGGER_RTT_printf(0,"*data_addr start : %p = %d\r\n",data_addr,*data_addr);
	while(data_addr != (uint8_t *)0x80000)
	{
		if(*data_addr == 0xff)
		{
			if(*(uint16_t *)data_addr == 0xffff)
			{
				break;
			}
		}
		data_addr++;
		total_sample_num++;
	}
	SEGGER_RTT_printf(0,"*data_addr end : %p = %d\r\n",data_addr,*data_addr);
	SEGGER_RTT_printf(0,"total_sample_num :%d\r\n",total_sample_num);
	total_sample_num = (total_sample_num/20)*4;//ÿһ������4�������㡣
	return 	total_sample_num;
}

/***************************************************************************************************
 * ��������:
 *          ���flash���ܵĲ���������
 * ����˵����start_storage_addr  �����ݶ˵���ʼ��ַ
 * ����ֵ��  total_sample_num ���ܵĲ������� 
 **************************************************************************************************/
uint32_t count_total_sample_num(uint32_t * start_storage_addr)// ���flash���ܵĲ���������
{	
	memset(sample_info_buffer,0,sizeof(sample_info_buffer));
	uint8_t data_end_falg = 0;
	uint32_t total_sample_num = 0;
	uint32_t * data_star_addr =(uint32_t *)start_storage_addr;//ָ��������ʼλ��
	data_star_addr = data_star_addr + 1;//����ʱ��ָ���������
	
	memset(sample_info_buffer,0,sizeof(sample_info_buffer));
	while(data_end_falg == 0)
	{	
		SEGGER_RTT_printf(0,"data_star_addr(%p) = 0x%x\r\n",data_star_addr,*data_star_addr);
		if(*data_star_addr == 0xffffffff)//ֻ��һ������
		{
			total_sample_num = get_smaple_num((uint8_t*)(data_star_addr+2));
			SEGGER_RTT_printf(0,"\r\n %p \r\n",(uint8_t*)(data_star_addr+2));
			sample_info_buffer[0][0] = 1;
			sample_info_buffer[0][1] = *(data_star_addr+1);
			sample_info_buffer[0][2] = total_sample_num;
			sample_info_buffer[0][3] = (uint32_t)(data_star_addr+2);	
			
			data_end_falg = 1;
			break;
		}
		else
		{
			char sample_data_num = 0;
			
			while(*data_star_addr != 0xffffffff)
			{
				total_sample_num = total_sample_num+(*data_star_addr);
					
				sample_info_buffer[sample_data_num][0] = sample_data_num+1;
				sample_info_buffer[sample_data_num][1] = *(data_star_addr+1);
				sample_info_buffer[sample_data_num][2] = *data_star_addr;
				sample_info_buffer[sample_data_num][3] = (uint32_t)(data_star_addr+2);	
				
				data_star_addr = (*data_star_addr)*5+data_star_addr+5/*�������ݶ�ĩβ��20�ֽڣ�����ͻȻ����û�������Ĵ�����Ϣ��*/+3/*���������ʺͲ��������Լ��¸����ݶ˵�ʱ��*/;	//ָ����һ������������	
				sample_data_num++;
				
			}
			
			sample_data_num++;
			
			data_star_addr = data_star_addr+2;//ָ�����һ�����ݶ˵����ݵ�λ��
			char end_sample_num = get_smaple_num((uint8_t*)data_star_addr);
			
			sample_info_buffer[sample_data_num][0] = sample_data_num+1;
			sample_info_buffer[sample_data_num][1] = *(data_star_addr-1);
			sample_info_buffer[sample_data_num][2] = end_sample_num;
			sample_info_buffer[sample_data_num][3] = (uint32_t)(data_star_addr);
			total_sample_num += end_sample_num;
			
			data_end_falg = 1;
			break;			
		}
	} 
	
	int  n = 0;
	while(sample_info_buffer[n][0] != 0)
	{
		int m = 0;
		for(m=0 ;m<4; m++)
			SEGGER_RTT_printf(0,"sample_info_buffer[%d][%d] = 0x%02x \r\n ",n,m,sample_info_buffer[n][m]);
		SEGGER_RTT_printf(0,"\r\n");
		n++;
	}
	
	return total_sample_num;
}


/***************************************************************************************************
 * ��������:
 *          ��Ϣ�¼���������
 * ����˵����
 * ����ֵ��  
 **************************************************************************************************/
void msg_event_handler(uint8_t *p_data, uint16_t length)
{
	th_ble_msg_evt_hdr_t msg;
    uint32_t ret;

    msg.len = length;

    memcpy(&msg.msg[0], p_data, length);

    SEGGER_RTT_printf(0,"msg_event_handler_length: %d\r\n",length);

    ret = app_sched_event_put(&msg, sizeof(th_ble_msg_evt_hdr_t), msg_scheduler_event_handler);
	if(ret != 0)
		SEGGER_RTT_printf(0,"app_sched_event_put RETURN %u\r\n", ret);
	
}
