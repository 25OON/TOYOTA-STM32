/*******************************************************************************
 * File Name    : drv_led.c
 * Description  : ควบคุม LED 4 ดวงบน Training Shield 1
 * Date         : 2026-09-15
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_led.h"
#include "Driver/drv_board.h"
#include "Driver/drv_gpio.h"
#include "Driver/drv_mcu.h"

/* Private struct ------------------------------------------------------------*/
typedef struct
{
    GPIO_TypeDef *pPort;
    uint32_t      pin;
} LedPin_t;

/* Private constants ---------------------------------------------------------*/
/* ลำดับต้องตรงกับ DrvLedId_t */
static const LedPin_t s_ledPins[DRV_LED_COUNT] =
{
    { DRV_LED_GREEN_PORT,  DRV_LED_GREEN_PIN  },
    { DRV_LED_YELLOW_PORT, DRV_LED_YELLOW_PIN },
    { DRV_LED_RED_PORT,    DRV_LED_RED_PIN    },
    { DRV_LED_BLUE_PORT,   DRV_LED_BLUE_PIN   }
};

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvLed_Init
 * @brief             - ตั้งขา LED เป็น output และดับทุกดวง
 *//////////////////////////////////////////////////////////////////////
void DrvLed_Init(void)
{
    uint32_t index;

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN;

    for (index = 0U; index < (uint32_t)DRV_LED_COUNT; index++)
    {
        DrvGpio_SetMode(s_ledPins[index].pPort, s_ledPins[index].pin, DRV_GPIO_MODE_OUTPUT);
        DrvGpio_Write(s_ledPins[index].pPort, s_ledPins[index].pin, false);
    }
}

/*********************************************************************
 * @fn                - DrvLed_Set
 * @brief             - เปิดหรือปิด LED หนึ่งดวง
 *
 * @param[in]         - led    : LED ที่ต้องการ
 * @param[in]         - turnOn : true = ติด
 *//////////////////////////////////////////////////////////////////////
void DrvLed_Set(DrvLedId_t led, bool turnOn)
{
    if (led < DRV_LED_COUNT)
    {
        DrvGpio_Write(s_ledPins[led].pPort, s_ledPins[led].pin, turnOn);
    }
}
