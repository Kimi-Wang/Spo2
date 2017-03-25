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
//    nrf_gpio_cfg_input(key_0, NRF_GPIO_PIN_NOPULL);// 配置按键BUTTON_0为输入
//    
//    // 配置BUTTON_1管脚为DETECT信号输出，该句很重要，是CPU被GPIO唤醒的必要条件
//    nrf_gpio_cfg_sense_input(key_1, NRF_GPIO_PIN_NOPULL, NRF_GPIO_PIN_SENSE_LOW); 


//    // 睡眠保留RAM的参数
//    NRF_POWER->RAMON = POWER_RAMON_ONRAM0_RAM0On   << POWER_RAMON_ONRAM0_Pos
//                     | POWER_RAMON_ONRAM1_RAM1On   << POWER_RAMON_ONRAM1_Pos
//                     | POWER_RAMON_OFFRAM0_RAM0Off << POWER_RAMON_OFFRAM0_Pos
//                     | POWER_RAMON_OFFRAM1_RAM1Off << POWER_RAMON_OFFRAM1_Pos;
//    
//    while(1)
//    {     
//        // 如果BUTTON0 被按下.，则熄灭LED灯让CPU进入睡眠
//        if(nrf_gpio_pin_read(key_0) == KeyPressFlag)
//        {            //CPU进入睡眠模式，若有GPIO的DETECT信号唤，则CPU被唤醒而后复位从main函数开始执行。
//            NRF_POWER->SYSTEMOFF = 1;
//        }
//    }
//}