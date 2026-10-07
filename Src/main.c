/*******************************************************************************
 * File Name    : main.c
 * Description  : Snake บน NUCLEO-F411RE + Training Shield 1
 *                 main() ทำหน้าที่เรียกชั้น Application เท่านั้น
 *                 - Src/App    : ตรรกะเกม, state machine, วาดฉาก
 *                 - Src/Driver : GPIO, EXTI, TIM2, ADC1+DMA2, USART2+DMA1, SysTick
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "App/app_main.h"

/* Main function -------------------------------------------------------------*/
int main(void)
{
    App_Init();

    for (;;)
    {
        App_Run();
    }
}
