#include "Lcd.h"
#include "nrf_gpio.h"
#include "stdint.h"
#include "Bmp.h"
#include "Font.h"
#include "nrf_delay.h"

#define OLED_CS      2
#define OLED_RES     3
#define OLED_DC      4
#define OLED_SCLK    16
#define OLED_SDIN    18

#define OLED_CS_CLR()  nrf_gpio_pin_clear(OLED_CS) //OLED_CS 12
#define OLED_CS_SET()  nrf_gpio_pin_set(OLED_CS)

#define OLED_RST_CLR() nrf_gpio_pin_clear(OLED_RES) //OLED_RES 28
#define OLED_RST_SET() nrf_gpio_pin_set(OLED_RES)

#define OLED_DC_CLR() nrf_gpio_pin_clear(OLED_DC) //29
#define OLED_DC_SET() nrf_gpio_pin_set(OLED_DC)

#define OLED_SCLK_CLR() nrf_gpio_pin_clear(OLED_SCLK) //CLK 30 P0
#define OLED_SCLK_SET() nrf_gpio_pin_set(OLED_SCLK)

#define OLED_SDIN_CLR() nrf_gpio_pin_clear(OLED_SDIN) //DIN 31 P1
#define OLED_SDIN_SET() nrf_gpio_pin_set(OLED_SDIN)
	     
#define OLED_CMD    0	//д����
#define OLED_DATA   1	//д����

static unsigned char size = 16;//�����С��Ĭ��16		   
static unsigned char oledGram[128][8] = {0};	 

/***************************************************************************************************
 * ��������:
 *            дһ���ֽ�  
 * ����˵����			 
 *		   	 dat��8λ����
 *		   	 cmd������ �� ����
 *		   
 * ����ֵ��
 **************************************************************************************************/
static void writeByte(unsigned char dat, unsigned char cmd)
{	
	unsigned char i = 0;
    
	if (cmd)
	{
        OLED_DC_SET();
	}
	else 
	{
        OLED_DC_CLR();
	}	
	
	OLED_CS_CLR();
    
	for(i = 0; i < 8; i++)
	{			  
		OLED_SCLK_CLR();
		if(dat&0x80)
		{
            OLED_SDIN_SET();
		}
		else
		{			
            OLED_SDIN_CLR();
		}
		OLED_SCLK_SET();
		dat <<= 1;   
	}				 		  
	OLED_CS_SET();
	OLED_DC_SET();   	  
} 

/***************************************************************************************************
 * ��������:
 *          ����oled�ڴ�
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/		 
static void refreshGram(void)
{
	unsigned char i = 0, n = 0;		    
	for(i = 0; i < 8; i++)  
	{  
		writeByte(0xb0 + i, OLED_CMD);    //����ҳ��ַ��0~7��
		writeByte(0x00, OLED_CMD);      //������ʾλ�á��е͵�ַ
		writeByte(0x12, OLED_CMD);      //������ʾλ�á��иߵ�ַ   
		for(n = 0; n < 128; n++)
		{
			writeByte(oledGram[n][i], OLED_DATA); 
		}
	}   
}

/***************************************************************************************************
 * ��������:
 *          ���� 
 * ����˵����			 
 *		    x:0~63
 *		    y:0~47
 *		    t:1 ��� 0,���
 * ����ֵ��
 **************************************************************************************************/
static void drawPoint(unsigned char x, unsigned char y, unsigned char t) //add by wpc
{
	unsigned char pos = 0, bx = 0,temp=0;
    
	if((x > 63) || (y > 47))
	{
		return;//������Χ��.
	}
	pos = 7 - (y / 8);
	bx = y % 8;
	temp = 1 << (7 - bx);
	if(t)
	{
		oledGram[x][pos] |= temp;
	}
	else 
	{
		oledGram[x][pos] &= ~temp;
	}
	refreshGram();//������ʾ	
}

///***************************************************************************************************
// * ��������:
// *          ����
// * ����˵����			 
// *		    (x1,y1)��ʼ�����
// *		    (x2,y2)���������
// *		    
// * ����ֵ��
// **************************************************************************************************/
//static void drawLine(unsigned short int x1, unsigned short int y1, unsigned short int x2, unsigned short int y2) 
//{
//	unsigned short int t = 0; 
//	int xerr = 0;
//    int yerr = 0;
//	int delta_x = 0;
//	int delta_y = 0;
//	int distance = 0; 
//	int incx = 0;
//	int incy = 0;
//	int uRow = 0;
//	int uCol = 0; 

