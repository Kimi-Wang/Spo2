#ifndef _FLASH_H
#define _FLASH_H 

#include <stdint.h>
#include "fstorage.h"
struct device_id
{
	//�豸��Ϣ��
	struct 
	{
		uint32_t * device_type[4]; 			//�豸����   16Byte 			ASCII�ַ������16�ַ�	��ʼ��ַ��	0x4a010 
		uint32_t * device_sequence[4];  	//�豸���к� 16Byte			ASCII�ַ������16�ַ�			 	0x4a020
		uint32_t * device_batch[4];			//�豸����   16Byte			ASCII�ַ������16�ַ�			  	0x4a030
		uint32_t * hw_version[4];			//�豸HW�汾�� 16Byte		ASCII�ַ������16�ַ�				0x4a040
		uint32_t * fw_version[4];			//�豸FW�汾�� 16Byte		ASCII�ַ������16�ַ�				0x4a050
		uint32_t * bt_version[4];			//�豸BT�� 16Byte			ASCII�ַ������16�ַ�				0x4a060
		uint32_t  storage[2];				//�豸�洢���� 8Byte			�ܴ洢����		U32		�豸�洢�ռ䣬��λ��KBytes  				0x4a070
																	  //��ʹ������		U32  	��ʹ�õĴ洢�ռ䣬��λ��KBytes			
		uint32_t  battery;					//��ص��� 1Byte				U8				��ʾ�����İٷֱȣ���Χ��0%-100%					0x4a078
		uint32_t  mac[2];						//MAC��ַ 6Byte				U8-Array		�豸��MAC��ַ���� AABBCCDDEEFF					0x4a07c
		
		uint32_t  * status[4];					//�豸״̬ 16Byte			�豸������		S16			�������							0x4a084
																	  /*������Ϣ�ַ���	ASCII�ַ���	��ʾ������Ϣ�����14���ַ�
																		������		������Ϣ					����
																		0 			��OK��					�豸һ������
																		-1			��UNKNOW ERROR!!!��		δ����ԭ�����
																		-2			��Unknown CMD��			���֧��
																		-3			��Invalid PARAM��		������Ч
		
																		���ڸ���ģ����Զ��������������¹�����϶���         
																		1���̶�ֵ1bit��+ ҵ��ID��8bit�� + �����롾7bit��*/
	}device_info;
	struct
	{
		//ҵ����Ϣ
		struct 
		{
			uint32_t  command_id;				//ҵ��״̬ 1Byte				HEX����	��ʶ����ҵ��ΨһID										0x4a094
			uint32_t * user_name[4];			//�������� 16Byte			HEX����	��ʶ����ҵ��Ĳ�������									0x4a098
			
			uint32_t  user_info[2];				/*������Ϣ 7Byte				�����Ա�	U8	0��Ů�� 1����										0x4a0a8
																			�������	U8	0 - 255 ��λ����
																			��������	U32	��λg
																			����Ѫ��	U8	0 ����	a��;
																						1 ����	b��
																						2 ����	ab��
																						3 ����	o��*/
			uint32_t * user_number[4];			//���˲����� 16Byte			HEX����	�洢���˵Ĳ�������Ϣ										0x4a0b0
		}bus_info;
		//ҵ��ָ��
		struct
		{
			uint32_t  running_status;			//����ģʽ 1Byte																				0x4a0c0
																			/*ҵ��״̬��	U8	0  ����	IDLE
																							1  ����	RUN
																							2  ����	STOP*/
																							
			uint32_t running_mode;				/*							U8	0  ����	ManualRun��Ĭ��ֵ��									0x4a0c4
																				1  ����	AutoRun*/
			uint32_t sample_mode;				/*���ݲɼ�ģʽ 1Byte         U8	0  ����	ManualTrigger��Ĭ��ֵ��								0x4a0c8	
																			1  ����	AutoTrigger*/
			uint32_t * device_start_time[3];								//�豸������ȡ��RTCʱ��											0x4a0cc
			
			uint32_t * time_calibration[3];		/*�豸У׼ʱ�� 9Byte			ʱ��BLE�����ʽ����9Byte�� /R ��ȡ��ǰʱ�� /WУ׼ʱ��				0x4a0d8					
																			ƫ��	����	����	����
																			0		2		Year	2016
																			2		1		month	11
																			3		1		Day		12
																			4		1		Hour	18
																			5		1		Minute	20
																			6		1		Seconds	30
																			7		2		Microsecond	500*/
			
			
			uint32_t  sample_rate;				/*�豸������ 2Byte			15		1	I	��ת��־����I=1ʱ�������� = 1/Rate Hz,���������ΪRate Hz     0x4a0e4
																			0-14	15	SR	�����ʣ�15BIT */  //total command number 19
			uint32_t  function_switch;			//Ѫ���ݻ�������          	0 -- �رգ�Ĭ�ϣ� 1 -- ��										0x4a0e8
		}bus_comm;
	}bus_mode;
};

extern struct device_id device_id_set;
extern uint32_t sample_info_buffer[10][4];

extern void device_id_change(void * value,uint16_t style);

extern void flashInit(void * p_event_data, uint16_t event_size);

#endif







