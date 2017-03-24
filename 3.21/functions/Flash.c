#include "Flash.h"
#include "SEGGER_RTT.h"
#include "fstorage.h"
#include <stdint.h>
#include <string.h>
#include "stdio.h"
#include "app_scheduler.h"
#include "nrf_delay.h"

#include "commandHandle.h"

struct device_id  device_id_set;

uint8_t * data_end_addr = NULL;
uint8_t erase_all_data_flag = 0;
uint32_t test_buffer[25] = {0};
uint32_t sample_data_buffer[3] = {0};
uint32_t first_init_flag = 0x01;

/***************************************************************************************************
* ��������:
*          flash_handler��ʼ��
* ����˵����evt �� fstorage event �ṹ�� �� result �������
* ����ֵ��
**************************************************************************************************/
static void fstorage_handler(fs_evt_t const * const evt, fs_ret_t result)
{
	if(evt->id == 0)//д����
	{
		SEGGER_RTT_printf(0,"data_length %d\r\n",evt->store.length_words);
		
//		if(evt->store.length_words == 1)
//		{
//			SEGGER_RTT_printf(0,"<----data : %d\r\n",*(uint32_t *)evt->store.p_data);
//			if((*(uint32_t *)0x49000) == 0x01 )
//			{
//				first_init_flag = 0;
//				SEGGER_RTT_printf(0,"write first flag success \r\n");				
//			}		

//		}	
		
		SEGGER_RTT_printf(0,"device_id_init.device_type : %s\r\n",(char *)0x4a010);
		SEGGER_RTT_printf(0,"device_id_init.device_sequence : %s\r\n",(char *)0x4a020);
		SEGGER_RTT_printf(0,"device_id_init.device_batch : %s\r\n",(char *)0x4a030);
		SEGGER_RTT_printf(0,"device_id_init.sample_rate : 0x%08x\r\n",*(uint32_t *)0x4a0e4);
	}
	else
	{
		if(erase_all_data_flag == 1)
		{
			SEGGER_RTT_printf(0,"erase all \r\n");
			SEGGER_RTT_printf(0,"erase start %d  end %d\r\n",evt->erase.first_page,evt->erase.last_page);
			
			erase_all_data_flag = 0;
		}
	}

}

/***************************************************************************************************
* ��������:
*          Fstorage Param Init Macro
* ����˵����
* ����ֵ��
**************************************************************************************************/
FS_REGISTER_CFG(fs_config_t fs_config)  =
{
	.callback  = fstorage_handler,
	.num_pages = 55,   //�����һҳ��ʼ�㣬0x49000 
	.p_start_addr = (uint32_t *)(0x49000),
	.p_end_addr   = (uint32_t *)(0x7f000),
	
	// We register with the highest priority in order to be assigned
	// the pages with the highest memory address (closest to the bootloader).
	.priority  = 0xFE,
};

