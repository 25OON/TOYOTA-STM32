/*******************************************************************************
 * File Name    : drv_system.c
 * Description  : SystemInit (เปิด FPU) และ SysTick 1 ms สำหรับนับเวลา
 * Date         : 2026-09-15
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_system.h"
#include "Driver/drv_mcu.h"
#include "Driver/drv_isr.h"

/* Private define ------------------------------------------------------------*/
#define SYSTICK_RATE_HZ            (1000UL)
/* CP10 และ CP11 = full access (bit 20-23 ของ CPACR) */
#define FPU_CP10_CP11_FULL_ACCESS  ((uint32_t)0xFUL << 20U)

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t s_msTicks = 0U;

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - SystemInit
 * @brief             - startup เรียกก่อน main() ใช้เปิด FPU
 *
 * @Note              - โปรเจกต์คอมไพล์แบบ hard-float จึงต้องเปิด FPU
 *                       ก่อนมีการใช้ float ไม่อย่างนั้นจะเกิด HardFault
 *                       ห้ามแตะตัวแปรใน .data/.bss เพราะยังไม่ถูกเตรียม
 *//////////////////////////////////////////////////////////////////////
void SystemInit(void)
{
    SCB->CPACR |= FPU_CP10_CP11_FULL_ACCESS;
}

/*********************************************************************
 * @fn                - DrvSystem_Init
 * @brief             - ตั้ง SysTick ให้ขัดจังหวะทุก 1 ms
 *//////////////////////////////////////////////////////////////////////
void DrvSystem_Init(void)
{
    (void)SysTick_Config(DRV_SYSTEM_CLOCK_HZ / SYSTICK_RATE_HZ);
}

/*********************************************************************
 * @fn                - DrvSystem_GetMs
 * @brief             - คืนเวลาเป็นมิลลิวินาที
 *
 * @Note              - อ่านค่า 32 บิตครั้งเดียวบน Cortex-M4 เป็น atomic
 *//////////////////////////////////////////////////////////////////////
uint32_t DrvSystem_GetMs(void)
{
    return s_msTicks;
}

/* Callback functions --------------------------------------------------------*/

void SysTick_Handler(void)
{
    s_msTicks++;
}
