/**
  ******************************************************************************
  * @file    dev_Flash.h
  * @author  lizdDong
  * @version V1.0
  * @date    2021-4-19
  * @brief   None
  * @attention
  *
  ******************************************************************************
  */

#ifndef _DEV_FLASH_H_
#define _DEV_FLASH_H_

#include <stdint.h>
#include "dev_flash_cfg.h"


#if defined (STM32F103x8)
#define FLASH_SIZE                        ((uint32_t)0x10000)  /* 64 KBytes */
#define PAGE_SIZE                         ((uint32_t)0x400)    /* 1 Kbytes */
#define SRAM_SIZE                         ((uint32_t)0x5000)   /* 20 KBytes */
#elif defined (STM32F103xB)
#define FLASH_SIZE                        ((uint32_t)0x20000)  /* 128 KBytes */
#define PAGE_SIZE                         ((uint32_t)0x400)    /* 1 Kbytes */
#define SRAM_SIZE                         ((uint32_t)0x5000)   /* 20 KBytes */
#elif defined (STM32F103xC)
#define FLASH_SIZE                        ((uint32_t)0x40000)  /* 256 KBytes */
#define PAGE_SIZE                         ((uint32_t)0x800)    /* 2 Kbytes */
#define SRAM_SIZE                         ((uint32_t)0xC000)   /* 48 KBytes */
#elif defined (STM32F103xD)
#define FLASH_SIZE                        ((uint32_t)0x60000)  /* 384 KBytes */
#define PAGE_SIZE                         ((uint32_t)0x800)    /* 2 Kbytes */
#define SRAM_SIZE                         ((uint32_t)0x10000)  /* 64 KBytes */
#elif defined (STM32F103xE)
#define FLASH_SIZE                        ((uint32_t)0x80000)  /* 512 KBytes */
#define PAGE_SIZE                         ((uint32_t)0x800)    /* 2 Kbytes */
#define SRAM_SIZE                         ((uint32_t)0x10000)  /* 64 KBytes */
#else
#error "Please define the target serie such as (STM32F103xC, STM32F103xD, STM32F103xE)."
#endif

#define LAST0_PAGE    ((uint32_t)(FLASH_BASE + FLASH_SIZE - PAGE_SIZE * 1))
#define LAST1_PAGE    ((uint32_t)(FLASH_BASE + FLASH_SIZE - PAGE_SIZE * 2))
#define LAST2_PAGE    ((uint32_t)(FLASH_BASE + FLASH_SIZE - PAGE_SIZE * 3))
#define LAST3_PAGE    ((uint32_t)(FLASH_BASE + FLASH_SIZE - PAGE_SIZE * 4))


uint32_t dev_flashWrite(uint32_t addr, const uint8_t *pBuff, uint32_t size);
uint32_t dev_flashRead(uint32_t addr, uint8_t *pBuff, uint32_t size);


#endif

















