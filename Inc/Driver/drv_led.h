/*******************************************************************************
 * File Name    : drv_led.h
 * Description  : ควบคุม LED 4 ดวงบน Training Shield 1 ผ่าน GPIO
 * Date         : 2026-09-15
 ******************************************************************************/
#ifndef DRV_LED_H
#define DRV_LED_H

#include <stdbool.h>

/* Public enum ---------------------------------------------------------------*/
typedef enum
{
    DRV_LED_GREEN = 0,   /* D10 PB6 */
    DRV_LED_YELLOW,      /* D11 PA7 */
    DRV_LED_RED,         /* D12 PA6 */
    DRV_LED_BLUE,        /* D13 PA5 */
    DRV_LED_COUNT
} DrvLedId_t;

/* Public function prototypes ------------------------------------------------*/
void DrvLed_Init(void);
void DrvLed_Set(DrvLedId_t led, bool turnOn);

#endif /* DRV_LED_H */
