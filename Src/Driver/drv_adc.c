/*******************************************************************************
 * File Name    : drv_adc.c
 * Description  : ADC1 สแกน IN8 (VRx) -> IN10 (VRy) -> IN4 (pot)
 *                 - ทริกเกอร์ภายนอก: TIM2 TRGO ขอบขาขึ้น (ทุก 25 ms)
 *                 - DMA2 Stream0 Channel0 แบบ circular คัดลอกผลลง buffer
 *                 - DMA transfer-complete interrupt แจ้งว่าได้ค่าครบทั้งชุด
 *                 - ADC overrun interrupt สั่งเริ่ม DMA ใหม่
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "Driver/drv_adc.h"
#include "Driver/drv_board.h"
#include "Driver/drv_critical.h"
#include "Driver/drv_gpio.h"
#include "Driver/drv_isr.h"
#include "Driver/drv_mcu.h"

/* Private define ------------------------------------------------------------*/
#define ADC_CHANNEL_COUNT          (3U)
#define ADC_RANK_JOY_X             (0U)     /* ลำดับที่ 1 ในการสแกน */
#define ADC_RANK_JOY_Y             (1U)     /* ลำดับที่ 2 */
#define ADC_RANK_POT               (2U)     /* ลำดับที่ 3 */
#define ADC_EXTSEL_TIM2_TRGO       (6U)     /* EXTSEL = 0110 */
#define ADC_EXTEN_RISING           (1U)
#define ADC_DMA_CHANNEL            (0U)     /* ADC1 = DMA2 Stream0 Channel0 */
#define ADC_IRQ_PRIORITY           (1U)
#define DMA_DISABLE_WAIT_LIMIT     (10000UL)
#define DMA2_STREAM0_ALL_FLAGS     (DMA_LIFCR_CFEIF0 | DMA_LIFCR_CDMEIF0 | \
                                    DMA_LIFCR_CTEIF0 | DMA_LIFCR_CHTIF0 | \
                                    DMA_LIFCR_CTCIF0)

/* Private variables ---------------------------------------------------------*/
static uint16_t s_dmaBuffer[ADC_CHANNEL_COUNT];
static volatile uint16_t s_joyXRaw = 0U;
static volatile uint16_t s_joyYRaw = 0U;
static volatile uint16_t s_potRaw = 0U;
static volatile bool s_sampleReady = false;
static volatile uint32_t s_overrunCount = 0U;

/* Private function prototypes -----------------------------------------------*/
static void Adc_StopDmaStream(void);
static void Adc_StartDmaStream(void);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvAdc_Init
 * @brief             - ตั้งขา analog, DMA2 Stream0 และ ADC1 ให้สแกน 3 ช่อง
 *
 * @Note              - ADC ยังไม่แปลงค่าจนกว่า TIM2 จะเริ่มนับ
 *                       PC0 ต้องเปิดคล็อกของ GPIOC ด้วย
 *//////////////////////////////////////////////////////////////////////
void DrvAdc_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN |
                    RCC_AHB1ENR_GPIOCEN | RCC_AHB1ENR_DMA2EN;
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;

    DrvGpio_SetMode(DRV_JOY_X_PORT, DRV_JOY_X_PIN, DRV_GPIO_MODE_ANALOG);
    DrvGpio_SetMode(DRV_JOY_Y_PORT, DRV_JOY_Y_PIN, DRV_GPIO_MODE_ANALOG);
    DrvGpio_SetMode(DRV_POT_PORT, DRV_POT_PIN, DRV_GPIO_MODE_ANALOG);

    /* ---- DMA2 Stream0: ADC1->DR (16 บิต) -> s_dmaBuffer ---- */
    Adc_StopDmaStream();
    DMA2_Stream0->PAR = (uint32_t)&ADC1->DR;
    DMA2_Stream0->CR = ((uint32_t)ADC_DMA_CHANNEL << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_PSIZE_0 |      /* peripheral 16 บิต */
                       DMA_SxCR_MSIZE_0 |      /* memory 16 บิต */
                       DMA_SxCR_MINC |
                       DMA_SxCR_CIRC |
                       DMA_SxCR_TCIE |
                       DMA_SxCR_TEIE;
    NVIC_SetPriority(DMA2_Stream0_IRQn, ADC_IRQ_PRIORITY);
    NVIC_EnableIRQ(DMA2_Stream0_IRQn);
    Adc_StartDmaStream();

    /* ---- ADC1: scan 3 ช่อง, 12 บิต, 480 cycles ต่อช่อง ---- */
    ADC1->CR2 = 0U;
    ADC1->CR1 = ADC_CR1_SCAN | ADC_CR1_OVRIE;
    ADC1->SMPR1 = ADC_SMPR1_SMP10;                      /* ช่อง 10 (VRy) */
    ADC1->SMPR2 = ADC_SMPR2_SMP4 | ADC_SMPR2_SMP8;      /* ช่อง 4, 8     */
    ADC1->SQR1 = ((uint32_t)(ADC_CHANNEL_COUNT - 1U) << ADC_SQR1_L_Pos);
    ADC1->SQR3 = ((uint32_t)DRV_JOY_X_ADC_CHANNEL << ADC_SQR3_SQ1_Pos) |
                 ((uint32_t)DRV_JOY_Y_ADC_CHANNEL << ADC_SQR3_SQ2_Pos) |
                 ((uint32_t)DRV_POT_ADC_CHANNEL   << ADC_SQR3_SQ3_Pos);
    ADC1->CR2 = ADC_CR2_DMA |
                ADC_CR2_DDS |
                ((uint32_t)ADC_EXTSEL_TIM2_TRGO << ADC_CR2_EXTSEL_Pos) |
                ((uint32_t)ADC_EXTEN_RISING << ADC_CR2_EXTEN_Pos);

    NVIC_SetPriority(ADC_IRQn, ADC_IRQ_PRIORITY);
    NVIC_EnableIRQ(ADC_IRQn);

    ADC1->CR2 |= ADC_CR2_ADON;
}

