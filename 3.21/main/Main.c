#include "Afe4403.h"
#include "Battery.h"
#include "Bluetooth.h"
#include "Flash.h"
#include "Key.h"
#include "Lcd.h"
#include "RTC.h"
#include "Timer.h"
#include "Uart.h"
#include "G_sensor.h"
#include "SEGGER_RTT.h"
#include "app_scheduler.h" 
#include "nrf_gpio.h"
#include "nrf_delay.h"

/***************************************************************************************************
 * 功能描述:
 *          系统初始化
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void initSystem(void)
{    
	uartInit();
	timerInit();
	bleInit();
	
	afe4403Init();
	batteryInit();

	lcdInit();
//    app_sched_event_put(&device_id_set,sizeof(device_id_set),flashInit);
	G_sensorInit();
//  keyInit();

//  rtcInit();

}

/***************************************************************************************************
 * 功能描述:
 *          主函数
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
int main(void)
{
    initSystem();
	SEGGER_RTT_printf(0,"program start! \r\n");
	
	showSpo2page(0x00, 0x00);
	
	nrf_gpio_cfg_output(7);
	
    while(1)
    {
		timerRun();
        
    }
}


