/********************* COPYRIGHT  **********************
* File Name        : if_port.h
* Author           : Levetop Electronics
* Version          : V1.0
* Date             : 2017-9-11
* Description      : 选择不同的驱动接口
********************************************************/


#include "char.h"

//选择通信方式

#define STM32_FSMC_8	0		// 使用STM32的外设FSMC驱动LT768(STM32硬件模拟8080总线(8位))
#define STM32_FSMC_16	0		// 使用STM32的外设FSMC驱动LT738(STM32硬件模拟8080总线(16位))
#define STM32_SPI		1		// 使用STM32的SPI驱动LT738



//选择8位8080总线驱动
#if STM32_FSMC_8
#define LCD_BASE0        ((u32)(0x6C000000 | 0x0000007E))
#define LCD_BASE1        ((u32)(0x6C000000 | 0x00000080))
#endif

//选择16位8080总线驱动
#if STM32_FSMC_16
#define LCD_BASE0        ((u32)(0x6C000000 | 0x0000007E))
#define LCD_BASE1        ((u32)(0x6C000000 | 0x00000080))
#endif

//选择SPI驱动
#if STM32_SPI

		#define SPI_SDIN()  {GPIOE->MODER&=~(3<<(2*2);GPIOE->MODER|=0<<2*2;}///PF11输入模式
		#define SPI_SDOUT() {GPIOE->MODER&=~(3<<(2*3));GPIOE->MODER|=1<<2*3;} 	//PF11输出模式

												
		//-----------------端口定义----------------  					   
		#define SPI_CS_choosed     HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET)
		#define SPI_CS_chooseless    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET) 
		//#define LCD_RST PGout(6)			//注意：原工程中并未使用此宏定义复位
#endif


void Delay_us(u16 time);
void Delay_ms(u16 time);


void Parallel_Init(void);
void FSMC_IO_Init(void);

void FMSC_16_CmdWrite(u8 cmd);
void FMSC_16_DataWrite(u8 data);
void FMSC_16_DataWrite_Pixel(u16 data);
u8 FMSC_16_StatusRead(void);
u16 FMSC_16_DataRead(void);

void SPI2_Init(void);
u8 SPI2_ReadWriteByte(u8 TxData);
void SPI_CmdWrite(u8 cmd);
void SPI_DataWrite(u8 data);
void SPI_DataWrite_Pixel(u16 data);
u8 SPI_StatusRead(void);
u16 SPI_DataRead(void);


void LCD_CmdWrite(u8 cmd);
void LCD_DataWrite(u8 data);
void LCD_DataWrite_Pixel(u16 data);
u8 LCD_StatusRead(void);
u16 LCD_DataRead(void);
	 
void Delay_us(u16 time); //延时函数us级
void Delay_ms(u16 time); //延时函数ms级

void test_SPIIO(void);

void FMSC_16_DataWrite_Pixel(u16 data);




