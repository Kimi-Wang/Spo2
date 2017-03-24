#include "Afe4403.h"
#include "nrf_drv_spi.h"
#include "nrf_gpio.h"
#include "app_error.h"
#include "nrf_gpio.h"
#include <string.h>
#include "nrf_delay.h"
#include "uart.h"
#include "SEGGER_RTT.h"

// AFE4403控制管脚
#define AFE_DIAG_END  11 //4403自检 1~ok in
#define AFE_PDNZ      19 //拉高工作。out
#define AFE_RESETZ    22 //4403复位 out
#define AFE_RDY_PIN   23 //4403adc转换完成 in（中断采集）
#define AFE_SPISET    25 //4403片选 out
#define AFE_LED_EN    14 //电压转换脚，拉高。out

static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(0);

#define NUM_AFE_REGISTERS    38

/****************************************************************/
/* Global functions*/
/****************************************************************/

unsigned long AFE44xx_Current_Register_Settings[36] = 
{
  CONTROL0_VAL,           //Reg0:CONTROL0: CONTROL REGISTER 0                                                
  LED2STC_VAL,            //Reg1:REDSTARTCOUNT: SAMPLE RED START COUNT
  LED2ENDC_VAL,           //Reg2:REDENDCOUNT: SAMPLE RED END COUNT
  LED2LEDSTC_VAL,         //Reg3:REDLEDSTARTCOUNT: RED LED START COUNT
  LED2LEDENDC_VAL,        //Reg4:REDLEDENDCOUNT: RED LED END COUNT
  ALED2STC_VAL,           //Reg5:AMBREDSTARTCOUNT: SAMPLE AMBIENT RED START COUNT
  ALED2ENDC_VAL,          //Reg6:AMBREDENDCOUNT: SAMPLE AMBIENT RED END COUNT
  LED1STC_VAL,            //Reg7:IRSTARTCOUNT: SAMPLE IR START COUNT
  LED1ENDC_VAL,           //Reg8:IRENDCOUNT: SAMPLE IR END COUNT
  LED1LEDSTC_VAL,         //Reg9:IRLEDSTARTCOUNT: IR LED START COUNT
  LED1LEDENDC_VAL,        //Reg10:IRLEDENDCOUNT: IR LED END COUNT
  ALED1STC_VAL,           //Reg11:AMBIRSTARTCOUNT: SAMPLE AMBIENT IR START COUNT
  ALED1ENDC_VAL,          //Reg12:AMBIRENDCOUNT: SAMPLE AMBIENT IR END COUNT
  LED2CONVST_VAL,         //Reg13:REDCONVSTART: REDCONVST
  LED2CONVEND_VAL,        //Reg14:REDCONVEND: RED CONVERT END COUNT
  ALED2CONVST_VAL,        //Reg15:AMBREDCONVSTART: RED AMBIENT CONVERT START COUNT
  ALED2CONVEND_VAL,       //Reg16:AMBREDCONVEND: RED AMBIENT CONVERT END COUNT
  LED1CONVST_VAL,         //Reg17:IRCONVSTART: IR CONVERT START COUNT
  LED1CONVEND_VAL,        //Reg18:IRCONVEND: IR CONVERT END COUNT
  ALED1CONVST_VAL,        //Reg19:AMBIRCONVSTART: IR AMBIENT CONVERT START COUNT
  ALED1CONVEND_VAL,       //Reg20:AMBIRCONVEND: IR AMBIENT CONVERT END COUNT
  ADCRSTSTCT0_VAL,        //Reg21:ADCRESETSTCOUNT0: ADC RESET 0 START COUNT
  ADCRSTENDCT0_VAL,       //Reg22:ADCRESETENDCOUNT0: ADC RESET 0 END COUNT
  ADCRSTSTCT1_VAL,        //Reg23:ADCRESETSTCOUNT1: ADC RESET 1 START COUNT
  ADCRSTENDCT1_VAL,       //Reg24:ADCRESETENDCOUNT1: ADC RESET 1 END COUNT
  ADCRSTSTCT2_VAL,        //Reg25:ADCRESETENDCOUNT2: ADC RESET 2 START COUNT
  ADCRSTENDCT2_VAL,       //Reg26:ADCRESETENDCOUNT2: ADC RESET 2 END COUNT
  ADCRSTSTCT3_VAL,        //Reg27:ADCRESETENDCOUNT3: ADC RESET 3 START COUNT
  ADCRSTENDCT3_VAL,       //Reg28:ADCRESETENDCOUNT3: ADC RESET 3 END COUNT
  PRP,                    //Reg29:PRPCOUNT: PULSE REPETITION PERIOD COUNT
  CONTROL1_VAL,           //Reg30:CONTROL1: CONTROL REGISTER 1 //timer enabled, averages=3, RED and IR LED pulse ON PD_ALM AND LED_ALM pins
  0x00000,                //Reg31:?: ??          
  (ENSEPGAIN + STAGE2EN_LED1 + STG2GAIN_LED1_0DB + CF_LED1_5P + RF_LED1_1M),                      //Reg32:TIAGAIN: TRANS IMPEDANCE AMPLIFIER GAIN SETTING REGISTER
  (AMBDAC_0uA + FLTRCNRSEL_500HZ + STAGE2EN_LED2 + STG2GAIN_LED2_0DB + CF_LED2_5P + RF_LED2_1M),  //Reg33:TIA_AMB_GAIN: TRANS IMPEDANCE AAMPLIFIER AND AMBIENT CANELLATION STAGE GAIN
  (LEDCNTRL_VAL),                                                                                 //Reg34:LEDCNTRL: LED CONTROL REGISTER

  (TX_REF_1 + RST_CLK_ON_PD_ALM_PIN_DISABLE + ADC_BYP_DISABLE + TXBRGMOD_H_BRIDGE + DIGOUT_TRISTATE_DISABLE + XTAL_ENABLE + EN_FAST_DIAG + PDN_TX_OFF + PDN_RX_OFF + PDN_AFE_OFF),                 //Reg35:CONTROL2: CONTROL REGISTER 2 //bit 9   
};

