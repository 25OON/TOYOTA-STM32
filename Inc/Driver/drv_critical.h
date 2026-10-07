/*******************************************************************************
 * File Name    : drv_critical.h
 * Description  : ช่วงป้องกัน (critical section) สำหรับอ่าน/เขียนตัวแปร
 *                 ที่ใช้ร่วมกันระหว่าง interrupt กับ main loop
 * Date         : 2026-09-15
 ******************************************************************************/
#ifndef DRV_CRITICAL_H
#define DRV_CRITICAL_H

#include <stdint.h>

/* ปิด interrupt แล้วคืนค่า PRIMASK เดิมไว้ใช้ตอนออก */
uint32_t DrvCritical_Enter(void);

/* คืนสถานะ interrupt ตามค่า PRIMASK ที่ได้จาก DrvCritical_Enter() */
void DrvCritical_Exit(uint32_t primask);

#endif /* DRV_CRITICAL_H */
