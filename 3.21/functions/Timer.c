#include "Timer.h"
#include "Uart.h"
#include <stdbool.h>
#include <stdint.h>
#include "nrf_drv_timer.h"
#include "bsp.h"
#include "app_error.h"
#include "Lcd.h"
#include "Afe4403.h"
#include "nrf_delay.h"
#include "SEGGER_RTT.h"
#include "nrf_drv_saadc.h"
#include "app_scheduler.h" 
#include "SEGGER_RTT.h"

#define STANDARD_COUNT   5

static unsigned char timer5ms = 0;
static unsigned char timer10ms = 0;
static unsigned char timer100ms = 0;
static unsigned char timer500ms = 0;
static unsigned char timer1s = 0;

static unsigned char flag5ms = 0;
static unsigned char flag10ms = 0;
static unsigned char flag100ms = 0;
static unsigned char flag500ms = 0;
static unsigned char flag1s = 0;
unsigned char buff[3] = {0x28,0x56,0x5a};
static int num = 0;
/***************************************************************************************************
 * 功能描述:
 *          定时器事件
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void timerEventHandler(nrf_timer_event_t event_type, void* p_context)
{
    timer5ms++;
    timer10ms++;
    timer100ms++;
    timer500ms++;
    timer1s++;
    
    if(timer5ms > 0)
    {
        timer5ms = 0;
        flag5ms = 1;
    }
    if(timer10ms > 1)
    {
        timer10ms = 0;
        flag10ms = 1;
    }
    if(timer100ms > 19)
    {
        timer100ms = 0;
        flag100ms = 1;
    }
    if(timer500ms > 99)
    {
        timer500ms = 0;
        flag500ms = 1;
    }
    if(timer1s > 199)
    {
        timer1s = 0;
        flag1s = 1;		
    }
}

/***************************************************************************************************
 * 功能描述:
 *          定时器运行
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void timerInit(void)
{
    const nrf_drv_timer_t timer_ms = NRF_DRV_TIMER_INSTANCE(1);
    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    unsigned int time_ticks;
    unsigned int err_code = NRF_SUCCESS;
    
    err_code = nrf_drv_timer_init(&timer_ms, &timer_cfg, timerEventHandler);
    APP_ERROR_CHECK(err_code);
    
    time_ticks = nrf_drv_timer_ms_to_ticks(&timer_ms, STANDARD_COUNT);

    nrf_drv_timer_extended_compare(
         &timer_ms, NRF_TIMER_CC_CHANNEL0, time_ticks, NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK, true);

    nrf_drv_timer_enable(&timer_ms);
	
}

/***************************************************************************************************
 * 功能描述:
 *          定时器运行
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void timerRun(void)
{
    if(flag5ms)
    {
        flag5ms = 0;
		
    }
    else if(flag10ms)
    {
        flag10ms = 0;		
        
    }
    else if(flag100ms)
    {
        flag100ms = 0;

    }
    else if(flag500ms)
    {
        flag500ms = 0;			
	
    }
    else if(flag1s)
    {
        flag1s = 0;	
	
//		SEGGER_RTT_printf(0,"num : %d \r\n", num++);
//		nrf_drv_saadc_sample();
    }
	
	app_sched_execute();	
//	if(nrf_gpio_pin_read(11) == 1)
//	{
//		SEGGER_RTT_printf(0, "run \r\n");
//	}

}

