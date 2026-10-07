/*******************************************************************************
 * File Name    : drv_gpio.c
 * Description  : ฟังก์ชันพื้นฐานของ GPIO ระดับรีจิสเตอร์
 * Date         : 2026-09-15
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "Driver/drv_gpio.h"

/* Private define ------------------------------------------------------------*/
#define GPIO_PIN_COUNT             (16U)
#define GPIO_FIELD_2BIT_MASK       (0x3UL)
#define GPIO_FIELD_4BIT_MASK       (0xFUL)
#define GPIO_BITS_PER_PIN_2BIT     (2U)
#define GPIO_BITS_PER_PIN_4BIT     (4U)
#define GPIO_PINS_PER_AFR          (8U)
#define GPIO_BSRR_RESET_OFFSET     (16U)

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvGpio_SetMode
 * @brief             - ตั้งโหมดของขา (input / output / alternate / analog)
 *
 * @param[in]         - pPort : พอร์ต GPIO เช่น GPIOA
 * @param[in]         - pin   : หมายเลขขา 0-15
 * @param[in]         - mode  : โหมดที่ต้องการ
 *
 * @return            - none
 *
 * @Note              - ล้างฟิลด์ 2 บิตก่อนแล้วค่อยตั้งค่าใหม่
 *//////////////////////////////////////////////////////////////////////
void DrvGpio_SetMode(GPIO_TypeDef * const pPort, uint32_t pin, DrvGpioMode_t mode)
{
    if (pin < GPIO_PIN_COUNT)
    {
        uint32_t shift = pin * GPIO_BITS_PER_PIN_2BIT;
        uint32_t modeValue = (uint32_t)mode;
        uint32_t moder = pPort->MODER;

        moder &= ~(GPIO_FIELD_2BIT_MASK << shift);
        moder |= (modeValue << shift);
        pPort->MODER = moder;
    }
}

/*********************************************************************
 * @fn                - DrvGpio_SetPull
 * @brief             - ตั้งตัวต้านทาน pull-up / pull-down ภายในของขา
 *
 * @param[in]         - pPort : พอร์ต GPIO
 * @param[in]         - pin   : หมายเลขขา 0-15
 * @param[in]         - pull  : ชนิด pull
 *
 * @return            - none
 *
 * @Note              - none
 *//////////////////////////////////////////////////////////////////////
void DrvGpio_SetPull(GPIO_TypeDef * const pPort, uint32_t pin, DrvGpioPull_t pull)
{
    if (pin < GPIO_PIN_COUNT)
    {
        uint32_t shift = pin * GPIO_BITS_PER_PIN_2BIT;
        uint32_t pullValue = (uint32_t)pull;
        uint32_t pupdr = pPort->PUPDR;

        pupdr &= ~(GPIO_FIELD_2BIT_MASK << shift);
        pupdr |= (pullValue << shift);
        pPort->PUPDR = pupdr;
    }
}

/*********************************************************************
 * @fn                - DrvGpio_SetAltFunction
 * @brief             - เลือก alternate function (AF0-AF15) ของขา
 *
 * @param[in]         - pPort       : พอร์ต GPIO
 * @param[in]         - pin         : หมายเลขขา 0-15
 * @param[in]         - altFunction : หมายเลข AF เช่น 7 = USART2
 *
 * @return            - none
 *
 * @Note              - ต้องตั้งโหมดเป็น DRV_GPIO_MODE_ALTFN แยกต่างหาก
 *//////////////////////////////////////////////////////////////////////
void DrvGpio_SetAltFunction(GPIO_TypeDef * const pPort, uint32_t pin, uint32_t altFunction)
{
    if (pin < GPIO_PIN_COUNT)
    {
        uint32_t index = pin / GPIO_PINS_PER_AFR;
        uint32_t shift = (pin % GPIO_PINS_PER_AFR) * GPIO_BITS_PER_PIN_4BIT;
        uint32_t afr = pPort->AFR[index];

        afr &= ~(GPIO_FIELD_4BIT_MASK << shift);
        afr |= ((altFunction & GPIO_FIELD_4BIT_MASK) << shift);
        pPort->AFR[index] = afr;
    }
}

/*********************************************************************
 * @fn                - DrvGpio_Write
 * @brief             - สั่งขา output เป็น HIGH หรือ LOW
 *
 * @param[in]         - pPort     : พอร์ต GPIO
 * @param[in]         - pin       : หมายเลขขา 0-15
 * @param[in]         - levelHigh : true = HIGH, false = LOW
 *
 * @return            - none
 *
 * @Note              - ใช้ BSRR เพราะเขียนครั้งเดียวจบ (atomic)
 *//////////////////////////////////////////////////////////////////////
void DrvGpio_Write(GPIO_TypeDef * const pPort, uint32_t pin, bool levelHigh)
{
    if (pin < GPIO_PIN_COUNT)
    {
        if (levelHigh)
        {
            pPort->BSRR = (uint32_t)1U << pin;
        }
        else
        {
            pPort->BSRR = (uint32_t)1U << (pin + GPIO_BSRR_RESET_OFFSET);
        }
    }
}

/*********************************************************************
 * @fn                - DrvGpio_ReadHigh
 * @brief             - อ่านระดับของขา input
 *
 * @param[in]         - pPort : พอร์ต GPIO
 * @param[in]         - pin   : หมายเลขขา 0-15
 *
 * @return            - true ถ้าขาเป็น HIGH
 *
 * @Note              - none
 *//////////////////////////////////////////////////////////////////////
bool DrvGpio_ReadHigh(const GPIO_TypeDef * const pPort, uint32_t pin)
{
    bool isHigh = false;

    if (pin < GPIO_PIN_COUNT)
    {
        isHigh = ((pPort->IDR & ((uint32_t)1U << pin)) != 0U);
    }

    return isHigh;
}
