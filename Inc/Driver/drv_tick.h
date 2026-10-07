/*******************************************************************************
 * File Name    : drv_tick.h
 * Description  : TIM2 สร้างจังหวะ 25 ms (update interrupt) และสัญญาณ TRGO
 *                 สั่ง ADC1 เริ่มแปลงค่าด้วยฮาร์ดแวร์
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_TICK_H
#define DRV_TICK_H

#include <stdint.h>

/* ตั้งค่า TIM2 (ยังไม่เริ่มนับ) */
void DrvTick_Init(void);

/* เริ่มนับ ต้องเรียกหลัง DrvAdc_Init() เพื่อให้ ADC พร้อมรับ TRGO ครั้งแรก */
void DrvTick_Start(void);

/* คืนจำนวน tick ที่เกิดขึ้นตั้งแต่เรียกครั้งก่อน แล้วล้างตัวนับ */
uint32_t DrvTick_TakePending(void);

#endif /* DRV_TICK_H */
