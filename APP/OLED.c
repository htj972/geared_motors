/**
* @Author kokirika
* @Name OLED
* @Date 2026-09-16
*/

/*
*
* Function List:
* 1. void I2C_Configuration(void) -- 配置CPU的硬件I2C
* 2. void I2C_WriteByte(uint8_t addr,uint8_t data) -- 向寄存器地址写一个byte的数据
* 3. void WriteCmd(unsigned char I2C_Command) -- 写命令
* 4. void WriteDat(unsigned char I2C_Data) -- 写数据
* 5. void OLED_Init(void) -- OLED屏初始化
* 6. void OLED_SetPos(unsigned char x, unsigned char y) -- 设置起始点坐标
* 7. void OLED_Fill(unsigned char fill_Data) -- 全屏填充
* 8. void OLED_CLS(void) -- 清屏
* 9. void OLED_ON(void) -- 唤醒
* 10. void OLED_OFF(void) -- 睡眠
* 11. void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize) -- 显示字符串(字体大小有6*8和8*16两种)
* 12. void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N) -- 显示中文(中文需要先取模，然后放到codetab.h中)
* 13. void OLED_DrawBMP(unsigned char x0,unsigned char y0,unsigned char x1,unsigned char y1,unsigned char BMP[]) -- BMP图片
*
* History: none;
*
*************************************************************************************/

#include "OLED.h"
#include "codetab.h"
#include "main.h"

void WriteCmd(unsigned char I2C_Command)//cmd
{
    HAL_I2C_Mem_Write(&hi2c1 ,OLED_ADDRESS,0x00,
                      I2C_MEMADD_SIZE_8BIT,&I2C_Command,1,0x100);
}

void WriteDat(unsigned char I2C_Data)//data
{
    HAL_I2C_Mem_Write(&hi2c1 ,OLED_ADDRESS,0x40,
                      I2C_MEMADD_SIZE_8BIT,&I2C_Data,1,0x100);
}


void OLED_Init(void)
{
//    int wait_time=255;
//    IIC_Start();
//    IIC_Send_Byte(OLED_ADDRESS);
//    while(IIC_Wait_Ack()&&wait_time--);
//    IIC_Stop();
//    if(wait_time<1)return 0;
//    WriteCmd(0xAE); //关闭显示
//    WriteCmd(0x20);	//Set Memory Addressing Mode
//    WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
//    WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
//    WriteCmd(0xc8);	//Set COM Output Scan Direction
//    WriteCmd(0x00); //---set low column address
//    WriteCmd(0x10); //---set high column address
//    WriteCmd(0x40); //--set start line address
//    WriteCmd(0x81); //--set contrast control register
//    WriteCmd(0xff); //亮度调节 0x00~0xff
//    WriteCmd(0xa1); //--set segment re-map 0 to 127
//    WriteCmd(0xa6); //--set normal display
//    WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
//    WriteCmd(0x3F); //
//    WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
//    WriteCmd(0xd3); //-set display offset
//    WriteCmd(0x00); //-not offset
//    WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
//    WriteCmd(0xf0); //--set divide ratio
//    WriteCmd(0xd9); //--set pre-charge period
//    WriteCmd(0x22); //
//    WriteCmd(0xda); //--set com pins hardware configuration
//    WriteCmd(0x12);
//    WriteCmd(0xdb); //--set vcomh
//    WriteCmd(0x20); //0x20,0.77xVcc
//    WriteCmd(0x8d); //--set DC-DC enable
//    WriteCmd(0x14); //
//    WriteCmd(0xaf); //--turn on oled panel
		
		WriteCmd(0xAE);

    WriteCmd(0x20);
    WriteCmd(0x02);       // Page Addressing Mode

    WriteCmd(0xB0);
    WriteCmd(0xC8);

    WriteCmd(0x00);
    WriteCmd(0x10);

    WriteCmd(0x40);

    WriteCmd(0x81);
    WriteCmd(0xFF);

    WriteCmd(0xA1);
    WriteCmd(0xA6);

    WriteCmd(0xA8);
    WriteCmd(0x3F);

    WriteCmd(0xA4);

    WriteCmd(0xD3);
    WriteCmd(0x00);

    WriteCmd(0xD5);
    WriteCmd(0xF0);

    WriteCmd(0xD9);
    WriteCmd(0x22);

    WriteCmd(0xDA);
    WriteCmd(0x12);

    WriteCmd(0xDB);
    WriteCmd(0x20);

    WriteCmd(0x8D);
    WriteCmd(0x14);

    WriteCmd(0xAF);

//    return wait_time;
}

