/*******************************************************************************
 * File Name    : drv_wdg.c
 * Description  : IWDG วิ่งด้วย LSI ~32 kHz
 *                 prescaler /64 -> 500 Hz (1 นับ = 2 ms), RLR = 249 -> 500 ms
 *                 - อ่าน RCC->CSR บิต IWDGRSTF ก่อน แล้วล้างด้วย RMVF
 *                 - หยุด watchdog ตอน core ถูก debugger หยุดไว้ (DBGMCU)
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_wdg.h"
#include "Driver/drv_mcu.h"

/* Private define ------------------------------------------------------------*/
#define WDG_KEY_ACCESS             (0x5555UL)  /* ปลดล็อก PR/RLR */
#define WDG_KEY_RELOAD             (0xAAAAUL)  /* เตะ watchdog */
#define WDG_KEY_START              (0xCCCCUL)  /* เริ่มนับ (ปิดไม่ได้อีก) */
#define WDG_PRESCALER_DIV64        (4UL)       /* PR = 100 */
#define WDG_RELOAD_500MS           (249UL)     /* 250 นับ x 2 ms */
#define WDG_SYNC_WAIT_LIMIT        (100000UL)  /* กันลูปค้างตอนรอ PVU/RVU */

/* Private variables ---------------------------------------------------------*/
static bool s_wasWatchdogReset = false;

/* Private function prototypes -----------------------------------------------*/
static void Wdg_WaitStatusClear(void);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvWdg_Init
 * @brief             - จำสาเหตุการรีเซ็ตรอบก่อน แล้วเปิด IWDG คาบ 500 ms
 *
 * @Note              - ต้องอ่าน IWDGRSTF ก่อนล้าง ไม่อย่างนั้นจะไม่รู้ว่า
 *                       รอบก่อนถูก watchdog รีเซ็ต
 *                       เปิดแล้วปิดไม่ได้จนกว่าจะรีเซ็ตชิป
 *//////////////////////////////////////////////////////////////////////
void DrvWdg_Init(void)
{
    s_wasWatchdogReset = ((RCC->CSR & RCC_CSR_IWDGRSTF) != 0U);
    RCC->CSR |= RCC_CSR_RMVF;

    /* ให้ watchdog หยุดนับตอน debugger สั่งหยุด core ไม่อย่างนั้น
     * การ breakpoint ทีเดียวจะทำให้บอร์ดรีเซ็ตทันที */
    DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_IWDG_STOP;

    IWDG->KR = WDG_KEY_ACCESS;
    IWDG->PR = WDG_PRESCALER_DIV64;
    IWDG->RLR = WDG_RELOAD_500MS;
    Wdg_WaitStatusClear();
    IWDG->KR = WDG_KEY_RELOAD;
    IWDG->KR = WDG_KEY_START;
}

/*********************************************************************
 * @fn                - DrvWdg_Reload
 * @brief             - เตะ watchdog ให้เริ่มนับใหม่
 *//////////////////////////////////////////////////////////////////////
void DrvWdg_Reload(void)
{
    IWDG->KR = WDG_KEY_RELOAD;
}

/*********************************************************************
 * @fn                - DrvWdg_WasWatchdogReset
 * @brief             - บอกว่าการบูตรอบนี้มาจาก IWDG รีเซ็ตหรือไม่
 *//////////////////////////////////////////////////////////////////////
bool DrvWdg_WasWatchdogReset(void)
{
    return s_wasWatchdogReset;
}

/* Private functions ---------------------------------------------------------*/

/* รอให้ค่า PR/RLR ถูกคัดลอกเข้าโดเมนคล็อก LSI เสร็จ (มีตัวกันลูปค้าง) */
static void Wdg_WaitStatusClear(void)
{
    uint32_t guard = 0U;

    while (((IWDG->SR & (IWDG_SR_PVU | IWDG_SR_RVU)) != 0U) &&
           (guard < WDG_SYNC_WAIT_LIMIT))
    {
        guard++;
    }
}
