///*****************************************************/
//#include <stdbool.h>
//#include <stdint.h>
//#include "nrf.h"
//#include "nrf_gpio.h"
//#include "boards.h"

//#define KeyPressFlag 0
//#define key_0 7
//int main(void)
//{
//    
//    nrf_gpio_cfg_input(key_0, NRF_GPIO_PIN_NOPULL);// ���ð���BUTTON_0Ϊ����
//    
//    // ����BUTTON_1�ܽ�ΪDETECT�ź�������þ����Ҫ����CPU��GPIO���ѵı�Ҫ����
//    nrf_gpio_cfg_sense_input(key_1, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW); 


//    // ˯�߱���RAM�Ĳ���
//    NRF_POWER->RAMON = POWER_RAMON_ONRAM0_RAM0On   << POWER_RAMON_ONRAM0_Pos
//                     | POWER_RAMON_ONRAM1_RAM1On   << POWER_RAMON_ONRAM1_Pos
//                     | POWER_RAMON_OFFRAM0_RAM0Off << POWER_RAMON_OFFRAM0_Pos
//                     | POWER_RAMON_OFFRAM1_RAM1Off << POWER_RAMON_OFFRAM1_Pos;
//    
//    while(1)
//    {     
//        // ���BUTTON0 ������.����Ϩ��LED����CPU����˯��
//        if(nrf_gpio_pin_read(key_0) == KeyPressFlag)
//        {            //CPU����˯��ģʽ������GPIO��DETECT�źŻ�����CPU�����Ѷ���λ��main������ʼִ�С�
//            NRF_POWER->SYSTEMOFF = 1;
//        }
//    }
//}