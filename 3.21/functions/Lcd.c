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
	     
#define OLED_CMD    0	//写命令
#define OLED_DATA   1	//写数据

static unsigned char size = 16;//字体大小，默认16		   
static unsigned char oledGram[128][8] = {0};	 

/***************************************************************************************************
 * 功能描述:
 *            写一个字节  
 * 参数说明：			 
 *		   	 dat：8位数据
 *		   	 cmd：命令 或 数据
 *		   
 * 返回值：
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
 * 功能描述:
 *          更新oled内存
 * 参数说明：
 * 返回值：
 **************************************************************************************************/		 
static void refreshGram(void)
{
	unsigned char i = 0, n = 0;		    
	for(i = 0; i < 8; i++)  
	{  
		writeByte(0xb0 + i, OLED_CMD);    //设置页地址（0~7）
		writeByte(0x00, OLED_CMD);      //设置显示位置—列低地址
		writeByte(0x12, OLED_CMD);      //设置显示位置—列高地址   
		for(n = 0; n < 128; n++)
		{
			writeByte(oledGram[n][i], OLED_DATA); 
		}
	}   
}

/***************************************************************************************************
 * 功能描述:
 *          画点 
 * 参数说明：			 
 *		    x:0~63
 *		    y:0~47
 *		    t:1 填充 0,清空
 * 返回值：
 **************************************************************************************************/
static void drawPoint(unsigned char x, unsigned char y, unsigned char t) //add by wpc
{
	unsigned char pos = 0, bx = 0,temp=0;
    
	if((x > 63) || (y > 47))
	{
		return;//超出范围了.
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
	refreshGram();//更新显示	
}

///***************************************************************************************************
// * 功能描述:
// *          画线
// * 参数说明：			 
// *		    (x1,y1)起始坐标点
// *		    (x2,y2)结束坐标点
// *		    
// * 返回值：
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

//	delta_x = x2 - x1; //计算坐标增量 
//	delta_y = y2 - y1; 
//	uRow = x1; 
//	uCol = y1; 
//	if(delta_x > 0)
//	{
//		incx=1; //设置单步方向 
//	}
//	else if(!delta_x)
//	{
//		incx = 0;//垂直线 
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
//		incy = 0;//水平线 
//	}
//	else
//	{
//		incy = -1;
//        delta_y = -delta_y;
//	} 
//    
//	if( delta_x > delta_y)
//	{
//		distance = delta_x; //选取基本增量坐标轴 
//	}
//	else
//	{
//		distance = delta_y; 
//	}
//	for(t = 0; t <= (distance + 1); t++ )//画线输出 
//	{  
//		drawPoint(uRow, uCol, 1);//画点 
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
 * 功能描述:
 *            填充指定区域  
 * 参数说明：			 
 *		   x1,y1,x2,y2 填充区域的对角坐标
 *		   确保x1<=x2;y1<=y2 0<=x1<=127 0<=y1<=63
 *		   dot:0,清空;1,填充	
 * 返回值：
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
	//refreshGram();//更新显示
}
	
/***************************************************************************************************
 * 功能描述:
 *           设置显示位置
 * 参数说明：			 
 *		   
 *		   
 *		    
 * 返回值：
 **************************************************************************************************/
static void setPos(unsigned char x, unsigned char y) 
{ 
	writeByte(0xb2 + y, OLED_CMD);
	writeByte((((x + 0x20)&0xf0) >> 4) | 0x10, OLED_CMD);
	writeByte((x&0x0f) | 0x01, OLED_CMD); 
}   	  

/***************************************************************************************************
 * 功能描述:
 *           开启OLED显示 
 * 参数说明：			 
 *		   
 *		   
 *		    
 * 返回值：
 **************************************************************************************************/
void displayOn(void)
{
	writeByte(0X8D, OLED_CMD);  //SET OLED_DCOLED_DC命令
	writeByte(0X14, OLED_CMD);  //OLED_DCOLED_DC ON
	writeByte(0XAF, OLED_CMD);  //DISPLAY ON
}

/***************************************************************************************************
 * 功能描述:
 *           关闭OLED显示 
 * 参数说明：			 
 *		   
 *		   
 *		    
 * 返回值：
 **************************************************************************************************/
void displayOff(void)
{
	writeByte(0X8D, OLED_CMD);  //SET OLED_DCOLED_DC命令
	writeByte(0X10, OLED_CMD);  //OLED_DCOLED_DC OFF
	writeByte(0XAE, OLED_CMD);  //DISPLAY OFF
}		   			 

/***************************************************************************************************
 * 功能描述:
 *          清屏函数,清完屏,整个屏幕是黑色的!和没点亮一样!!!
 * 参数说明：			 
 *		   
 *		   
 *		    
 * 返回值：
 **************************************************************************************************/
void clearScreen(void)  
{  
	unsigned char i = 0, n = 0;		    
	for(i = 0; i < 8; i++)  
	{  
		writeByte(0xb2 + i, OLED_CMD);    //设置页地址（0~7）
		writeByte(0x00, OLED_CMD);      //设置显示位置—列低地址
		writeByte(0x10, OLED_CMD);      //设置显示位置—列高地址	
		
		for(n = 0; n < 128; n++)
		{
			oledGram[n][i] = 0x00;
			writeByte(0x00, OLED_DATA); 			
		}
	} //更新显示
}

/***************************************************************************************************
 * 功能描述:
 *          在指定位置显示一个字符,包括部分字符
 * 参数说明：		
			x:0~127
			y:0~63
			mode:0,反白显示;1,正常显示				 
			size:选择字体 16/12 	    
 * 返回值：
 **************************************************************************************************/