/***************************************************************************************************
* 功能描述:
*          afe4403GPIO初始化
* 参数说明：
* 返回值：
**************************************************************************************************/
static void afe4403GpioInit(void)
{
	nrf_gpio_cfg_input(AFE_DIAG_END, NRF_GPIO_PIN_NOPULL);//自检完成引脚 11	
	nrf_gpio_cfg_output(AFE_PDNZ);//输入标志,低有效。
	nrf_gpio_cfg_output(AFE_RESETZ);//复位引脚
	nrf_gpio_cfg_input(AFE_RDY_PIN, NRF_GPIO_PIN_NOPULL);//4403转换完成信号脚23，接52832pin脚中断
	nrf_gpio_cfg_output(AFE_SPISET);//4403片选
	nrf_gpio_cfg_output(AFE_RESETZ);//4403复位引脚，低有效
	nrf_gpio_cfg_output(AFE_LED_EN);//电压转换芯片使能脚
	
	nrf_gpio_cfg_input(SPI_MISO_PIN, NRF_GPIO_PIN_NOPULL);
	nrf_gpio_cfg_output(SPI_MOSI_PIN);
	nrf_gpio_cfg_output(SPI_SCK_PIN);
	
}

/***************************************************************************************************
* 功能描述:
*          afe4403SPI初始化
* 参数说明：
* 返回值：
**************************************************************************************************/
static void afe4403SpiInit(void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
//  APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, spiEventHandler));
	APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL));
}

