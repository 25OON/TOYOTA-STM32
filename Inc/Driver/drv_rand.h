/*******************************************************************************
 * File Name    : drv_rand.h
 * Description  : ตัวสุ่มเลขแบบ xorshift32 เขียนเอง
 *                 MISRA Rule 21.24 ห้ามใช้ rand()/srand() จาก stdlib
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_RAND_H
#define DRV_RAND_H

#include <stdint.h>

/* ตั้งค่าเริ่มต้นของลำดับ (seed = 0 จะถูกแทนด้วยค่าคงที่ เพราะ xorshift ติดที่ 0) */
void DrvRand_Seed(uint32_t seed);

/* คืนเลขสุ่ม 32 บิตถัดไป */
uint32_t DrvRand_Next(void);

/* คืนเลขสุ่มในช่วง 0 .. (limit - 1) · limit = 0 จะคืน 0 */
uint16_t DrvRand_Below(uint16_t limit);

#endif /* DRV_RAND_H */
