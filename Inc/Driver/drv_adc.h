/*******************************************************************************
 * File Name    : drv_adc.h
 * Description  : ADC1 สแกน 3 ช่อง (joystick VRx, VRy และ pot) ทุกครั้งที่
 *                 TIM2 ส่ง TRGO · ผลลัพธ์ถูกคัดลอกลง buffer ด้วย DMA2 Stream0
 *                 (ไม่ใช้ polling ตามโจทย์)
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_ADC_H
#define DRV_ADC_H

#include <stdbool.h>
#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define DRV_ADC_MAX_RAW            (4095U)  /* ADC 12 บิต */

/* Public struct -------------------------------------------------------------*/
typedef struct
{
    uint16_t joyXRaw;    /* VRx : แกนซ้าย-ขวา */
    uint16_t joyYRaw;    /* VRy : แกนขึ้น-ลง */
    uint16_t potRaw;     /* pot : ระดับความเร็วเกม */
} DrvAdcSample_t;

/* Public function prototypes ------------------------------------------------*/
void DrvAdc_Init(void);
bool DrvAdc_GetSample(DrvAdcSample_t * const pSample);
uint32_t DrvAdc_GetOverrunCount(void);

#endif /* DRV_ADC_H */