/***************************************************************************************************
* 功能描述:
*          SPI读写一个字节
* 参数说明：
* 返回值：
**************************************************************************************************/
unsigned int spiReadWriteByte(unsigned char *data)
{	
	unsigned char recv_data[4] = {0};
	unsigned char error = 0;
	
	unsigned int value = 0;	
	
	int i = 4;
	int a = 0;
	
	unsigned char *tmp =  data;
	
	while( i-- )
	{
		char tmp_value =  *data;
		
		nrf_gpio_pin_clear(SPI_SS_PIN);
		
		for(a =0 ;a<8 ;a++)
		{
			nrf_gpio_pin_clear(SPI_SCK_PIN);			
			
			if(nrf_gpio_pin_read(SPI_MISO_PIN) == 0)
			{
				recv_data[3-i] = ((recv_data[3-i] << 1) + 0x00);
			}
			else
			{
				recv_data[3-i] = ((recv_data[3-i] << 1) + 0x01);
			}
			
			
			if((tmp_value & 0x80 ) == 0 )
			{
				nrf_gpio_pin_clear(SPI_MOSI_PIN);
			}
			else
			{
				nrf_gpio_pin_set(SPI_MOSI_PIN);
			}

			nrf_gpio_pin_set(SPI_SCK_PIN);
			
			tmp_value <<= 1;		

		}
			nrf_gpio_pin_clear(SPI_SCK_PIN);
			nrf_gpio_pin_set(SPI_SS_PIN);
		
			if(nrf_gpio_pin_read(SPI_MISO_PIN) == 0)
			{
				recv_data[3-i] = ((recv_data[3-i] << 1) + 0x00);
			}
			else
			{
				recv_data[3-i] = ((recv_data[3-i] << 1) + 0x01);
			}
		
		data++;
	}
	
//	error = nrf_drv_spi_transfer(&spi, (const unsigned char *)data, 4, (unsigned char *)recv_data, 4);
//	if(error != 0)
//	{
//		SEGGER_RTT_printf(0,"SPI operation error %d \r\n", error);
//	}//	

//	
	SEGGER_RTT_printf(0,"<----------------------------------> \r\n");
	
	SEGGER_RTT_printf(0,"send_data[0] %02x \r\n", tmp[0]);
	SEGGER_RTT_printf(0,"send_data[1] %02x \r\n", tmp[1]);	
	SEGGER_RTT_printf(0,"send_data[2] %02x \r\n", tmp[2]);
	SEGGER_RTT_printf(0,"send_data[3] %02x \r\n", tmp[3]);
	
	SEGGER_RTT_printf(0,"\r\n");
	
	SEGGER_RTT_printf(0,"recv_data[0] %02x \r\n", recv_data[0]);
	SEGGER_RTT_printf(0,"recv_data[1] %02x \r\n", recv_data[1]);	
	SEGGER_RTT_printf(0,"recv_data[2] %02x \r\n", recv_data[2]);
	SEGGER_RTT_printf(0,"recv_data[3] %02x \r\n", recv_data[3]);
	
	SEGGER_RTT_printf(0,"<----------------------------------> \r\n");
	
	value = recv_data[0];
	value = (value << 8) + recv_data[1];
	value = (value << 8) + recv_data[2];
	value = (value << 8) + recv_data[3];
	
    return value;
}

/***************************************************************************************************
* 功能描述:
*          Spins for specified clock cycles(延时函数ms)
* 参数说明：
* 返回值：
**************************************************************************************************/
void delayCycles(unsigned int delay)
{
//    for (int i = 0; i < delay; i++) 
//    {
//        
//    }
	nrf_delay_ms(delay);
}

/***************************************************************************************************
* 功能描述:
*          CS拉低
* 参数说明：
* 返回值：
**************************************************************************************************/
static void chipSelectLow(void)
{
	//nrf_gpio_pin_clear(AFE_SPISET);
}

/***************************************************************************************************
* 功能描述:
*          CS拉高
* 参数说明：
* 返回值：
**************************************************************************************************/
static void chipSelectHigh(void)
{
	//nrf_gpio_pin_set(AFE_SPISET);
}

/***************************************************************************************************
* 功能描述:
*          写afe4403寄存器
* 参数说明：
* 返回值：
**************************************************************************************************/
static void writeRegister(unsigned char address, unsigned long data)
{
	unsigned char writeData[4] = {0};
	
	writeData[0] = address;
    writeData[1] = (unsigned char)(data >> 16);
    writeData[2] = (unsigned char)(data >> 8);
    writeData[3] = (unsigned char)data;
	
    chipSelectLow();
	
	spiReadWriteByte(writeData);

    chipSelectHigh();
}

/***************************************************************************************************
* 功能描述:
*          读afe4403寄存器
* 参数说明：
* 返回值：
**************************************************************************************************/
static unsigned int readRegister(unsigned char address)
{
	unsigned char nullVale[4] = {0};

    unsigned int spiReceive=0;

    chipSelectLow();

    spiReceive = spiReadWriteByte(nullVale);

    chipSelectHigh();    // Combine the three received bytes into a signed long


    return (spiReceive);
}

