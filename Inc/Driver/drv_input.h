/*******************************************************************************
 * File Name    : drv_input.h
 * Description  : รับปุ่ม 4 ปุ่มของเกมผ่าน EXTI
 *                 (D2 เริ่ม, D3 เล่นใหม่, D4 พัก, D5 สลับโหมด)
 *                 กันปุ่มเด้งด้วยเวลาจาก SysTick
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_INPUT_H
#define DRV_INPUT_H

#include <stdbool.h>

/* Public struct -------------------------------------------------------------*/
typedef struct
{
    bool start;      /* D2: เริ่มเกม */
    bool restart;    /* D3: เล่นใหม่ */
    bool pause;      /* D4: พัก / เล่นต่อ */
    bool mode;       /* D5: สลับโหมด CLASSIC <-> TWIN */
} DrvButtonEvents_t;

/* Public function prototypes ------------------------------------------------*/
void DrvInput_Init(void);
void DrvInput_TakeButtonEvents(DrvButtonEvents_t * const pEvents);

#endif /* DRV_INPUT_H */
