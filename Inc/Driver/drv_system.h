/*******************************************************************************
 * File Name    : drv_system.h
 * Description  : ตัวนับเวลาระดับมิลลิวินาทีจาก SysTick (ใช้กันปุ่มเด้ง)
 * Date         : 2026-09-15
 ******************************************************************************/
#ifndef DRV_SYSTEM_H
#define DRV_SYSTEM_H

#include <stdint.h>

/* ตั้ง SysTick ให้ขัดจังหวะทุก 1 ms */
void DrvSystem_Init(void);

/* คืนเวลาเป็นมิลลิวินาทีนับจากเรียก DrvSystem_Init() */
uint32_t DrvSystem_GetMs(void);

#endif /* DRV_SYSTEM_H */