/***************************************************************************************************
* 功能描述:
*          Sets register write mode of AFE4403
* 参数说明：
* 返回值：
**************************************************************************************************/
static void enableWrite(void)
{
    writeRegister(CONTROL0, 0x000000);
}

/***************************************************************************************************
* 功能描述:
*          Sets register read mode of AFE4403
* 参数说明：
* 返回值：
**************************************************************************************************/
static void enableRead(void)
{
    writeRegister(CONTROL0, 0x000001);
}

/***************************************************************************************************
* 功能描述:
*          读取AFE4403的Diagnostics标记
* 参数说明：
* 返回值：
**************************************************************************************************/
unsigned long readDiagnostics() //标记处理
{
	enableRead();
    return readRegister(DIAGNOSTICS);
}

/***************************************************************************************************
* 功能描述:
*          Resets the AFE4403 device
* 参数说明：
* 返回值：
**************************************************************************************************/
static void afe4403Reset(void)
{
	nrf_gpio_pin_set(AFE_LED_EN);//拉高电压转换芯片引脚，板子工作。
	nrf_gpio_pin_set(AFE_PDNZ);//
	
//	nrf_delay_ms(10);
	
	nrf_gpio_pin_clear(AFE_RESETZ);//使能复位
	nrf_delay_ms(1000);
	
	nrf_gpio_pin_set(AFE_RESETZ);
	
	if(nrf_gpio_pin_read(AFE_DIAG_END) == 0)
	{
		unsigned int Diag_value = 0;
		Diag_value = readDiagnostics();
		SEGGER_RTT_printf(0, "Diag_value: 0x%x\r\n", (Diag_value));

	}
}

/***************************************************************************************************
* 功能描述:
*          Read the red value in the register
* 参数说明：
* 返回值：
**************************************************************************************************/
long collectRED(void)
{
    return readRegister(LED2VAL);
}

/***************************************************************************************************
* 功能描述:
*          Read the IR value in the register 
* 参数说明：
* 返回值：
**************************************************************************************************/
long collectIR(void)
{
    return readRegister(LED1VAL);
}

/***************************************************************************************************
* 功能描述:
*          Read the ambient red value in the register
* 参数说明：
* 返回值：
**************************************************************************************************/
long collectAMRED(void)
{
    return readRegister(ALED2VAL);
}

/***************************************************************************************************
* 功能描述:
*          Read the ambient IR value in the register
* 参数说明：
* 返回值：
**************************************************************************************************/
long collectAMIR(void)
{
    return readRegister(ALED1VAL);
}

/***************************************************************************************************
* 功能描述:
*          Read the red ambient red value in the register
* 参数说明：
* 返回值：
**************************************************************************************************/
long collectREDAMRED(void)
{
    return readRegister(LED2_ALED2VAL);
}

/***************************************************************************************************
* 功能描述:
*          Read the IR ambient IR value in the register
* 参数说明：
* 返回值：
**************************************************************************************************/
long collectIRAMIR(void)
{
    return readRegister(LED1_ALED1VAL);
}

/***************************************************************************************************
* 功能描述:
*          Issue AFE diagnostics command
* 参数说明：
* 返回值：
**************************************************************************************************/
unsigned char issueDiagnostics()
{
    return 0;
}

