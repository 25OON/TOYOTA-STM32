/*******************************************************************************
 * File Name    : drv_mcu.h
 * Description  : จุดเดียวที่เลือกรุ่น MCU และ include CMSIS ของ STM32F411RE
 *                 (ไฟล์ภายในชั้น Driver เท่านั้น ห้าม include จากชั้น App)
 * Date         : 2026-09-15
 ******************************************************************************/
#ifndef DRV_MCU_H
#define DRV_MCU_H

/* Private includes ----------------------------------------------------------*/
#ifndef STM32F411xE
#define STM32F411xE
#endif
#include "stm32f4xx.h"

/* Private define ------------------------------------------------------------*/
/* โปรเจกต์ไม่ได้ตั้ง PLL จึงวิ่งด้วย HSI 16 MHz (APB1/APB2 ไม่หาร) */
#define DRV_SYSTEM_CLOCK_HZ        (16000000UL)

#endif /* DRV_MCU_H */
