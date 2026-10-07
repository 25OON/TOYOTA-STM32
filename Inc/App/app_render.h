/*******************************************************************************
 * File Name    : app_render.h
 * Description  : วาดฉากทั้งหน้าลง frame buffer แล้วส่งออก UART ครั้งเดียวจบ
 *                 ใช้ ESC[H เลื่อนเคอร์เซอร์กลับมุมบนซ้ายแล้วทับของเดิม
 *                 (ห้ามใช้ ESC[2J ทุกเฟรม เพราะภาพจะกะพริบ)
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef APP_RENDER_H
#define APP_RENDER_H

#include <stdbool.h>
#include <stdint.h>

/* Public struct -------------------------------------------------------------*/
typedef struct
{
    uint16_t     score;
    uint16_t     best;
    uint16_t     length;
    uint8_t      speedLevel;
    const char  *pStateText;    /* ข้อความสถานะบนหัวจอ (ห้ามเป็น NULL) */
    const char  *pModeText;     /* ชื่อโหมดมุมขวาบน (ห้ามเป็น NULL) */
    const char  *pNoticeText;   /* ข้อความแจ้งเตือนบรรทัดล่างสุด (NULL = เว้นว่าง) */
    uint16_t     joyXRaw;       /* ค่า ADC ดิบ โชว์ตอน showAdc เป็น true */
    uint16_t     joyYRaw;
    uint16_t     potRaw;
    bool         showAdc;       /* true = โชว์บรรทัดค่าดิบไว้ตรวจการต่อสาย */
} AppRenderInfo_t;

/* Public function prototypes ------------------------------------------------*/

/* เตรียมโครง frame buffer (ขอบบรรทัดและรหัสเลื่อนเคอร์เซอร์) เรียกครั้งเดียว */
void AppRender_Init(void);

/* วาดหนึ่งเฟรมแล้วส่ง · false = UART ยังส่งเฟรมก่อนไม่จบ ให้ข้ามเฟรมนี้ */
bool AppRender_SendFrame(const AppRenderInfo_t * const pInfo);

#endif /* APP_RENDER_H */
