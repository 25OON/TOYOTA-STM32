/*******************************************************************************
 * File Name    : drv_seg.c
 * Description  : ขับ 7-segment ผ่าน BCD decoder บน shield
 *                 ชั้นนี้รู้แค่ "เลขหนึ่งหลัก" ไม่รู้ว่าเลขนั้นคืออะไรของเกม
 *                 ผังขาอยู่ที่ drv_board.h (2^0 = PC7 อ่านจากบอร์ดจริงแล้ว)
 * Date         : 2026-10-05
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_seg.h"
#include "Driver/drv_board.h"
#include "Driver/drv_gpio.h"
#include "Driver/drv_mcu.h"

/* Private define ------------------------------------------------------------*/
#define SEG_BIT_COUNT              (4U)
#define SEG_BIT_MASK               (0x01U)
/* decoder ดับจอเมื่อรหัสอยู่นอกช่วง 0-9 · ใช้ 0xF เป็นรหัสดับ */
#define SEG_BLANK_CODE             (0x0FU)

/* Private struct ------------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef *pPort;
    uint32_t      pin;
} SegPin_t;

/* Private constants ---------------------------------------------------------*/
/* เรียงจากบิตต่ำไปบิตสูง: 2^0, 2^1, 2^2, 2^3 */
static const SegPin_t s_segPins[SEG_BIT_COUNT] =
{
    { DRV_SEG_B0_PORT, DRV_SEG_B0_PIN },
    { DRV_SEG_B1_PORT, DRV_SEG_B1_PIN },
    { DRV_SEG_B2_PORT, DRV_SEG_B2_PIN },
    { DRV_SEG_B3_PORT, DRV_SEG_B3_PIN }
};

/* Private function prototypes -----------------------------------------------*/
static void Seg_WriteCode(uint8_t code);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvSeg_Init
 * @brief             - ตั้งขา BCD ทั้งสี่เป็น output แล้วดับจอไว้ก่อน
 *
 * @Note              - 2^0 ใช้ PC7 จึงต้องเปิดคล็อกของ GPIOC ด้วย
 *//////////////////////////////////////////////////////////////////////
void DrvSeg_Init(void)
{
    uint32_t index;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;

    for (index = 0U; index < SEG_BIT_COUNT; index++)
    {
        DrvGpio_SetPull(s_segPins[index].pPort, s_segPins[index].pin, DRV_GPIO_PULL_NONE);
        DrvGpio_SetMode(s_segPins[index].pPort, s_segPins[index].pin, DRV_GPIO_MODE_OUTPUT);
    }

    DrvSeg_Blank();
}

/*********************************************************************
 * @fn                - DrvSeg_ShowDigit
 * @brief             - แสดงเลขหนึ่งหลักบน 7-segment
 *
 * @param[in]         - digit : 0 - 9 (เกินช่วงนี้จอจะดับ)
 *//////////////////////////////////////////////////////////////////////
void DrvSeg_ShowDigit(uint8_t digit)
{
    uint8_t code = SEG_BLANK_CODE;

    if (digit <= (uint8_t)DRV_SEG_MAX_DIGIT)
    {
        code = digit;
    }

    Seg_WriteCode(code);
}

/*********************************************************************
 * @fn                - DrvSeg_Blank
 * @brief             - ดับจอ
 *//////////////////////////////////////////////////////////////////////
void DrvSeg_Blank(void)
{
    Seg_WriteCode((uint8_t)SEG_BLANK_CODE);
}

/* Private functions ---------------------------------------------------------*/

/* ส่งรหัส 4 บิตออกขา BCD ทีละบิต (บิตต่ำสุดก่อน) */
static void Seg_WriteCode(uint8_t code)
{
    uint32_t index;

    for (index = 0U; index < SEG_BIT_COUNT; index++)
    {
        uint8_t bit = (uint8_t)(((uint32_t)code >> index) & SEG_BIT_MASK);

        DrvGpio_Write(s_segPins[index].pPort, s_segPins[index].pin, (bit != 0U));
    }
}
