/*******************************************************************************
 * File Name    : drv_tick.c
 * Description  : TIM2 จังหวะ 25 ms (ฐานเวลาของเกม: 40 ครั้งต่อวินาที)
 *                 - update interrupt นับ tick ให้ชั้น App
 *                 - TRGO (update event) สั่ง ADC1 เริ่มสแกนโดยตรง
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_tick.h"
#include "Driver/drv_mcu.h"
#include "Driver/drv_isr.h"
#include "Driver/drv_critical.h"

/* Private define ------------------------------------------------------------*/
/* 16 MHz / (1599 + 1) = 10 kHz, นับ 250 ครั้ง = 25 ms
 * 25 ms เป็นฐานเวลาเดียวของเกม: หนึ่งก้าวของงู = 4-10 tick ตามระดับความเร็ว
 * และ ADC ถูกทริกด้วย TRGO ทุก tick (40 ตัวอย่างต่อวินาที อ่าน joystick ทัน) */
#define TIM2_PRESCALER             (1599U)
#define TIM2_AUTO_RELOAD           (249U)
#define TIM2_MMS_UPDATE_EVENT      (2U)     /* TRGO = update event */
#define TIM2_IRQ_PRIORITY          (2U)
#define TICK_PENDING_LIMIT         (0xFFFFFFFFUL)

/* Private variables ---------------------------------------------------------*/
static volatile uint32_t s_pendingTicks = 0U;

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvTick_Init
 * @brief             - ตั้ง prescaler, คาบ, TRGO และ interrupt ของ TIM2
 *//////////////////////////////////////////////////////////////////////
void DrvTick_Init(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;

    TIM2->CR1 = 0U;
    TIM2->PSC = TIM2_PRESCALER;
    TIM2->ARR = TIM2_AUTO_RELOAD;

    TIM2->CR2 &= ~TIM_CR2_MMS;
    TIM2->CR2 |= ((uint32_t)TIM2_MMS_UPDATE_EVENT << TIM_CR2_MMS_Pos);

    /* บังคับโหลด PSC ทันที แล้วล้างธง UIF ที่เกิดจากการบังคับนี้ */
    TIM2->EGR = TIM_EGR_UG;
    TIM2->SR = 0U;

    TIM2->DIER |= TIM_DIER_UIE;
    NVIC_SetPriority(TIM2_IRQn, TIM2_IRQ_PRIORITY);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/*********************************************************************
 * @fn                - DrvTick_Start
 * @brief             - เริ่มนับ (ADC จะเริ่มถูกสั่งทุก 25 ms จากจุดนี้)
 *//////////////////////////////////////////////////////////////////////
void DrvTick_Start(void)
{
    TIM2->CR1 |= TIM_CR1_CEN;
}

/*********************************************************************
 * @fn                - DrvTick_TakePending
 * @brief             - รับจำนวน tick ที่ค้างอยู่แล้วล้างเป็น 0
 *
 * @return            - จำนวน tick ตั้งแต่เรียกครั้งก่อน
 *//////////////////////////////////////////////////////////////////////
uint32_t DrvTick_TakePending(void)
{
    uint32_t primask = DrvCritical_Enter();
    uint32_t ticks = s_pendingTicks;

    s_pendingTicks = 0U;
    DrvCritical_Exit(primask);

    return ticks;
}

/* Callback functions --------------------------------------------------------*/

void TIM2_IRQHandler(void)
{
    if ((TIM2->SR & TIM_SR_UIF) != 0U)
    {
        TIM2->SR = ~TIM_SR_UIF;     /* rc_w0: เขียน 0 เพื่อล้างเฉพาะ UIF */

        if (s_pendingTicks < TICK_PENDING_LIMIT)
        {
            s_pendingTicks++;
        }
    }
}