/***************************************************************************************************
* ��������:
*          flashInit��������
* ����˵����value Ҫд����е����� �� style Ҫ�ı������
* ����ֵ��
**************************************************************************************************/
void flashInit(void * p_event_data, uint16_t event_size)
{
	uint32_t err_code;
	
	err_code = fs_init();
	
	//SEGGER_RTT_printf(0,"mac %s\r\n",device_id_set.device_info.mac);
	
	err_code = fs_init();	
			
	uint32_t * first_run_flag = (uint32_t *)0x49000;
	if(*first_run_flag == 0xffffffff)
	{	
		
		err_code =  fs_erase(&fs_config, (uint32_t *)0x49000,2,NULL);
		
		if( err_code != 0)
		{
			SEGGER_RTT_printf(0,"device_id_operate.c device_ID_init fs_init error \r\n");
		}
		//��һ������ʱĬ�ϵ��趨ֵ
		strcpy((char *)device_id_set.device_info.device_type,"TBOS03");
		strcpy((char *)device_id_set.device_info.device_sequence,"2016P030000001");
		strcpy((char *)device_id_set.device_info.device_batch,"2016P03");
		strcpy((char *)device_id_set.device_info.hw_version,"Nordic52832");
		strcpy((char *)device_id_set.device_info.fw_version,"SDK12.0.0_12f24");
		strcpy((char *)device_id_set.device_info.bt_version,"BT4.2");		
		
		device_id_set.device_info.storage[0] = 0x52d0; device_id_set.device_info.storage[1] = 0x52d0;//total 53 page 212.00KByte��
		device_id_set.bus_mode.bus_comm.running_mode = 0x00;//����ģʽ ���ȴ�����������
		device_id_set.bus_mode.bus_comm.sample_mode = 0x00; //����ģʽ ���Զ�������
		device_id_set.bus_mode.bus_comm.function_switch = 0x00;// ���ܿ��� :�ݻ����ر�
		device_id_set.bus_mode.bus_comm.sample_rate = (uint32_t)0x8005;	//�������ʣ�5s
		
		
		SEGGER_RTT_printf(0,"frist ID_init -- \r\n");
		
		err_code = fs_store(&fs_config, (uint32_t *)0x4a010,(uint32_t *)&device_id_set,(sizeof(device_id_set)/4),NULL);
			SEGGER_RTT_printf(0,"first fs_store :err_code: %d\r\n",err_code);		
		
		
		uint32_t first_init_flag = 0x01;
		err_code = fs_store(&fs_config, (uint32_t *)0x49000,&first_init_flag,1,NULL);	
		if(err_code != 0)
			SEGGER_RTT_printf(0,"device_id_operate.c device_ID_init  fs_store struct error\r\n",err_code);
	}
	else
	{
		device_id_set = *(struct device_id *)0x4a010;
		
		for(err_code = 0;err_code<25;err_code++)
		{
			test_buffer[err_code] = 0xff00+err_code;
		}
		test_buffer[1] = 0xffffffff;
		test_buffer[2] = 0x8005;
		
		err_code =  fs_erase(&fs_config, (uint32_t *)0x4a000,3,NULL);
		if(err_code != 0)
			SEGGER_RTT_printf(0,"fs_erase error\r\n",err_code);	
		
		err_code =  fs_store(&fs_config, (uint32_t *)0x4b000,(uint32_t *)test_buffer,sizeof(test_buffer)/4,NULL);
		if(err_code != 0)
			SEGGER_RTT_printf(0,"fs_erase error\r\n",err_code);			

		SEGGER_RTT_printf(0,"second ID_init --\r\n");
		
		
		
		err_code = fs_store(&fs_config, (uint32_t *)0x4a010,(uint32_t *)&device_id_set,(sizeof(device_id_set)/4),NULL);
		if(err_code != 0)
			SEGGER_RTT_printf(0,"second fs_store :err_code: %d\r\n",err_code);		
		
		memset(sample_info_buffer,0,sizeof(sample_info_buffer));		
		count_total_sample_num((uint32_t *)0x4b000);
		
		/*�����ϴεĲ�����*/
		int tmp =0;
		while(sample_info_buffer[tmp][0] != 0)
		{
			tmp++ ;			
		}		

		err_code = fs_store(&fs_config, (uint32_t *)sample_info_buffer[tmp-1][3] - 2,(uint32_t *)sample_info_buffer[tmp-1][2],1,NULL);//�ϵ�����һ�εĴ洢���ݶΣ��������������洢�����λ�õ㡣
		if(err_code != 0)
			SEGGER_RTT_printf(0,"write sample count error %d\r\n",err_code);

		
		/*����µ����ݶ�*/

		sample_data_buffer[0] = 0x20160313;//RTCʱ��
		sample_data_buffer[1] = 0x20160313;//�̶�������
		sample_data_buffer[2] = device_id_set.bus_mode.bus_comm.sample_rate;//������
		
		SEGGER_RTT_printf(0,"%d 0x%x\r\n",tmp,((uint8_t *)sample_info_buffer[tmp-1][3] + sample_info_buffer[tmp-1][2]*5+20));
		
		err_code = fs_store(&fs_config,(uint32_t *)((uint8_t *)sample_info_buffer[tmp-1][3] + sample_info_buffer[tmp-1][2]*5+20),sample_data_buffer,3,NULL);
		if(err_code !=0 )
		{
			SEGGER_RTT_printf(0,"write next sample data side error %d\r\n",err_code);
		}
		
		//����flash����	
		data_end_addr = (uint8_t * )0x4b000;
		while(data_end_addr != (uint8_t *)0x80000)
		{
			if(*data_end_addr == 0xff)
			{
				if(*((uint32_t *)data_end_addr + 5) == 0xffffffff)
				{
					break;
				}
			}
			data_end_addr++;			 
		}
		
		device_id_set.device_info.storage[1] =(int)(((float)( 0x80000 - (uint32_t )data_end_addr)/ 1024.0)*100.0); //��λ����
			SEGGER_RTT_printf(0,"flash_storage %d\r\n",device_id_set.device_info.storage[1]);
		//end����flash����
				err_code = fs_store(&fs_config, (uint32_t *)0x4a010,(uint32_t *)&device_id_set,(sizeof(device_id_set)/4),NULL);
		if(err_code != 0)
			SEGGER_RTT_printf(0,"second fs_store :err_code: %d\r\n",err_code);
	}
}

/***************************************************************************************************
* ��������:
*          device_id_change��������
* ����˵����value Ҫд����µ����� �� style Ҫ�ı������
* ����ֵ��
**************************************************************************************************/
void device_id_change(void * value,uint16_t style)
{
	struct device_id id_change_set;
	id_change_set = device_id_set;
	if(style != 5)
	{
		uint8_t error =  fs_erase(&fs_config, (uint32_t *)0x4a000,1,NULL);	
			if(error != 0)
			{
				SEGGER_RTT_printf(0,"1 fs_store :err_code: %d\r\n",error);	
				//return;
			}				
		if(style == 2)//����ҵ����Ϣ
		{			
			memset(&id_change_set.bus_mode,0xff,sizeof(id_change_set.bus_mode));	
		
			memcpy(&id_change_set.bus_mode,value,sizeof(id_change_set.bus_mode));
		}
		
		if(style == 0)//�����豸��Ϣ
		{
			memset(&id_change_set.device_info,0xff,sizeof(id_change_set.device_info));
			memcpy(&id_change_set.device_info,value,sizeof(id_change_set.device_info));
		}
					
		uint8_t err_code = fs_store(&fs_config, (uint32_t *)0x4a010,(uint32_t *)&device_id_set,(sizeof(device_id_set)/4),NULL);
		if(err_code != 0)
		{
			SEGGER_RTT_printf(0,"2 fs_store :err_code: %d\r\n",err_code);
			//return;
		}
		
		//app_sched_event_put(&device_id_set,sizeof(device_id_set),device_id_change);
	}
	else
	{
		if(erase_all_data_flag == 1)
		{
			uint8_t error =  fs_erase(&fs_config, (uint32_t *)0x4b000,34,NULL);	
			if(error != 0)
			{
				SEGGER_RTT_printf(0,"erase_all_data :err_code: %d\r\n",error);		
				//return;
			}
			SEGGER_RTT_printf(0,"erase_all_data \r\n");
					
		}
	}
}
