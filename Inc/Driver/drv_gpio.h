/*******************************************************************************
 * File Name    : drv_gpio.h
 * Description  : ฟังก์ชันพื้นฐานของ GPIO ระดับรีจิสเตอร์ (ใช้ร่วมกันในชั้น Driver)
 *                 (ไฟล์ภายในชั้น Driver เท่านั้น ห้าม include จากชั้น App)
 * Date         : 2026-09-15
 ******************************************************************************/
#ifndef DRV_GPIO_H
#define DRV_GPIO_H

#include <stdbool.h>
#include <stdint.h>
#include "Driver/drv_mcu.h"

/* Private enum --------------------------------------------------------------*/
typedef enum
{
    DRV_GPIO_MODE_INPUT  = 0,
    DRV_GPIO_MODE_OUTPUT = 1,
    DRV_GPIO_MODE_ALTFN  = 2,
    DRV_GPIO_MODE_ANALOG = 3
} DrvGpioMode_t;

typedef enum
{
    DRV_GPIO_PULL_NONE = 0,
    DRV_GPIO_PULL_UP   = 1,
    DRV_GPIO_PULL_DOWN = 2
} DrvGpioPull_t;

/* Public function prototypes ------------------------------------------------*/
void DrvGpio_SetMode(GPIO_TypeDef * const pPort, uint32_t pin, DrvGpioMode_t mode);
void DrvGpio_SetPull(GPIO_TypeDef * const pPort, uint32_t pin, DrvGpioPull_t pull);
void DrvGpio_SetAltFunction(GPIO_TypeDef * const pPort, uint32_t pin, uint32_t altFunction);
void DrvGpio_Write(GPIO_TypeDef * const pPort, uint32_t pin, bool levelHigh);
bool DrvGpio_ReadHigh(const GPIO_TypeDef * const pPort, uint32_t pin);

#endif /* DRV_GPIO_H */
