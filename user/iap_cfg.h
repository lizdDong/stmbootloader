/**
  ******************************************************************************
  * @file    iap_cfg.h
  * @author  lizdDong
  * @version V1.0
  * @date    2021-4-13
  * @brief   None
  * @attention
  *
  ******************************************************************************
  */

#ifndef _IAP_CFG_H_
#define _IAP_CFG_H_

#include "dev_flash.h"


#define ApplicationAddress   IAP_APP_ADDR

#define FLASH_IMAGE_SIZE     IAP_APP_SIZE

#define PRINT_MSG_EN         1

#define USE_RS485_PORT       0

#define RUN_APP_DELAY_S      1

#define UPGRADE_FROM_IMAGE   1

#define COM_PORT         USART3
#define USART_PORT_USE   3

#if (USE_RS485_PORT)
#define RCC_RS485_TXEN   RCC_APB2Periph_GPIOA
#define PORT_RS485_TXEN  GPIOA
#define PIN_RS485_TXEN   GPIO_Pin_8

#define RS485_RX_EN()   PORT_RS485_TXEN->BRR = PIN_RS485_TXEN
#define RS485_TX_EN()   PORT_RS485_TXEN->BSRR = PIN_RS485_TXEN
#endif

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

typedef void (*pFunction)(void);

#endif

