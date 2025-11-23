/* =======================================================================
 * mem_ext_full.ld — Enhanced memory sections for STM32H7 FreeRTOS C++ project
 * Author: Carlos Q / 2025
 * Compatible with: CubeMX-generated STM32H743VITx_CM7.ld
 * -----------------------------------------------------------------------
 * Features:
 *  - ITCM:  High-speed instruction memory (.itcm.code)
 *  - DTCM:  High-speed CPU-only data (.dtcm.data / .dtcm.bss)
 *  - AXI:   DMA-accessible cacheable buffers (.axi.data / .axi.bss)
 *  - D2:    Medium-speed DMA SRAM (.d2.data / .d2.bss)
 *  - D3:    Low-speed shared SRAM (.d3.data / .d3.bss)
 *  - Automatic section routing based on initialization (.bss.xxx / .data.xxx)
 * ----------------------------------------------------------------------- */

/* ======================================================================= */
/* SECTIONS DEFINITIONS */
/* ======================================================================= */
SECTIONS
{
  wip_sec (NOLOAD) : 
  { 
    . = ABSOLUTE(0x30040000); 
    *(.RxDecripSection) 
    
    . = ABSOLUTE(0x30040080); 
    *(.TxDecripSection) 
    
    . = ABSOLUTE(0x30040100); 
    *(.Rx_PoolSection)   
  } >SRAM3 
  /* ======================= ITCM ======================================== */
  /* Highest-speed instruction memory, CPU-only (not for data)            */
  .itcm.code : ALIGN(16)
  {
    __start_itcm_code = .;
    KEEP(*(.itcm))           /* 用户指定的 ITCM 代码 */
    KEEP(*(.itcm.code))
    KEEP(*(.itcm.code*))
    KEEP(*(.itcm.text))
    KEEP(*(.itcm.text*))
    KEEP(*(.itcm_text))
    KEEP(*(.itcm_text*))
    KEEP(*(.text.fast))
    KEEP(*(.text.fast*))
    . = ALIGN(16);
    __end_itcm_code = .;
  } >ITCMRAM AT>FLASH

  __itcm_code_load_start = LOADADDR(.itcm.code);
  __itcm_code_load_end   = LOADADDR(.itcm.code) + SIZEOF(.itcm.code);
  __itcm_code_start = __start_itcm_code;
  __itcm_code_end   = __end_itcm_code;

  /* ======================= DTCM ======================================== */
  /* High-speed CPU-only data, not DMA accessible                         */
  .dtcm.data (NOLOAD) : ALIGN(8)
  {
    __start_dtcm_data = .;
    KEEP(*(.data.dtcm))      /* 已初始化的 DTCM 数据 */
    KEEP(*(.data.dtcm*))
    KEEP(*(.dtcm.data))
    KEEP(*(.dtcm.data*))
    . = ALIGN(8);
    __end_dtcm_data = .;
  } >DTCMRAM

  .dtcm.bss (NOLOAD) : ALIGN(8)
  {
    __start_dtcm_bss = .;
    *(.bss.dtcm)             /* 未初始化的 DTCM 变量 */
    *(.bss.dtcm*)
    *(.dtcm.bss)
    *(.dtcm.bss*)
    . = ALIGN(8);
    __end_dtcm_bss = .;
  } >DTCMRAM

  /* ======================= AXI SRAM ==================================== */
  /* DMA-capable, cacheable, main system RAM                              */
  .axi.data : ALIGN(32)
  {
    __start_axi_data = .;
    KEEP(*(.data.axi))
    KEEP(*(.data.axi*))
    KEEP(*(.axi.data))
    KEEP(*(.axi.data*))
    . = ALIGN(32);
    __end_axi_data = .;
  } >RAM

  .axi.bss (NOLOAD) : ALIGN(32)
  {
    __start_axi_bss = .;
    *(.bss.axi)
    *(.bss.axi*)
    *(.axi.bss)
    *(.axi.bss*)
    . = ALIGN(32);
    __end_axi_bss = .;
  } >RAM

  /* ======================= D2 SRAM ===================================== */
  /* Secondary DMA-capable RAM (medium speed)                             */
  .d2.data : ALIGN(8)
  {
    __start_d2_data = .;
    KEEP(*(.data.d2))
    KEEP(*(.data.d2*))
    KEEP(*(.d2.data))
    KEEP(*(.d2.data*))
    . = ALIGN(8);
    __end_d2_data = .;
  } >SRAM1

  .d2.bss (NOLOAD) : ALIGN(8)
  {
    __start_d2_bss = .;
    *(.bss.d2)
    *(.bss.d2*)
    *(.d2.bss)
    *(.d2.bss*)
    . = ALIGN(8);
    __end_d2_bss = .;
  } >SRAM1

  /* ======================= D3 SRAM ===================================== */
  /* Shared or retained memory (low-speed, for system states)             */
  .d3.data : ALIGN(8)
  {
    __start_d3_data = .;
    KEEP(*(.data.d3))
    KEEP(*(.data.d3*))
    KEEP(*(.d3.data))
    KEEP(*(.d3.data*))
    . = ALIGN(8);
    __end_d3_data = .;
  } >RAM_D3

  .d3.bss (NOLOAD) : ALIGN(8)
  {
    __start_d3_bss = .;
    *(.bss.d3)
    *(.bss.d3*)
    *(.d3.bss)
    *(.d3.bss*)
    . = ALIGN(8);
    __end_d3_bss = .;
  } >RAM_D3

  /* SRAM2 initialized data: RUN in SRAM2, LOAD from FLASH */
  .sram2.data : ALIGN(4)
  {
      __sram2_data_start = .;
      KEEP(*(.sram2.data))
      KEEP(*(.sram2.data*))
      __sram2_data_end = .;
  } >SRAM2 AT> FLASH

  /* Flash load address */
  __sram2_data_source = LOADADDR(.sram2.data);

  /* Sizes */
  PROVIDE(__sram2_data_start = ADDR(.sram2.data));
  PROVIDE(__sram2_data_end = ADDR(.sram2.data) + SIZEOF(.sram2.data));
  PROVIDE(__sram2_data_source = LOADADDR(.sram2.data));

  .sram2.bss (NOLOAD) : ALIGN(4)
  {
      __sram2_bss_start = .;
      *(.sram2.bss)
      *(.sram2.bss*)
      *(.sram2)
      *(.sram2*)
      __sram2_bss_end = .;
  } >SRAM2

  .axi (NOLOAD) : ALIGN(32)
  {
    __axi_start = .;
    *(.axi)
    *(.axi*)
    . = ALIGN(32);
    __axi_end = .;
  } >RAM

  .d3 (NOLOAD) : ALIGN(8)
  {
    __d3_start = .;
    *(.d3)
    *(.d3*)
    . = ALIGN(8);
    __d3_end = .;
  } >RAM_D3

}