/***************************************************************************************************
* 功能描述:
*          AFE4403初始化
* 参数说明：
* 返回值：
**************************************************************************************************/
static void afe4403DefaultRegInit()
{
  enableWrite();
  
  writeRegister((unsigned char)PRPCOUNT, (unsigned long)PRP);
  writeRegister((unsigned char)LED2STC, (unsigned long)LED2STC_VAL);
  writeRegister((unsigned char)LED2ENDC, (unsigned long)LED2ENDC_VAL);
  writeRegister((unsigned char)LED2LEDSTC, (unsigned long)LED2LEDSTC_VAL);
  writeRegister((unsigned char)LED2LEDENDC, (unsigned long)LED2LEDENDC_VAL);
  writeRegister((unsigned char)ALED2STC, (unsigned long)ALED2STC_VAL);
  writeRegister((unsigned char)ALED2ENDC, (unsigned long)ALED2ENDC_VAL);
  writeRegister((unsigned char)LED1STC, (unsigned long)LED1STC_VAL);
  writeRegister((unsigned char)LED1ENDC, (unsigned long)LED1ENDC_VAL);
  writeRegister((unsigned char)LED1LEDSTC, (unsigned long)LED1LEDSTC_VAL);
  writeRegister((unsigned char)LED1LEDENDC, (unsigned long)LED1LEDENDC_VAL);
  writeRegister((unsigned char)ALED1STC, (unsigned long)ALED1STC_VAL);
  writeRegister((unsigned char)ALED1ENDC, (unsigned long)ALED1ENDC_VAL);
  writeRegister((unsigned char)LED2CONVST, (unsigned long)LED2CONVST_VAL);
  writeRegister((unsigned char)LED2CONVEND, (unsigned long)LED2CONVEND_VAL);
  writeRegister((unsigned char)ALED2CONVST, (unsigned long)ALED2CONVST_VAL);
  writeRegister((unsigned char)ALED2CONVEND, (unsigned long)ALED2CONVEND_VAL);
  writeRegister((unsigned char)LED1CONVST, (unsigned long)LED1CONVST_VAL);
  writeRegister((unsigned char)LED1CONVEND, (unsigned long)LED1CONVEND_VAL);
  writeRegister((unsigned char)ALED1CONVST, (unsigned long)ALED1CONVST_VAL);
  writeRegister((unsigned char)ALED1CONVEND, (unsigned long)ALED1CONVEND_VAL);
  writeRegister((unsigned char)ADCRSTSTCT0, (unsigned long)ADCRSTSTCT0_VAL);
  writeRegister((unsigned char)ADCRSTENDCT0, (unsigned long)ADCRSTENDCT0_VAL);
  writeRegister((unsigned char)ADCRSTSTCT1, (unsigned long)ADCRSTSTCT1_VAL);
  writeRegister((unsigned char)ADCRSTENDCT1, (unsigned long)ADCRSTENDCT1_VAL);
  writeRegister((unsigned char)ADCRSTSTCT2, (unsigned long)ADCRSTSTCT2_VAL);
  writeRegister((unsigned char)ADCRSTENDCT2, (unsigned long)ADCRSTENDCT2_VAL);
  writeRegister((unsigned char)ADCRSTSTCT3, (unsigned long)ADCRSTSTCT3_VAL);
  writeRegister((unsigned char)ADCRSTENDCT3, (unsigned long)ADCRSTENDCT3_VAL);
  
  writeRegister((unsigned char)CONTROL0, AFE44xx_Current_Register_Settings[0]);            //0x00
  writeRegister((unsigned char)CONTROL2, AFE44xx_Current_Register_Settings[35]);           //0x23
  writeRegister((unsigned char)TIAGAIN, AFE44xx_Current_Register_Settings[32]);            //0x20
  writeRegister((unsigned char)TIA_AMB_GAIN, AFE44xx_Current_Register_Settings[33]);       //0x21
  writeRegister((unsigned char)LEDCNTRL, AFE44xx_Current_Register_Settings[34]);           //0x22
  writeRegister((unsigned char)CONTROL1, AFE44xx_Current_Register_Settings[30]);           //0x1E
  

  writeRegister((unsigned char)CONTROL3, 0);           //0x31
  writeRegister((unsigned char)PDNCYCLESTC, 0);        //0x32
  writeRegister((unsigned char)PDNCYCLEENDC, 0);       //0x33

  enableRead();
}

/***************************************************************************************************
* 功能描述:
*          AFE4403初始化
* 参数说明：
* 返回值：
**************************************************************************************************/
void afe4403Init(void)
{
    afe4403GpioInit();	
    
//    afe4403SpiInit();
    
    afe4403Reset();
    
    //afe4403DRDYInterruptInit();
    
    enableWrite();
  
//	afe4403DefaultRegInit();
	
    writeRegister(0x01,0x00012345);
	
    enableRead();
	
	int value = 0;
	value = readRegister(0x01);

	SEGGER_RTT_printf(0,"recv_value : 0x%04x \r\n",value);
	
}