//	delta_x = x2 - x1; //������������ 
//	delta_y = y2 - y1; 
//	uRow = x1; 
//	uCol = y1; 
//	if(delta_x > 0)
//	{
//		incx=1; //���õ������� 
//	}
//	else if(!delta_x)
//	{
//		incx = 0;//��ֱ�� 
//	}
//	else 
//	{
//		incx = -1;
//        delta_x = -delta_x;
//	} 
//	if(delta_y > 0)
//	{
//		incy=1; 
//	}
//	else if(!delta_y)
//	{
//		incy = 0;//ˮƽ�� 
//	}
//	else
//	{
//		incy = -1;
//        delta_y = -delta_y;
//	} 
//    
//	if( delta_x > delta_y)
//	{
//		distance = delta_x; //ѡȡ�������������� 
//	}
//	else
//	{
//		distance = delta_y; 
//	}
//	for(t = 0; t <= (distance + 1); t++ )//������� 
//	{  
//		drawPoint(uRow, uCol, 1);//���� 
//		xerr += delta_x ; 
//		yerr += delta_y ; 
//		if(xerr > distance) 
//		{ 
//			xerr -= distance; 
//			uRow += incx; 
//		} 
//        
//		if(yerr > distance) 
//		{ 
//			yerr -= distance; 
//			uCol += incy; 
//		} 
//	}  
//} 

/***************************************************************************************************
 * ��������:
 *            ���ָ������  
 * ����˵����			 
 *		   x1,y1,x2,y2 �������ĶԽ�����
 *		   ȷ��x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63
 *		   dot:0,���;1,���	
 * ����ֵ��
 **************************************************************************************************/
void fillScreen(unsigned char x1, unsigned char y1, unsigned char x2, unsigned char y2, unsigned char dot)  
{  
	unsigned char x = 0, y = 0;
    
	for(x = x1; x <= x2; x++)
	{
		for(y = y1; y <= y2; y++)
		{
			drawPoint(x, y, dot);
		}
	}													    
	//refreshGram();//������ʾ
}
	
/***************************************************************************************************
 * ��������:
 *           ������ʾλ��
 * ����˵����			 
 *		   
 *		   
 *		    
 * ����ֵ��
 **************************************************************************************************/
static void setPos(unsigned char x, unsigned char y) 
{ 
	writeByte(0xb2 + y, OLED_CMD);
	writeByte((((x + 0x20)&0xf0) >> 4) | 0x10, OLED_CMD);
	writeByte((x&0x0f) | 0x01, OLED_CMD); 
}   	  

/***************************************************************************************************
 * ��������:
 *           ����OLED��ʾ 
 * ����˵����			 
 *		   
 *		   
 *		    
 * ����ֵ��
 **************************************************************************************************/
void displayOn(void)
{
	writeByte(0X8D, OLED_CMD);  //SET OLED_DCOLED_DC����
	writeByte(0X14, OLED_CMD);  //OLED_DCOLED_DC ON
	writeByte(0XAF, OLED_CMD);  //DISPLAY ON
}

/***************************************************************************************************
 * ��������:
 *           �ر�OLED��ʾ 
 * ����˵����			 
 *		   
 *		   
 *		    
 * ����ֵ��
 **************************************************************************************************/
void displayOff(void)
{
	writeByte(0X8D, OLED_CMD);  //SET OLED_DCOLED_DC����
	writeByte(0X10, OLED_CMD);  //OLED_DCOLED_DC OFF
	writeByte(0XAE, OLED_CMD);  //DISPLAY OFF
}		   			 

/***************************************************************************************************
 * ��������:
 *          ��������,������,������Ļ�Ǻ�ɫ��!��û����һ��!!!
 * ����˵����			 
 *		   
 *		   
 *		    
 * ����ֵ��
 **************************************************************************************************/
void clearScreen(void)  
{  
	unsigned char i = 0, n = 0;		    
	for(i = 0; i < 8; i++)  
	{  
		writeByte(0xb2 + i, OLED_CMD);    //����ҳ��ַ��0~7��
		writeByte(0x00, OLED_CMD);      //������ʾλ�á��е͵�ַ
		writeByte(0x10, OLED_CMD);      //������ʾλ�á��иߵ�ַ	
		
		for(n = 0; n < 128; n++)
		{
			oledGram[n][i] = 0x00;
			writeByte(0x00, OLED_DATA); 			
		}
	} //������ʾ
}

