/**
  ******************************************************************************
  * @file    dev_flash_cfg.c
  * @author  lizdDong
  * @version V1.0
  * @date    2021-4-19
  * @brief   None
  * @attention
  *
  ******************************************************************************
  */


#ifndef _DEV_FLASH_CFG_H_
#define _DEV_FLASH_CFG_H_

#include <stdint.h>

/* define device type */
//#define STM32F103xC


/* define the save address of the parameter */
#if (LOG_USE_LEVEL_SAVE_FLASH)
#include "dcom_log_cfg.h"
#define LOG_LEVEL_SAVE_FLASH_ADDR        (FLASH_BASE + FLASH_SIZE - PAGE_SIZE / 2)
#endif

#define CALIB_PARAM_ADDR                  LAST0_PAGE
#define CALIB_PARAM_SAVED_FLAG_ADDR       (LAST0_PAGE + sizeof(g_asCalibParam))
#define CALIB_PARAM_SAVED_FLAG            0xAA55


#define IAP_BOOT_SIZE            (1024 * 16)

#define IAP_APP_ADDR             (FLASH_BASE + IAP_BOOT_SIZE)
#define IAP_APP_SIZE             (1024 * 96)

#define IAP_IMAGE_ADDR           (IAP_APP_ADDR + IAP_APP_SIZE)
#define IAP_IMAGE_SIZE           IAP_APP_SIZE

#define IAP_FLAG_ADDR            (IAP_IMAGE_ADDR + IAP_IMAGE_SIZE)
#define IAP_FLAG                 0xA55A

#endif

















