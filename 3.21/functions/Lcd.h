#ifndef _LCD_H
#define _LCD_H 

//OLED¿ØÖÆÓÃº¯Êý
void lcdInit(void);
void fillScreen(unsigned char x1,unsigned char y1,unsigned char x2,unsigned char y2,unsigned char dot);
void displayOn(void);
void displayOff(void);
void clearScreen(void);
void showString(unsigned char x,unsigned char y,unsigned char *chr);
void showChar(unsigned char x,unsigned char y,unsigned char chr);
void showChinese(unsigned char x,unsigned char y,unsigned char no);
void drawBMP(unsigned char BMP[]);
void showTimepage(unsigned char * timer,unsigned char * year);
void showSpo2page(unsigned char * SPO2,unsigned char * PR);
void showHellopage(unsigned char * PR,unsigned char * bpm);
void showByepage(unsigned char * PR,unsigned char * bpm);
void lcdInit(void);
#endif

