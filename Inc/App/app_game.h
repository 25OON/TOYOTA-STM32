/*******************************************************************************
 * File Name    : app_game.h
 * Description  : state machine ของเกม (IDLE / PLAYING / PAUSED / OVER / WIN)
 *                 คุมจังหวะก้าว คะแนน คะแนนสูงสุด และสั่งวาดฉาก
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef APP_GAME_H
#define APP_GAME_H

#include <stdbool.h>
#include <stdint.h>
#include "Driver/drv_input.h"

/* Public enum ---------------------------------------------------------------*/
typedef enum
{
    APP_GAME_IDLE = 0,    /* รอกด D2 เริ่มเกม */
    APP_GAME_PLAYING,     /* งูเลื้อยอยู่ */
    APP_GAME_PAUSED,      /* พักเกม */
    APP_GAME_OVER,        /* ชนแล้ว รอกด D3 เล่นใหม่ */
    APP_GAME_WIN          /* งูเต็มกระดาน */
} AppGameState_t;

/* Public function prototypes ------------------------------------------------*/

/* เตรียมเกมเข้าสถานะ IDLE (ยังไม่เริ่มเดิน) */
void AppGame_Init(void);

/* ข้อความแจ้งเตือนบรรทัดล่างสุดของจอ (NULL = ไม่แสดง) */
void AppGame_SetNotice(const char * const pText);

/* นับเวลาหนึ่ง tick (25 ms) · เดินงูเมื่อครบจำนวน tick ต่อก้าว */
void AppGame_OnTick(void);

/* ป้อนเหตุการณ์ปุ่มเพื่อเปลี่ยนสถานะ */
void AppGame_OnButtons(const DrvButtonEvents_t * const pEvents);

/* ส่งเฟรมถ้ามีอะไรเปลี่ยนและ UART ว่าง · เรียกทุกรอบของ main loop */
void AppGame_Service(void);

/* สถานะปัจจุบัน (ชั้น App ใช้เลือกสีไฟ LED) */
AppGameState_t AppGame_GetState(void);

/* true ครั้งเดียวต่อการกินอาหารหนึ่งครั้ง (ใช้สั่งไฟฟ้าแวบ) */
bool AppGame_TakeFoodEaten(void);

#endif /* APP_GAME_H */
