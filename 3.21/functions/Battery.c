#include "Battery.h"
#include "nrf_saadc.h"
#include "nrf_drv_saadc.h"
#include "uart.h"
#include "SEGGER_RTT.h"

#define SAMPLES_IN_BUFFER 5

float voltage = 0.0;  									//adc转换后的电池电压

static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];


/***************************************************************************************************
* 功能描述:
*         电压采集回调函数
* 参数说明：
* 返回值：
**************************************************************************************************/
static void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
	uint16_t total_voltage = 0;
	  //char buf[32];
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        int i;
        for (i = 0; i < SAMPLES_IN_BUFFER; i++)
        {
			total_voltage += p_event->data.done.p_buffer[i];
						
        }
			total_voltage = total_voltage / SAMPLES_IN_BUFFER;
			voltage = total_voltage * (3.6/1024.0);	
			
			uint8_t buf[8] = {0};
			sprintf((char *)buf,"%f",voltage);

			SEGGER_RTT_printf(0,"voltage :%s \r\n",buf);
    }	
}
/***************************************************************************************************
* 功能描述:
*         	ADC采集初始化
* 参数说明：
* 返回值：
**************************************************************************************************/
void saadc_init(void)
{
    ret_code_t err_code;
		// NRF_SAADC_INPUT_VDD
//    nrf_saadc_channel_config_t channel_config =NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_VDD);
	nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN7);
	
	nrf_drv_saadc_config_t adc_config = NRF_DRV_SAADC_DEFAULT_CONFIG;
	
    err_code = nrf_drv_saadc_init(&adc_config, saadc_callback);
    APP_ERROR_CHECK(err_code);
		
    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code); 

}
/***************************************************************************************************
* 功能描述:
*          Battery初始化
* 参数说明：
* 返回值：
**************************************************************************************************/
void batteryInit(void)
{
	saadc_init();
}