/***************************************************************************************************
 * ��������:
 *          ��ָ��λ����ʾһ���ַ�,���������ַ�
 * ����˵����		
			x:0~127
			y:0~63
			mode:0,������ʾ;1,������ʾ				 
			size:ѡ������ 16/12 	    
 * ����ֵ��
 **************************************************************************************************/
void showChar(unsigned char x,unsigned char y,unsigned char chr)
{      	
	unsigned char c = 0, i = 0;	
	c = chr-' ';//�õ�ƫ�ƺ��ֵ			
	if(x > 63) 
	{
		x = 0;
        y = y + 2;
	}
	if(size == 16)
	{
		setPos(x, y);	
		for(i = 0; i < 8; i++)
		{
			writeByte(F8X16[c*16+i], OLED_DATA);
		}
		setPos(x, y+1);
		for(i = 0; i < 8; i++)
		{
			writeByte(F8X16[c * 16 + i + 8], OLED_DATA);
		}
	}	
	else 
	{	
		setPos(x, (y + 1));
		for(i = 0; i < 6; i++)
		{
			writeByte(F6x8[c][i], OLED_DATA);
		}	
	}
}				  

/***************************************************************************************************
 * ��������:
 *          ��ʾһ���ַ��Ŵ�
 * ����˵����		
			
			x,y :�������	 
			chr ����ָ��
 * ����ֵ��
 **************************************************************************************************/
void showString(unsigned char x, unsigned char y, unsigned char *chr)
{
	unsigned char j = 0;
	while (chr[j] != '\0')
	{
		showChar(x, y, chr[j]);
		x += 8;
		if(x > 56)
		{
			x = 0;
            y += 2;
		}
		j++;
	}
}

/***************************************************************************************************
 * ��������:
 *          ��ʾ����
 * ����˵����			
			x,y :�������	 
			chr ����ָ��
 * ����ֵ��
 **************************************************************************************************/
void showChinese(unsigned char x, unsigned char y, unsigned char no)
{      			    
	unsigned char t = 0, adder = 0;
	setPos(x, y);	
    for(t = 0;t < 16; t++)
	{
		writeByte(Hzk[2*no][t], OLED_DATA);
		adder += 1;
    }	
	setPos(x, y+1);	
    for(t = 0; t < 16; t++)
	{
		writeByte(Hzk[2 * no + 1][t], OLED_DATA);
		adder += 1;
	}					
}

/***************************************************************************************************
 * ��������:
 *         ��ʾͼƬ,��ʾ��ʾBMPͼƬ128��64��ʼ������(x,y),x�ķ�Χ0��127��yΪҳ�ķ�Χ0��7
 * ����˵����			
			BMP:ͼƬ����	 
			
 * ����ֵ��
 **************************************************************************************************/
void drawBMP(unsigned char BMP[])
{ 	
	unsigned int j=0;
	unsigned char x = 0, y = 0;
	for(y = 2; y < 8; y++)
	{	
		for(x = 0; x < 64; x++)		
		{
			oledGram[x][y] = BMP[j++];
		}
		refreshGram();//������ʾ
		
		setPos(0,0);
	}
} 

