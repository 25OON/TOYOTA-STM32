/*******************************************************************************
 * File Name    : app_input.h
 * Description  : แปลงค่า ADC เป็นคำสั่งของผู้เล่น
 *                 - joystick 2 แกน -> ทิศทาง (มี deadzone)
 *                 - pot            -> ระดับความเร็วเกม 1-5
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef APP_INPUT_H
#define APP_INPUT_H

#include <stdbool.h>
#include <stdint.h>
#include "App/app_snake.h"
#include "Driver/drv_adc.h"

/* Public define -------------------------------------------------------------*/
#define APP_INPUT_SPEED_LEVELS     (5U)

/* Public function prototypes ------------------------------------------------*/

/* ล้างค่าที่ค้างอยู่ (เรียกตอนเริ่มเกมใหม่) */
void AppInput_Reset(void);

/* ป้อนค่า ADC ชุดใหม่ (เรียกจาก main loop ทุกครั้งที่ DMA ได้ค่าครบ) */
void AppInput_Update(const DrvAdcSample_t * const pSample);

/* รับทิศที่ผู้เล่นสะบัด joystick ล่าสุดแล้วล้างทิ้ง · true = มีทิศใหม่ */
bool AppInput_TakeDirection(AppDir_t * const pDir);

/* ระดับความเร็วจาก pot: 1 (ช้าสุด) ถึง APP_INPUT_SPEED_LEVELS (เร็วสุด) */
uint8_t AppInput_GetSpeedLevel(void);

/* จำนวน tick 25 ms ต่อหนึ่งก้าวของงู ตามระดับความเร็วปัจจุบัน */
uint8_t AppInput_GetStepTicks(void);

/* ค่าดิบรวมของ ADC ใช้เป็น seed ของตัวสุ่มตอนเริ่มเกม (บิตล่างเป็น noise จริง) */
uint32_t AppInput_GetSeedSource(void);

/* ค่าดิบ 3 ช่องล่าสุด ใช้โชว์บนจอตอนตรวจการต่อสาย */
void AppInput_GetRaw(DrvAdcSample_t * const pSample);

#endif /* APP_INPUT_H */