void OLED_SetPos(unsigned char x, unsigned char y) //设置起始点坐标
{
    WriteCmd(0xb0+y);
    WriteCmd(((x&0xf0)>>4)|0x10);
    WriteCmd((x&0x0f)|0x01);
}

void OLED_Fill(unsigned char fill_Data)//全屏填充
{
    unsigned char m,n;
    for(m=0;m<8;m++)
    {
        WriteCmd(0xb0+m);		//page0-page1
        WriteCmd(0x00);		//low column start address
        WriteCmd(0x10);		//high column start address
        for(n=0;n<128;n++)
        {
            WriteDat(fill_Data);
        }
    }
}

void OLED_CLS_DMA(void)
{
	static uint8_t OLED_ClearBuffer[8*128] = {0};
    /* 切换为水平寻址模式 */
    WriteCmd(0x20);
    WriteCmd(0x00);

    /* 设置列范围：0 ~ 127 */
    WriteCmd(0x21);
    WriteCmd(0x00);
    WriteCmd(0x7F);

    /* 设置页范围：0 ~ 7 */
    WriteCmd(0x22);
    WriteCmd(0x00);
    WriteCmd(0x07);

    /* DMA一次发送1024字节0x00 */
    HAL_I2C_Mem_Write_DMA(&hi2c1,
                          OLED_ADDRESS,
                          0x40,
                          I2C_MEMADD_SIZE_8BIT,
                          OLED_ClearBuffer,
                          sizeof(OLED_ClearBuffer));

    /* 等DMA完成 */
    while (HAL_I2C_GetState(&hi2c1) != HAL_I2C_STATE_READY)
    {
    }
		
		HAL_Delay(500);

    /* 恢复Page Addressing Mode，
       因为你现有的OLED_SetPos()是按Page模式写的 */
    WriteCmd(0x20);
    WriteCmd(0x02);
		
}

void OLED_CLS(void)//清屏
{
    OLED_Fill(0x00);
}



//--------------------------------------------------------------
// Prototype      : void OLED_ON(void)
// Calls          :
// Parameters     : none
// Description    : 将OLED从休眠中唤醒
//--------------------------------------------------------------
void OLED_ON(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X14);  //开启电荷泵
    WriteCmd(0XAF);  //OLED唤醒
}

//--------------------------------------------------------------
// Prototype      : void OLED_OFF(void)
// Calls          :
// Parameters     : none
// Description    : 让OLED休眠 -- 休眠模式下,OLED功耗不到10uA
//--------------------------------------------------------------
void OLED_OFF(void)
{
    WriteCmd(0X8D);  //设置电荷泵
    WriteCmd(0X10);  //关闭电荷泵
    WriteCmd(0XAE);  //OLED休眠
}