/***************************************************************************************************
 * ��������:
 *          led��ʾ���棬��ʾ������ʱ�䡢������־
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/	
static void pinInit(void)
{
	nrf_gpio_cfg_output(OLED_CS);
	nrf_gpio_cfg_output(OLED_RES);
	nrf_gpio_cfg_output(OLED_DC);
	nrf_gpio_cfg_output(OLED_SCLK);
	nrf_gpio_cfg_output(OLED_SDIN);
}

/***************************************************************************************************
 * ��������:
 *          led��ʾ���棬��ʾ������ʱ�䡢������־
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
void showTimepage(unsigned char * timer, unsigned char * year)
{
	clearScreen();	

	showString(12, 1, "08:08");
	size = 8;
	showString(17, 3, "2017");
	showString(12, 4, "12/18");
	size = 16;	
}

/***************************************************************************************************
 * ��������:
 *          led��ʾ���棬��ʾSpo2��PR
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
void showSpo2page(unsigned char * SPO2,unsigned char * PR)
{
	drawBMP(bmp_spo2);  //ͼƬ��ʾ(ͼƬ��ʾ���ã����ɵ��ֱ�ϴ󣬻�ռ�ý϶�ռ䣬FLASH�ռ�8K��������)

	showString(28, 2, "100");
	
	showString(38, 4, "120");
	
}

/***************************************************************************************************
 * ��������:
 *          ������ʾ����
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
void showHellopage(unsigned char * PR,unsigned char * bpm)
{
	showString(15,2,"HELLO");

	size = 16;
	nrf_delay_ms (4000);
}
/***************************************************************************************************
 * ��������:
 *          �ػ���ʾ����
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
void showByepage(unsigned char * PR,unsigned char * bpm)
{
	showString(3,2,"Bye_Bye");
	
	size = 16;
	nrf_delay_ms (4000);
}

 
/***************************************************************************************************
 * ��������:
 *          OLEDʼ��
 * ����˵����
 * ����ֵ��
 **************************************************************************************************/
 void lcdInit(void)
 {
	pinInit();
 
	OLED_RST_SET();
	nrf_delay_ms (100);
	OLED_RST_CLR();
	nrf_delay_ms (100);
	OLED_RST_SET(); 
					  
	writeByte(0xAE, OLED_CMD);//--turn off oled panel  �ر�OLED���
	writeByte(0x00, OLED_CMD);//---set low column address ���õ��е�ַ
	writeByte(0x12, OLED_CMD);//---set high column address ���ø��е�ַ
	writeByte(0x40, OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F) ������ʼ�е�ַ����ӳ��RAM��ʾ��ʼ�У�0x00~0x3F��
	writeByte(0x81, OLED_CMD);//--set contrast control register ���öԱȶȿ��ƼĴ���
	writeByte(0xCF, OLED_CMD); // Set SEG Output Current Brightness ����SEG�����������
	writeByte(0xA1, OLED_CMD);//--Set SEG/Column Mapping     0xa0���ҷ��� 0xa1����
	writeByte(0xC8, OLED_CMD);//Set COM/Row Scan Direction   0xc0���·��� 0xc8����
	writeByte(0xA6, OLED_CMD);//--set normal display
	writeByte(0xA8, OLED_CMD);//--set multiplex ratio(1 to 64) �趨�����ʣ�1~64��
	writeByte(0x3f, OLED_CMD);//--1/64 duty 
	writeByte(0xD3, OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F) ������ʾƫ��Shift  Mapping RAM��������0x00~0x3F��
	writeByte(0x00, OLED_CMD);//-not offset 
	writeByte(0xd5, OLED_CMD);//--set display clock divide ratio/oscillator frequency ������ʾʱ�ӷ�Ƶ��/����Ƶ��
	writeByte(0x80, OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec ���÷�Ƶ�ȣ���ʱ������Ϊ100֡/��
	writeByte(0xD9, OLED_CMD);//--set pre-charge period ����Ԥ�������
	writeByte(0xF1, OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock ��Ԥ�������Ϊ15ʱ�Ӻͷŵ�Ϊ1ʱ��
	writeByte(0xDA, OLED_CMD);//--set com pins hardware configuration ����com����Ӳ������
	writeByte(0x12, OLED_CMD);
	writeByte(0xDB, OLED_CMD);//--set vcomh ����vcomh
	writeByte(0x40, OLED_CMD);//Set VCOM Deselect Level ����VCOMȡ��ѡ�񼶱�
	writeByte(0x20, OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02) ����ҳ��ַģʽ��0x00 / 0x01 / 0x02��
	writeByte(0x02, OLED_CMD);//
	writeByte(0x8D, OLED_CMD);//--set Charge Pump enable/disable ���õ�ɱ�����/����
	writeByte(0x14, OLED_CMD);//--set(0x10) disable
	writeByte(0xA4, OLED_CMD);// Disable Entire Display On (0xa4/0xa5) ����������ʾ����0xa4 / 0xa5��
	writeByte(0xA6, OLED_CMD);// Disable Inverse Display On (0xa6/a7)  ���÷�����ʾ����0xa6 / a7��
	writeByte(0xAF, OLED_CMD);//--turn on oled panel ��OLED���
	
	writeByte(0xAF, OLED_CMD); /*display ON*/ 
	clearScreen();
	setPos(0, 0); 	
	
	clearScreen();
}	


 