void showChar(unsigned char x,unsigned char y,unsigned char chr)
{      	
	unsigned char c = 0, i = 0;	
	c = chr-' ';//得到偏移后的值			
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
 * 功能描述:
 *          显示一个字符号串
 * 参数说明：		
			
			x,y :起点坐标	 
			chr 数据指针
 * 返回值：
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
 * 功能描述:
 *          显示汉字
 * 参数说明：			
			x,y :起点坐标	 
			chr 数据指针
 * 返回值：
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
 * 功能描述:
 *         显示图片,显示显示BMP图片128×64起始点坐标(x,y),x的范围0～127，y为页的范围0～7
 * 参数说明：			
			BMP:图片数组	 
			
 * 返回值：
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
		refreshGram();//更新显示
		
		setPos(0,0);
	}
} 

/***************************************************************************************************
 * 功能描述:
 *          led显示画面，显示电量、时间、蓝牙标志
 * 参数说明：
 * 返回值：
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
 * 功能描述:
 *          led显示画面，显示电量、时间、蓝牙标志
 * 参数说明：
 * 返回值：
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
 * 功能描述:
 *          led显示画面，显示Spo2、PR
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void showSpo2page(unsigned char * SPO2,unsigned char * PR)
{
	drawBMP(bmp_spo2);  //图片显示(图片显示慎用，生成的字表较大，会占用较多空间，FLASH空间8K以下慎用)

	showString(28, 2, "100");
	
	showString(38, 4, "120");
	
}

/***************************************************************************************************
 * 功能描述:
 *          开机显示画面
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void showHellopage(unsigned char * PR,unsigned char * bpm)
{
	showString(15,2,"HELLO");

	size = 16;
	nrf_delay_ms (4000);
}
/***************************************************************************************************
 * 功能描述:
 *          关机显示画面
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
void showByepage(unsigned char * PR,unsigned char * bpm)
{
	showString(3,2,"Bye_Bye");
	
	size = 16;
	nrf_delay_ms (4000);
}

 
/***************************************************************************************************
 * 功能描述:
 *          OLED始化
 * 参数说明：
 * 返回值：
 **************************************************************************************************/
 void lcdInit(void)
 {
	pinInit();
 
	OLED_RST_SET();
	nrf_delay_ms (100);
	OLED_RST_CLR();
	nrf_delay_ms (100);
	OLED_RST_SET(); 
					  
	writeByte(0xAE, OLED_CMD);//--turn off oled panel  关闭OLED面板
	writeByte(0x00, OLED_CMD);//---set low column address 设置低列地址
	writeByte(0x12, OLED_CMD);//---set high column address 设置高列地址
	writeByte(0x40, OLED_CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F) 设置起始行地址设置映射RAM显示起始行（0x00~0x3F）
	writeByte(0x81, OLED_CMD);//--set contrast control register 设置对比度控制寄存器
	writeByte(0xCF, OLED_CMD); // Set SEG Output Current Brightness 设置SEG输出电流亮度
	writeByte(0xA1, OLED_CMD);//--Set SEG/Column Mapping     0xa0左右反置 0xa1正常
	writeByte(0xC8, OLED_CMD);//Set COM/Row Scan Direction   0xc0上下反置 0xc8正常
	writeByte(0xA6, OLED_CMD);//--set normal display
	writeByte(0xA8, OLED_CMD);//--set multiplex ratio(1 to 64) 设定复用率（1~64）
	writeByte(0x3f, OLED_CMD);//--1/64 duty 
	writeByte(0xD3, OLED_CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F) 设置显示偏移Shift  Mapping RAM计数器（0x00~0x3F）
	writeByte(0x00, OLED_CMD);//-not offset 
	writeByte(0xd5, OLED_CMD);//--set display clock divide ratio/oscillator frequency 设置显示时钟分频比/振荡器频率
	writeByte(0x80, OLED_CMD);//--set divide ratio, Set Clock as 100 Frames/Sec 设置分频比，将时钟设置为100帧/秒
	writeByte(0xD9, OLED_CMD);//--set pre-charge period 设置预充电周期
	writeByte(0xF1, OLED_CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock 将预充电设置为15时钟和放电为1时钟
	writeByte(0xDA, OLED_CMD);//--set com pins hardware configuration 设置com引脚硬件配置
	writeByte(0x12, OLED_CMD);
	writeByte(0xDB, OLED_CMD);//--set vcomh 设置vcomh
	writeByte(0x40, OLED_CMD);//Set VCOM Deselect Level 设置VCOM取消选择级别
	writeByte(0x20, OLED_CMD);//-Set Page Addressing Mode (0x00/0x01/0x02) 设置页地址模式（0x00 / 0x01 / 0x02）
	writeByte(0x02, OLED_CMD);//
	writeByte(0x8D, OLED_CMD);//--set Charge Pump enable/disable 设置电荷泵启用/禁用
	writeByte(0x14, OLED_CMD);//--set(0x10) disable
	writeByte(0xA4, OLED_CMD);// Disable Entire Display On (0xa4/0xa5) 禁用整个显示开（0xa4 / 0xa5）
	writeByte(0xA6, OLED_CMD);// Disable Inverse Display On (0xa6/a7)  禁用反向显示开（0xa6 / a7）
	writeByte(0xAF, OLED_CMD);//--turn on oled panel 打开OLED面板
	
	writeByte(0xAF, OLED_CMD); /*display ON*/ 
	clearScreen();
	setPos(0, 0); 	
	
	clearScreen();
}	


 


