/********************* COPYRIGHT  **********************
* File Name        : if_port.c
* Author           : Levetop Electronics
* Version          : V1.0
* Date             : 2017-9-11
* Description      : 不同的驱动接口
********************************************************/

/*
  文件介绍：此文件为硬件抽象层接口，余三个文件是LCD屏幕的底层驱动文件
*/

#include "if_port.h"
#include "main.h"
#include "spi.h"

void Delay_us(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=12;        //自己定义 12
      while(i--);    
   }
}

void Delay_ms(u16 time)
{    
   u16 i=0;  
   while(time--)
   {
      i=12000;    //自己定义
      while(i--);    
   }
}



// -------------------------------------------------------- 16位的8080总线驱动 -------------------------------------------------------------


	  
#if STM32_SPI

/*
	cubemx自动生成
*/
void SPI2_Init(void)
{	

}


u8 SPI2_ReadWriteByte(u8 txdata)
{
	uint8_t RxData;
	HAL_SPI_TransmitReceive(&hspi2, &txdata, &RxData, 1, HAL_MAX_DELAY);
	return RxData;	
}



void SPI_CmdWrite(u8 cmd)
{  

	SPI_CS_choosed;		
	SPI2_ReadWriteByte(0x00);
	SPI2_ReadWriteByte(cmd);
	SPI_CS_chooseless;		
}

void SPI_DataWrite(u8 data)
{ 

	SPI_CS_choosed;	
	SPI2_ReadWriteByte(0x80);
	SPI2_ReadWriteByte(data);
	SPI_CS_chooseless;
}

void SPI_DataWrite_Pixel(u16 data)
{ 

	SPI_CS_choosed;	
	SPI2_ReadWriteByte(0x80);
	SPI2_ReadWriteByte(data);	
	SPI_CS_chooseless;	
	
	SPI_CS_choosed;	
	SPI2_ReadWriteByte(0x80);
	SPI2_ReadWriteByte(data>>8);	
	SPI_CS_chooseless;
}

u8 SPI_StatusRead(void)
{
	u8 temp = 0;	
	SPI_CS_choosed;	
	SPI2_ReadWriteByte(0x40);
	temp = SPI2_ReadWriteByte(0xff);
	SPI_CS_chooseless;
	return temp;
}

u16 SPI_DataRead(void)
{
	u16 temp = 0;	
	SPI_CS_choosed;	
	SPI2_ReadWriteByte(0xc0);
	temp = SPI2_ReadWriteByte(0xff);
	SPI_CS_chooseless;
	return temp;
}
#endif




//============================================================
//-----------------------------------------------------------------------------------------------------------------------------------

void LCD_CmdWrite(u8 cmd)
{

	#if STM32_FSMC_8
	FMSC_8_CmdWrite(cmd);
	#endif
	
	#if STM32_FSMC_16
	FMSC_16_CmdWrite(cmd);
	#endif
	
	#if STM32_SPI
	SPI_CmdWrite(cmd);
	#endif

}

void LCD_DataWrite(u8 data)
{

	#if STM32_FSMC_8
	FMSC_8_DataWrite(data);
	#endif
	
	#if STM32_FSMC_16
	FMSC_16_DataWrite(data);
	#endif
	
	#if STM32_SPI
	SPI_DataWrite(data);
	#endif

}

void LCD_DataWrite_Pixel(u16 data)
{

	#if STM32_FSMC_8
	FMSC_8_DataWrite_Pixel(data);
	#endif
	
	#if STM32_FSMC_16
	FMSC_16_DataWrite_Pixel(data);
	#endif
	
	#if STM32_SPI
	SPI_DataWrite_Pixel(data);
	#endif

}


u8 LCD_StatusRead(void)
{
	u8 temp = 0;
	
	#if STM32_FSMC_8
	temp = FMSC_8_StatusRead();
	#endif
	
	#if STM32_FSMC_16
	temp = FMSC_16_StatusRead();
	#endif
	
	#if STM32_SPI
	temp = SPI_StatusRead();
	#endif
	
	return temp;
}

u16 LCD_DataRead(void)
{
	u16 temp = 0;
	
	#if STM32_FSMC_8
	temp = FMSC_8_DataRead();
	#endif
	
	#if STM32_FSMC_16
	temp = FMSC_16_DataRead();
	#endif
	
	#if STM32_SPI
	temp = SPI_DataRead();
	#endif
	
	return temp;
}
	  
	 


void Parallel_Init(void)
{

	#if STM32_FSMC_8
		FSMC_Init_8();
	#endif
	
	#if STM32_FSMC_16
		FSMC_Init_16();
	#endif
	
	#if STM32_SPI
		SPI2_Init();
	#endif

}



void test_SPIIO(void)
	
{  
  	
  u8 temp1,temp2;
  temp1=LCD_StatusRead();
  printf("Status:0x%X\r\n",temp1);//0x50


  LCD_CmdWrite(0x01);
  temp2 =LCD_DataRead();
  printf("R01h:0x%X\r\n",temp2);//0xC8
	
	
}












