#include "G_sensor.h"
#include "stdint.h"
#include "nrf_drv_twi.h"
#include "app_error.h"
#include "app_util_platform.h"
#include "SEGGER_RTT.h"

#define TWI_INSTANCE_ID 1

static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);

#define ARDUINO_SDA_PIN 8
#define ARDUINO_SCL_PIN 13

/***************************************************************************************************
* ��������:
*          spi��ʼ��
* ����˵����
* ����ֵ��
**************************************************************************************************/
static void spiHandle(nrf_drv_twi_evt_t const * p_event, void * p_context)
{
	
}

/***************************************************************************************************
* ��������:
*          spi��ʼ��
* ����˵����
* ����ֵ��
**************************************************************************************************/
static void twiInit()
{
	ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_TWI_FREQ_100K,
       .interrupt_priority = APP_IRQ_PRIORITY_LOWEST,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, spiHandle, NULL);
    APP_ERROR_CHECK(err_code);
	
    nrf_drv_twi_enable(&m_twi);	
}

/***************************************************************************************************
* ��������:
*          G_sensor��ʼ��
* ����˵����
* ����ֵ��
**************************************************************************************************/
void G_sensorInit(void)
{
	twiInit();
	
//	unsigned char value = 0x85;
//	
//	value = nrf_drv_twi_tx(&m_twi, 0x08, (const uint8_t *)&value, 1, false);
//	
//	SEGGER_RTT_printf(0," value : %d\r\n", value);	
//	
//	
//	value = nrf_drv_twi_rx(&m_twi, 0x08, (uint8_t *)&value, 1);
//		
//	SEGGER_RTT_printf(0," value : %d\r\n", value);
}

