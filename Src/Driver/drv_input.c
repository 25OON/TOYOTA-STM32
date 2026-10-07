/*******************************************************************************
 * File Name    : drv_input.c
 * Description  : EXTI ของปุ่ม D2/D3/D4/D5
 *                 ขอบที่กด -> กันเด้ง 50 ms -> เก็บเป็นบิตเหตุการณ์ให้ชั้น App
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include <stdint.h>
#include "Driver/drv_input.h"
#include "Driver/drv_board.h"
#include "Driver/drv_critical.h"
#include "Driver/drv_gpio.h"
#include "Driver/drv_isr.h"
#include "Driver/drv_mcu.h"
#include "Driver/drv_system.h"

/* Private define ------------------------------------------------------------*/
#define BUTTON_COUNT               (4U)
#define BUTTON_INDEX_START         (0U)
#define BUTTON_INDEX_RESTART       (1U)
#define BUTTON_INDEX_PAUSE         (2U)
#define BUTTON_INDEX_MODE          (3U)

#define EXTI_LINE_START            ((uint32_t)1U << DRV_BTN_START_PIN)
#define EXTI_LINE_RESTART          ((uint32_t)1U << DRV_BTN_RESTART_PIN)
#define EXTI_LINE_PAUSE            ((uint32_t)1U << DRV_BTN_PAUSE_PIN)
#define EXTI_LINE_MODE             ((uint32_t)1U << DRV_BTN_MODE_PIN)
#define EXTI_BUTTON_LINES          (EXTI_LINE_START | EXTI_LINE_RESTART |                                     EXTI_LINE_PAUSE | EXTI_LINE_MODE)

#define EXTI_IRQ_PRIORITY          (2U)

/* Private struct ------------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef *pPort;
    uint32_t      pin;
    uint8_t       eventBit;
} ButtonPin_t;

/* Private constants ---------------------------------------------------------*/
static const ButtonPin_t s_buttons[BUTTON_COUNT] =
{
    { DRV_BTN_START_PORT,   DRV_BTN_START_PIN,   0x01U },
    { DRV_BTN_RESTART_PORT, DRV_BTN_RESTART_PIN, 0x02U },
    { DRV_BTN_PAUSE_PORT,   DRV_BTN_PAUSE_PIN,   0x04U },
    { DRV_BTN_MODE_PORT,    DRV_BTN_MODE_PIN,    0x08U }
};

/* Private variables ---------------------------------------------------------*/
static volatile uint8_t s_buttonEventBits = 0U;
static uint32_t s_lastEdgeMs[BUTTON_COUNT];

/* Private function prototypes -----------------------------------------------*/
static bool Input_IsButtonActive(uint32_t buttonIndex);
static void Input_OnButtonEdge(uint32_t buttonIndex);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvInput_Init
 * @brief             - ตั้งขา input, เชื่อม EXTI กับพอร์ต และเปิด NVIC
 *
 * @Note              - EXTI3 <- PB3, EXTI4 <- PB4, EXTI5 <- PB5, EXTI10 <- PA10
 *                       EXTI4 มี IRQ ของตัวเอง ส่วน EXTI5 ใช้ร่วมกับสาย 5-9
 *//////////////////////////////////////////////////////////////////////
