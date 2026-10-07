/*******************************************************************************
 * File Name    : drv_rand.c
 * Description  : xorshift32 (Marsaglia) — คาบ 2^32-1 ใช้แค่ shift กับ xor
 *                 เร็วพอสำหรับการวางอาหารในเกม และไม่ต้องพึ่ง stdlib
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_rand.h"

/* Private define ------------------------------------------------------------*/
#define RAND_FALLBACK_SEED         (0x1D872B41UL)   /* ใช้เมื่อ seed เป็น 0 */
#define RAND_SHIFT_A               (13U)
#define RAND_SHIFT_B               (17U)
#define RAND_SHIFT_C               (5U)

/* Private variables ---------------------------------------------------------*/
static uint32_t s_state = RAND_FALLBACK_SEED;

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvRand_Seed
 * @brief             - ตั้งค่าเริ่มต้นของลำดับสุ่ม
 *
 * @param[in]         - seed : ค่าเริ่มต้น (ปกติมาจากบิตล่างของ ADC ตอนบูต)
 *
 * @Note              - xorshift จะติดอยู่ที่ 0 ตลอดไปถ้า state เป็น 0
 *//////////////////////////////////////////////////////////////////////
void DrvRand_Seed(uint32_t seed)
{
    if (seed == 0UL)
    {
        s_state = RAND_FALLBACK_SEED;
    }
    else
    {
        s_state = seed;
    }
}

/*********************************************************************
 * @fn                - DrvRand_Next
 * @brief             - คืนเลขสุ่ม 32 บิตถัดไปของลำดับ
 *//////////////////////////////////////////////////////////////////////
uint32_t DrvRand_Next(void)
{
    uint32_t value = s_state;

    value ^= (value << RAND_SHIFT_A);
    value ^= (value >> RAND_SHIFT_B);
    value ^= (value << RAND_SHIFT_C);
    s_state = value;

    return value;
}

/*********************************************************************
 * @fn                - DrvRand_Below
 * @brief             - คืนเลขสุ่มในช่วง 0 .. (limit - 1)
 *
 * @param[in]         - limit : ขอบบน (ไม่รวม)
 *
 * @return            - เลขสุ่มในช่วง หรือ 0 ถ้า limit เป็น 0
 *
 * @Note              - ใช้บิตบนของ state เพราะบิตล่างของ xorshift
 *                       กระจายตัวแย่กว่า จึง shift ขวา 16 ก่อนหารเศษ
 *//////////////////////////////////////////////////////////////////////
uint16_t DrvRand_Below(uint16_t limit)
{
    uint16_t result = 0U;

    if (limit > 0U)
    {
        uint32_t raw = DrvRand_Next() >> 16U;

        result = (uint16_t)(raw % (uint32_t)limit);
    }

    return result;
}
