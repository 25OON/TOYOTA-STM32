/*******************************************************************************
 * File Name    : drv_seg.h
 * Description  : 7-segment บน Training Shield 1 ผ่าน BCD decoder
 *                 ส่งเลข 0-9 เป็นรหัส BCD 4 บิตออก GPIO · เกินนั้นให้ดับจอ
 * Date         : 2026-10-05
 ******************************************************************************/
#ifndef DRV_SEG_H
#define DRV_SEG_H

#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define DRV_SEG_MAX_DIGIT          (9U)     /* decoder รับได้ถึงเลข 9 */

/* Public function prototypes ------------------------------------------------*/

/* ตั้งขา BCD ทั้ง 4 เป็น output แล้วดับจอไว้ก่อน */
void DrvSeg_Init(void);

/* แสดงเลขหนึ่งหลัก · ค่าเกิน DRV_SEG_MAX_DIGIT จะทำให้จอดับ */
void DrvSeg_ShowDigit(uint8_t digit);

/* ดับจอ (ส่งรหัสที่อยู่นอกช่วง 0-9 ให้ decoder) */
void DrvSeg_Blank(void);

#endif /* DRV_SEG_H */
