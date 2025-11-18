/********************* COPYRIGHT  **********************
 * File Name        : if_port.c
 * Author           : Levetop Electronics
 * Version          : V1.0
 * Date             : 2017-9-11
 * Description      : ��ͬ�������ӿ�
 ********************************************************/

/*
  �ļ����ܣ����ļ�ΪӲ�������ӿڣ��������ļ���LCD��Ļ�ĵײ������ļ�
*/

#include "if_port.h"
#include "gpio.h"
#include "main.h"
#include "spi.h"

void SPI_CS_choosed() { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET); }
void SPI_CS_chooseless() { HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET); }

void Delay_us(u16 time) {
  u16 i = 0;
  while (time--) {
    i = 12; // �Լ����� 12
    while (i--)
      ;
  }
}

void Delay_ms(u16 time) {
  u16 i = 0;
  while (time--) {
    i = 12000; // �Լ�����
    while (i--)
      ;
  }
}

// -------------------------------------------------------- 16λ��8080�������� -------------------------------------------------------------

#if STM32_SPI

/*
        cubemx�Զ�����
*/
void SPI2_Init(void) {
}

u8 SPI2_ReadWriteByte(u8 txdata) {
  uint8_t RxData;
  HAL_SPI_TransmitReceive(&hspi2, &txdata, &RxData, 1, HAL_MAX_DELAY);
  return RxData;
}

void SPI_CmdWrite(u8 cmd) {

  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x00);
  SPI2_ReadWriteByte(cmd);
  SPI_CS_chooseless();
}

void SPI_DataWrite(u8 data) {

  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x80);
  SPI2_ReadWriteByte(data);
  SPI_CS_chooseless();
}

void SPI_DataWrite_Pixel(u16 data) {

  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x80);
  SPI2_ReadWriteByte(data);
  SPI_CS_chooseless();

  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x80);
  SPI2_ReadWriteByte(data >> 8);
  SPI_CS_chooseless();
}

u8 SPI_StatusRead(void) {
  u8 temp = 0;
  SPI_CS_choosed();
  SPI2_ReadWriteByte(0x40);
  temp = SPI2_ReadWriteByte(0xff);
  SPI_CS_chooseless();
  return temp;
}

u16 SPI_DataRead(void) {
  u16 temp = 0;
  SPI_CS_choosed();
  SPI2_ReadWriteByte(0xc0);
  temp = SPI2_ReadWriteByte(0xff);
  SPI_CS_chooseless();
  return temp;
}
#endif

//============================================================
//-----------------------------------------------------------------------------------------------------------------------------------

void LCD_CmdWrite(u8 cmd) {

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

void LCD_DataWrite(u8 data) {

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

void LCD_DataWrite_Pixel(u16 data) {

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

u8 LCD_StatusRead(void) {
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

u16 LCD_DataRead(void) {
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

void Parallel_Init(void) {

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

  u8 temp1, temp2;
  temp1 = LCD_StatusRead();
  printf("Status:0x%X\r\n", temp1); // 0x50

  LCD_CmdWrite(0x01);
  temp2 = LCD_DataRead();
  printf("R01h:0x%X\r\n", temp2); // 0xC8
}