void DrvInput_Init(void)
{
    uint32_t index;
    DrvGpioPull_t buttonPull = DRV_GPIO_PULL_DOWN;

#if (DRV_BTN_ACTIVE_LOW == 1U)
    buttonPull = DRV_GPIO_PULL_UP;
#endif

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;
    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;

    for (index = 0U; index < BUTTON_COUNT; index++)
    {
        DrvGpio_SetMode(s_buttons[index].pPort, s_buttons[index].pin, DRV_GPIO_MODE_INPUT);
        DrvGpio_SetPull(s_buttons[index].pPort, s_buttons[index].pin, buttonPull);
        s_lastEdgeMs[index] = 0U;
    }

    /* เชื่อมสาย EXTI กับพอร์ต */
    SYSCFG->EXTICR[0] &= ~SYSCFG_EXTICR1_EXTI3;
    SYSCFG->EXTICR[0] |= SYSCFG_EXTICR1_EXTI3_PB;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI4;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI4_PB;
    SYSCFG->EXTICR[1] &= ~SYSCFG_EXTICR2_EXTI5;
    SYSCFG->EXTICR[1] |= SYSCFG_EXTICR2_EXTI5_PB;
    SYSCFG->EXTICR[2] &= ~SYSCFG_EXTICR3_EXTI10;
    SYSCFG->EXTICR[2] |= SYSCFG_EXTICR3_EXTI10_PA;

    /* เปิดเฉพาะขอบที่ตรงกับจังหวะ "กด" */
#if (DRV_BTN_ACTIVE_LOW == 1U)
    EXTI->FTSR |= EXTI_BUTTON_LINES;
    EXTI->RTSR &= ~EXTI_BUTTON_LINES;
#else
    EXTI->RTSR |= EXTI_BUTTON_LINES;
    EXTI->FTSR &= ~EXTI_BUTTON_LINES;
#endif

    EXTI->PR = EXTI_BUTTON_LINES;   /* ล้างธงค้าง (เขียน 1 เพื่อล้าง) */
    EXTI->IMR |= EXTI_BUTTON_LINES;

    NVIC_SetPriority(EXTI3_IRQn, EXTI_IRQ_PRIORITY);
    NVIC_SetPriority(EXTI4_IRQn, EXTI_IRQ_PRIORITY);
    NVIC_SetPriority(EXTI9_5_IRQn, EXTI_IRQ_PRIORITY);
    NVIC_SetPriority(EXTI15_10_IRQn, EXTI_IRQ_PRIORITY);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

/*********************************************************************
 * @fn                - DrvInput_TakeButtonEvents
 * @brief             - รับเหตุการณ์ปุ่มที่เกิดขึ้นแล้วล้างทิ้ง
 *
 * @param[out]        - pEvents : ปุ่มที่ถูกกดตั้งแต่เรียกครั้งก่อน
 *//////////////////////////////////////////////////////////////////////
void DrvInput_TakeButtonEvents(DrvButtonEvents_t * const pEvents)
{
    if (pEvents != NULL)
    {
        uint32_t primask = DrvCritical_Enter();
        uint8_t bits = s_buttonEventBits;

        s_buttonEventBits = 0U;
        DrvCritical_Exit(primask);

        pEvents->start   = ((bits & s_buttons[BUTTON_INDEX_START].eventBit) != 0U);
        pEvents->restart = ((bits & s_buttons[BUTTON_INDEX_RESTART].eventBit) != 0U);
        pEvents->pause   = ((bits & s_buttons[BUTTON_INDEX_PAUSE].eventBit) != 0U);
        pEvents->mode    = ((bits & s_buttons[BUTTON_INDEX_MODE].eventBit) != 0U);
    }
}

/* Private functions ---------------------------------------------------------*/

/* อ่านว่าปุ่มกำลังถูกกดอยู่หรือไม่ ตามระดับ active ที่ตั้งไว้ */
static bool Input_IsButtonActive(uint32_t buttonIndex)
{
    bool pinHigh = DrvGpio_ReadHigh(s_buttons[buttonIndex].pPort, s_buttons[buttonIndex].pin);
    bool isActive = pinHigh;

#if (DRV_BTN_ACTIVE_LOW == 1U)
    isActive = !pinHigh;
#endif

    return isActive;
}

/* เรียกจาก ISR เมื่อเกิดขอบของปุ่ม: ข้ามขอบที่มาเร็วกว่าเวลากันเด้ง */
static void Input_OnButtonEdge(uint32_t buttonIndex)
{
    uint32_t nowMs = DrvSystem_GetMs();

    if ((nowMs - s_lastEdgeMs[buttonIndex]) >= DRV_BTN_DEBOUNCE_MS)
    {
        s_lastEdgeMs[buttonIndex] = nowMs;

        if (Input_IsButtonActive(buttonIndex))
        {
            s_buttonEventBits |= s_buttons[buttonIndex].eventBit;
        }
    }
}

/* Callback functions --------------------------------------------------------*/

void EXTI3_IRQHandler(void)
{
    if ((EXTI->PR & EXTI_LINE_RESTART) != 0U)
    {
        EXTI->PR = EXTI_LINE_RESTART;
        Input_OnButtonEdge(BUTTON_INDEX_RESTART);
    }
}

void EXTI4_IRQHandler(void)
{
    if ((EXTI->PR & EXTI_LINE_MODE) != 0U)
    {
        EXTI->PR = EXTI_LINE_MODE;
        Input_OnButtonEdge(BUTTON_INDEX_MODE);
    }
}

void EXTI9_5_IRQHandler(void)
{
    if ((EXTI->PR & EXTI_LINE_PAUSE) != 0U)
    {
        EXTI->PR = EXTI_LINE_PAUSE;
        Input_OnButtonEdge(BUTTON_INDEX_PAUSE);
    }
}

void EXTI15_10_IRQHandler(void)
{
    if ((EXTI->PR & EXTI_LINE_START) != 0U)
    {
        EXTI->PR = EXTI_LINE_START;
        Input_OnButtonEdge(BUTTON_INDEX_START);
    }
}