/*********************************************************************
 * @fn                - DrvAdc_GetSample
 * @brief             - รับค่าชุดล่าสุดถ้ามีค่าใหม่
 *
 * @param[out]        - pSample : ที่เก็บผลลัพธ์
 *
 * @return            - true ถ้าได้ค่าใหม่ตั้งแต่เรียกครั้งก่อน
 *//////////////////////////////////////////////////////////////////////
bool DrvAdc_GetSample(DrvAdcSample_t * const pSample)
{
    bool gotSample = false;

    if (pSample != NULL)
    {
        uint32_t primask = DrvCritical_Enter();

        if (s_sampleReady)
        {
            pSample->joyXRaw = s_joyXRaw;
            pSample->joyYRaw = s_joyYRaw;
            pSample->potRaw = s_potRaw;
            s_sampleReady = false;
            gotSample = true;
        }

        DrvCritical_Exit(primask);
    }

    return gotSample;
}

/*********************************************************************
 * @fn                - DrvAdc_GetOverrunCount
 * @brief             - จำนวนครั้งที่เกิด ADC overrun (ใช้ดีบัก)
 *//////////////////////////////////////////////////////////////////////
uint32_t DrvAdc_GetOverrunCount(void)
{
    return s_overrunCount;
}

/* Private functions ---------------------------------------------------------*/

/* ปิด stream แล้วรอจนปิดจริง พร้อมล้างธงทั้งหมด */
static void Adc_StopDmaStream(void)
{
    uint32_t guard = 0U;

    DMA2_Stream0->CR &= ~DMA_SxCR_EN;
    while (((DMA2_Stream0->CR & DMA_SxCR_EN) != 0U) && (guard < DMA_DISABLE_WAIT_LIMIT))
    {
        guard++;
    }
    DMA2->LIFCR = DMA2_STREAM0_ALL_FLAGS;
}

/* ตั้งปลายทางและจำนวนข้อมูลใหม่ แล้วเปิด stream */
static void Adc_StartDmaStream(void)
{
    DMA2_Stream0->M0AR = (uint32_t)s_dmaBuffer;
    DMA2_Stream0->NDTR = ADC_CHANNEL_COUNT;
    DMA2_Stream0->CR |= DMA_SxCR_EN;
}

/* Callback functions --------------------------------------------------------*/

void DMA2_Stream0_IRQHandler(void)
{
    uint32_t status = DMA2->LISR;

    if ((status & DMA_LISR_TEIF0) != 0U)
    {
        DMA2->LIFCR = DMA_LIFCR_CTEIF0;
    }

    if ((status & DMA_LISR_TCIF0) != 0U)
    {
        DMA2->LIFCR = DMA_LIFCR_CTCIF0;
        s_joyXRaw = s_dmaBuffer[ADC_RANK_JOY_X];
        s_joyYRaw = s_dmaBuffer[ADC_RANK_JOY_Y];
        s_potRaw = s_dmaBuffer[ADC_RANK_POT];
        s_sampleReady = true;
    }
}

void ADC_IRQHandler(void)
{
    if ((ADC1->SR & ADC_SR_OVR) != 0U)
    {
        /* overrun ทำให้ DMA หยุด: ล้างธงแล้วเริ่ม DMA ใหม่ให้ลำดับตรงกับ rank */
        ADC1->CR2 &= ~ADC_CR2_DMA;
        ADC1->SR &= ~ADC_SR_OVR;
        Adc_StopDmaStream();
        Adc_StartDmaStream();
        ADC1->CR2 |= ADC_CR2_DMA;

        if (s_overrunCount < 0xFFFFFFFFUL)
        {
            s_overrunCount++;
        }
    }
}
