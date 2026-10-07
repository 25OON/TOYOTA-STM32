/*******************************************************************************
 * File Name    : drv_wdg.h
 * Description  : IWDG (independent watchdog) คาบประมาณ 500 ms
 *                 ถ้า main loop ค้างเกินคาบ ชิปจะรีเซ็ตตัวเอง
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_WDG_H
#define DRV_WDG_H

#include <stdbool.h>

/* อ่านสาเหตุการรีเซ็ตรอบก่อน (และล้างธง) แล้วเปิด IWDG — เรียกครั้งเดียวตอนบูต */
void DrvWdg_Init(void);

/* เตะ watchdog · ต้องเรียกถี่กว่าคาบ 500 ms */
void DrvWdg_Reload(void);

/* true ถ้าการบูตรอบนี้เกิดจาก IWDG รีเซ็ต (ค่าอ่านได้หลัง DrvWdg_Init()) */
bool DrvWdg_WasWatchdogReset(void);

#endif /* DRV_WDG_H */
