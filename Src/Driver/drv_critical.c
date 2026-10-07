/*******************************************************************************
 * File Name    : drv_critical.c
 * Description  : ช่วงป้องกันด้วย PRIMASK ของ Cortex-M4
 * Date         : 2026-09-15
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_critical.h"
#include "Driver/drv_mcu.h"

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvCritical_Enter
 * @brief             - ปิด interrupt ทั้งหมดชั่วคราว
 *
 * @return            - ค่า PRIMASK ก่อนปิด (ส่งต่อให้ DrvCritical_Exit)
 *
 * @Note              - เรียกซ้อนกันได้ เพราะเก็บสถานะเดิมไว้
 *//////////////////////////////////////////////////////////////////////
uint32_t DrvCritical_Enter(void)
{
    uint32_t primask = __get_PRIMASK();

    __disable_irq();

    return primask;
}

/*********************************************************************
 * @fn                - DrvCritical_Exit
 * @brief             - เปิด interrupt คืน ถ้าก่อนเข้าเปิดอยู่
 *
 * @param[in]         - primask : ค่าที่ได้จาก DrvCritical_Enter()
 *
 * @return            - none
 *//////////////////////////////////////////////////////////////////////
void DrvCritical_Exit(uint32_t primask)
{
    if (primask == 0U)
    {
        __enable_irq();
    }
}
