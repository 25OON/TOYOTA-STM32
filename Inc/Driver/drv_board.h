/*******************************************************************************
 * File Name    : drv_board.h
 * Description  : ค่าตั้งของบอร์ด NUCLEO-F411RE + Training Shield 1 + joystick
 *                 ขา, ระดับสัญญาณ และช่อง ADC ของเกม Snake
 *                 (ไฟล์ภายในชั้น Driver เท่านั้น ห้าม include จากชั้น App)
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_BOARD_H
#define DRV_BOARD_H

#include "Driver/drv_mcu.h"

/* ---- LED บน shield (ขา MCU -> ตัวต้านทาน -> LED -> GND, HIGH = ติด) ---------*/
#define DRV_LED_GREEN_PORT         (GPIOB)
#define DRV_LED_GREEN_PIN          (6U)     /* D10 */
#define DRV_LED_YELLOW_PORT        (GPIOA)
#define DRV_LED_YELLOW_PIN         (7U)     /* D11 */
#define DRV_LED_RED_PORT           (GPIOA)
#define DRV_LED_RED_PIN            (6U)     /* D12 */
#define DRV_LED_BLUE_PORT          (GPIOA)
#define DRV_LED_BLUE_PIN           (5U)     /* D13 */

/* ---- ปุ่มบน shield -> EXTI ---------------------------------------------------*/
#define DRV_BTN_START_PORT         (GPIOA)
#define DRV_BTN_START_PIN          (10U)    /* D2 -> EXTI10 : เริ่มเกม */
#define DRV_BTN_RESTART_PORT       (GPIOB)
#define DRV_BTN_RESTART_PIN        (3U)     /* D3 -> EXTI3  : เล่นใหม่หลังตาย */
#define DRV_BTN_PAUSE_PORT         (GPIOB)
#define DRV_BTN_PAUSE_PIN          (5U)     /* D4 -> EXTI5  : พัก / เล่นต่อ */
#define DRV_BTN_MODE_PORT          (GPIOB)
#define DRV_BTN_MODE_PIN           (4U)     /* D5 -> EXTI4  : สลับโหมดเกม */

/* 1U = กดแล้วเป็น LOW (ใช้ pull-up ภายใน) ตรงกับปุ่มบน Training Shield 1
 * 0U = กดแล้วเป็น HIGH (ใช้ pull-down ภายใน) */
#define DRV_BTN_ACTIVE_LOW         (1U)

/* เวลากันปุ่มเด้ง (มิลลิวินาที) */
#define DRV_BTN_DEBOUNCE_MS        (50U)

/* ---- ADC: joystick 2 แกน + pot ปรับความเร็ว ----------------------------------*/
/* joystick ต่อที่ Morpho CN7 แถวใน (ไฟเลี้ยงเอาจาก header AHT10: 3V3 เท่านั้น) */
#define DRV_JOY_X_PORT             (GPIOB)
#define DRV_JOY_X_PIN              (0U)     /* PB0 = ADC1_IN8  : VRx */
#define DRV_JOY_X_ADC_CHANNEL      (8U)
#define DRV_JOY_Y_PORT             (GPIOC)
#define DRV_JOY_Y_PIN              (0U)     /* PC0 = ADC1_IN10 : VRy */
#define DRV_JOY_Y_ADC_CHANNEL      (10U)
#define DRV_POT_PORT               (GPIOA)
#define DRV_POT_PIN                (4U)     /* PA4 = ADC1_IN4  : ป้าย A2 บน shield */
#define DRV_POT_ADC_CHANNEL        (4U)

/* ---- 7-segment ผ่าน BCD decoder บน shield ----------------------------------
 * ผังนี้อ่านจากป้ายบนบอร์ดจริง (ป้ายของ 2^0 ถูกตัวจอครอบทับ ยืนยันแยกต่างหาก)
 *     2^0 : D9 / PC7      2^1 : D7 / PA8
 *     2^2 : D6 / PB10     2^3 : D8 / PA9
 * decoder รับเลข 0-9 · รหัสนอกช่วงทำให้จอดับ */
#define DRV_SEG_B0_PORT            (GPIOC)
#define DRV_SEG_B0_PIN             (7U)     /* D9 */
#define DRV_SEG_B1_PORT            (GPIOA)
#define DRV_SEG_B1_PIN             (8U)     /* D7 */
#define DRV_SEG_B2_PORT            (GPIOB)
#define DRV_SEG_B2_PIN             (10U)    /* D6 */
#define DRV_SEG_B3_PORT            (GPIOA)
#define DRV_SEG_B3_PIN             (9U)     /* D8 */

/* ---- UART: USART2 TX -> ST-Link Virtual COM Port (จอภาพของเกม) -------------*/
#define DRV_UART_TX_PORT           (GPIOA)
#define DRV_UART_TX_PIN            (2U)
#define DRV_UART_TX_AF             (7U)     /* AF7 = USART2 */
/* 115200 bps ที่ 16 MHz: USARTDIV = 8.6875 -> mantissa 8, fraction 11 */
#define DRV_UART_BRR_115200        (0x8BU)

#endif /* DRV_BOARD_H */