//--------------------------------------------------------------
// Prototype      : void OLED_ShowStr(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowStr(unsigned char x, unsigned char y, char * ch, unsigned char TextSize)
{
    unsigned char c = 0,i = 0,j = 0;
    switch(TextSize)
    {
        case 1:
        {
            while(ch[j] != '\0')
            {
                c = ch[j] - 32;
                if(x > 126)
                {
                    x = 0;
                    y++;
                }
                OLED_SetPos(x,y);
                for(i=0;i<6;i++)
                    WriteDat(F6x8[c][i]);
                x += 6;
                j++;
            }
        }break;
        case 2:
        {
            while(ch[j] != '\0')
            {
                c = ch[j] - 32;
                if(x > 120)
                {
                    x = 0;
                    y++;
                }
                OLED_SetPos(x,y);
                for(i=0;i<8;i++)
                    WriteDat(F8X16[c*16+i]);
                OLED_SetPos(x,y+1);
                for(i=0;i<8;i++)
                    WriteDat(F8X16[c*16+i+8]);
                x += 8;
                j++;
            }
        }break;
    }
}
//--------------------------------------------------------------
// Prototype      : void OLED_ShowCHR(unsigned char x, unsigned char y, unsigned char ch[], unsigned char TextSize)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); ch[] -- 要显示的字符串; TextSize -- 字符大小(1:6*8 ; 2:8*16)
// Description    : 显示codetab.h中的ASCII字符,有6*8和8*16可选择
//--------------------------------------------------------------
void OLED_ShowCHR(unsigned char x, unsigned char y, unsigned char ch, unsigned char TextSize)
{
    unsigned char c = 0,i = 0;
    switch(TextSize)
    {
        case 16:case 1:
        {
            c = ch - 32;
            if(x > 126)
            {
                x = 0;
                y++;
            }
            OLED_SetPos(x,y);
            for(i=0;i<6;i++)
                WriteDat(F6x8[c][i]);
            x += 6;

        }break;
        case 32:case 2:
        {
            c = ch - 32;
            if(x > 120)
            {
                x = 0;
                y++;
            }
            OLED_SetPos(x,y);
            for(i=0;i<8;i++)
                WriteDat(F8X16[c*16+i]);
            OLED_SetPos(x,y+1);
            for(i=0;i<8;i++)
                WriteDat(F8X16[c*16+i+8]);
            x += 8;
        }break;
    }
}
//--------------------------------------------------------------
// Prototype      : void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char N)
// Calls          :
// Parameters     : x,y -- 起始点坐标(x:0~127, y:0~7); N:汉字在codetab.h中的索引
// Description    : 显示codetab.h中的汉字,16*16点阵
//--------------------------------------------------------------
void OLED_ShowCN(unsigned char x, unsigned char y, unsigned char *ch)
{
    unsigned char wm=0;
    OLED_SetPos(x , y);
    for(wm = 0;wm < 16;wm++)
        WriteDat(ch[wm]);
    OLED_SetPos(x,y + 1);
    for(wm = 0;wm < 16;wm++)
        WriteDat(ch[wm+16]);
}

void OLED_SHOW_CN_str(unsigned char x, unsigned char y, unsigned char ch[])
{
    uint8_t j=0;
    while(ch[j] != '\0')
    {
        if(x>111) {y+=2;x=0;}
        if(ch[j]<127)
        {
            OLED_ShowCHR(x,y,ch[j],2);
            //if(ch[j]=='m')x+=2;
            j++;
            x+=7+1;
        }
        else
        {
//#define X_fout_size		16
//            unsigned char dis_temp[X_fout_size*X_fout_size/8];//32
//            unsigned char data_temp;
//            unsigned char dis_temp_check[X_fout_size*X_fout_size/8];//32
//            unsigned char ch_temp[2];
//            ch_temp[0]=ch[j];
//            ch_temp[1]=ch[j+1];
//            Get_HzMat(ch_temp,dis_temp,X_fout_size);
//
//            u8 t,t1;
//            u8 csize=(X_fout_size/8+((X_fout_size%8)?1:0))*(X_fout_size);
//            for(t=0;t<csize;t++)
//            {
//                for(t1=0;t1<8;t1++)
//                    if((dis_temp[t]&(0x80>>t1))==(0x80>>t1))
//                        data_temp|=(0x01<<t1);
//                dis_temp[t]=data_temp;data_temp=0;
//                if(t%2)
//                    dis_temp_check[16+t/2]=dis_temp[t];
//                else
//                    dis_temp_check[0+t/2]=dis_temp[t];
//            }
//            OLED_ShowCN(x,y,dis_temp_check);
////			else
////			{
////				printf("没有\' %s \'这个字库\r\n",ch_temp);
////			}
//            j+=2;
//            x+=16+1;
        }
    }
}


