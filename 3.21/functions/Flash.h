#ifndef _FLASH_H
#define _FLASH_H 

#include <stdint.h>
#include "fstorage.h"
struct device_id
{
	//设备信息：
	struct 
	{
		uint32_t * device_type[4]; 			//设备类型   16Byte 			ASCII字符，最大16字符	起始地址：	0x4a010 
		uint32_t * device_sequence[4];  	//设备序列号 16Byte			ASCII字符，最大16字符			 	0x4a020
		uint32_t * device_batch[4];			//设备批号   16Byte			ASCII字符，最大16字符			  	0x4a030
		uint32_t * hw_version[4];			//设备HW版本号 16Byte		ASCII字符，最大16字符				0x4a040
		uint32_t * fw_version[4];			//设备FW版本号 16Byte		ASCII字符，最大16字符				0x4a050
		uint32_t * bt_version[4];			//设备BT号 16Byte			ASCII字符，最大16字符				0x4a060
		uint32_t  storage[2];				//设备存储容量 8Byte			总存储容量		U32		设备存储空间，单位：KBytes  				0x4a070
																	  //已使用容量		U32  	已使用的存储空间，单位：KBytes			
		uint32_t  battery;					//电池电量 1Byte				U8				表示电量的百分比，范围：0%-100%					0x4a078
		uint32_t  mac[2];						//MAC地址 6Byte				U8-Array		设备的MAC地址，如 AABBCCDDEEFF					0x4a07c
		
		uint32_t  * status[4];					//设备状态 16Byte			设备错误码		S16			错误编码							0x4a084
																	  /*错误信息字符串	ASCII字符串	显示错误消息，最大14个字符
																		错误码		错误信息					含义
																		0 			“OK”					设备一切正常
																		-1			“UNKNOW ERROR!!!”		未区分原因错误
																		-2			“Unknown CMD”			命令不支持
																		-3			“Invalid PARAM”		参数无效
		
																		对于各个模块的自定义错误码采用以下规则组合而成         
																		1【固定值1bit】+ 业务ID【8bit】 + 错误码【7bit】*/
	}device_info;
	struct
	{
		//业务信息
		struct 
		{
			uint32_t  command_id;				//业务状态 1Byte				HEX数组	标识本次业务唯一ID										0x4a094
			uint32_t * user_name[4];			//病人姓名 16Byte			HEX数组	标识本次业务的病人名称									0x4a098
			
			uint32_t  user_info[2];				/*病人信息 7Byte				病人性别	U8	0：女； 1：男										0x4a0a8
																			病人身高	U8	0 - 255 单位厘米
																			病人体重	U32	单位g
																			病人血型	U8	0 ——	a型;
																						1 ——	b型
																						2 ——	ab型
																						3 ——	o型*/
			uint32_t * user_number[4];			//病人病床号 16Byte			HEX数组	存储病人的病床号信息										0x4a0b0
		}bus_info;
		//业务指令
		struct
		{
			uint32_t  running_status;			//运行模式 1Byte																				0x4a0c0
																			/*业务状态码	U8	0  ——	IDLE
																							1  ——	RUN
																							2  ——	STOP*/
																							
			uint32_t running_mode;				/*							U8	0  ——	ManualRun（默认值）									0x4a0c4
																				1  ——	AutoRun*/
			uint32_t sample_mode;				/*数据采集模式 1Byte         U8	0  ——	ManualTrigger（默认值）								0x4a0c8	
																			1  ——	AutoTrigger*/
			uint32_t * device_start_time[3];								//设备启动读取的RTC时间											0x4a0cc
			
			uint32_t * time_calibration[3];		/*设备校准时间 9Byte			时间BLE编码格式（共9Byte） /R 读取当前时间 /W校准时间				0x4a0d8					
																			偏移	长度	内容	举例
																			0		2		Year	2016
																			2		1		month	11
																			3		1		Day		12
																			4		1		Hour	18
																			5		1		Minute	20
																			6		1		Seconds	30
																			7		2		Microsecond	500*/
			
			
			uint32_t  sample_rate;				/*设备采样率 2Byte			15		1	I	翻转标志，当I=1时，采样率 = 1/Rate Hz,否则采样率为Rate Hz     0x4a0e4
																			0-14	15	SR	采样率，15BIT */  //total command number 19
			uint32_t  function_switch;			//血氧容积波开关          	0 -- 关闭（默认） 1 -- 打开										0x4a0e8
		}bus_comm;
	}bus_mode;
};

extern struct device_id device_id_set;
extern uint32_t sample_info_buffer[10][4];

extern void device_id_change(void * value,uint16_t style);

extern void flashInit(void * p_event_data, uint16_t event_size);

#endif







