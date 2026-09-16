/**
* @Author kokirika
* @Name OLED
* @Date 2026-09-16
*/
#ifndef PROJECT_OLED_H
#define PROJECT_OLED_H


#include "main.h"
#include "i2c.h"


/*使用硬件IIC
不可修改GPIO
PB6 -- SCL
PB7 -- SDA
*/

extern const unsigned char F6x8[][6];
extern const unsigned char F8X16[];


#define OLED_ADDRESS	0x78 //通过调整0R电阻,屏可以0x78和0x7A两个地址 -- 默认0x78

void WriteCmd(unsigned char I2C_Command);
void WriteDat(unsigned char I2C_Data);
uint8_t	 OLED_Init(void);
void OLED_SetPos(unsigned char x, unsigned char y);
void OLED_Fill(unsigned char fill_Data);
void OLED_CLS(void);
void OLED_CLS_DMA(void);
void OLED_ON(void);
void OLED_OFF(void);
void OLED_ShowStr(unsigned char x, unsigned char y, char * ch, unsigned char TextSize);
void OLED_ShowCHR(unsigned char x, unsigned char y, unsigned char ch, unsigned char TextSize);
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char *ch);
//void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]);
void OLED_SHOW_CN_str(unsigned char x, unsigned char y, unsigned char ch[]);


#endif //PROJECT_OLED_H
