/*******************************************************************************
 * File Name    : drv_isr.h
 * Description  : ประกาศ prototype ของ interrupt handler ที่ชั้น Driver เขียนทับ
 *                 ชื่อต้องตรงกับตาราง vector ใน startup_stm32f411retx.s
 *                 (ให้มี declaration ก่อน definition ตาม MISRA 8.4)
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_ISR_H
#define DRV_ISR_H

void SysTick_Handler(void);
void TIM2_IRQHandler(void);
void ADC_IRQHandler(void);
void DMA2_Stream0_IRQHandler(void);
void DMA1_Stream6_IRQHandler(void);
void EXTI3_IRQHandler(void);
void EXTI4_IRQHandler(void);
void EXTI9_5_IRQHandler(void);
void EXTI15_10_IRQHandler(void);

#endif /* DRV_ISR_H */
