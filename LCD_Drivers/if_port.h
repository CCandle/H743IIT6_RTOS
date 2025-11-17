/********************* COPYRIGHT  **********************
 * File Name        : if_port.h
 * Author           : Levetop Electronics
 * Version          : V1.0
 * Date             : 2017-9-11
 * Description      : ѡ��ͬ�������ӿ�
 ********************************************************/
#pragma once
#include "char.h"
#include "gpio.h"

// ѡ��ͨ�ŷ�ʽ

#define STM32_FSMC_8 0  // ʹ��STM32������FSMC����LT768(STM32Ӳ��ģ��8080����(8λ))
#define STM32_FSMC_16 0 // ʹ��STM32������FSMC����LT738(STM32Ӳ��ģ��8080����(16λ))
#define STM32_SPI 1     // ʹ��STM32��SPI����LT738

// ѡ��8λ8080��������
#if STM32_FSMC_8
#define LCD_BASE0 ((u32)(0x6C000000 | 0x0000007E))
#define LCD_BASE1 ((u32)(0x6C000000 | 0x00000080))
#endif

// ѡ��16λ8080��������
#if STM32_FSMC_16
#define LCD_BASE0 ((u32)(0x6C000000 | 0x0000007E))
#define LCD_BASE1 ((u32)(0x6C000000 | 0x00000080))
#endif

// ѡ��SPI����
#if STM32_SPI

#define SPI_SDIN()                                \
  {GPIOE->MODER&=~(3<<(2*2);GPIOE->MODER|=0<<2*2; \
  } /// PF11����ģʽ
#define SPI_SDOUT()                  \
  {                                  \
    GPIOE->MODER &= ~(3 << (2 * 3)); \
    GPIOE->MODER |= 1 << 2 * 3;      \
  } // PF11���ģʽ

void SPI_CS_choosed();
void SPI_CS_chooseless();
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

void Delay_us(u16 time); // ��ʱ����us��
void Delay_ms(u16 time); // ��ʱ����ms��

void test_SPIIO(void);

void FMSC_16_DataWrite_Pixel(u16 data);
