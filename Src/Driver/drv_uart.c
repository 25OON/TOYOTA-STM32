/*******************************************************************************
 * File Name    : drv_uart.c
 * Description  : USART2 TX ผ่าน DMA1 Stream6 Channel4
 *                 ส่งข้อความโดยไม่บล็อก: คัดลอกลง buffer ภายใน แล้วให้ DMA
 *                 ป้อนทีละไบต์เอง จบแล้ว transfer-complete interrupt ปลด busy
 * Date         : 2026-09-15
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "Driver/drv_uart.h"
#include "Driver/drv_board.h"
#include "Driver/drv_gpio.h"
#include "Driver/drv_isr.h"
#include "Driver/drv_mcu.h"

/* Private define ------------------------------------------------------------*/
#define UART_DMA_CHANNEL           (4U)     /* USART2_TX = DMA1 Stream6 Channel4 */
#define UART_IRQ_PRIORITY          (3U)
#define DMA_DISABLE_WAIT_LIMIT     (10000UL)
#define DMA1_STREAM6_ALL_FLAGS     (DMA_HIFCR_CFEIF6 | DMA_HIFCR_CDMEIF6 | \
                                    DMA_HIFCR_CTEIF6 | DMA_HIFCR_CHTIF6 | \
                                    DMA_HIFCR_CTCIF6)

/* Private variables ---------------------------------------------------------*/
static uint8_t s_txBuffer[DRV_UART_TX_BUFFER_SIZE];
static volatile bool s_txBusy = false;

/* Private function prototypes -----------------------------------------------*/
static void Uart_StopDmaStream(void);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - DrvUart_Init
 * @brief             - ตั้งขา PA2, USART2 115200 8N1 และ DMA1 Stream6
 *//////////////////////////////////////////////////////////////////////
void DrvUart_Init(void)
{
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_DMA1EN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    DrvGpio_SetAltFunction(DRV_UART_TX_PORT, DRV_UART_TX_PIN, DRV_UART_TX_AF);
    DrvGpio_SetMode(DRV_UART_TX_PORT, DRV_UART_TX_PIN, DRV_GPIO_MODE_ALTFN);

    USART2->CR1 = 0U;
    USART2->BRR = DRV_UART_BRR_115200;
    USART2->CR3 = USART_CR3_DMAT;
    USART2->CR1 = USART_CR1_TE | USART_CR1_UE;

    Uart_StopDmaStream();
    DMA1_Stream6->PAR = (uint32_t)&USART2->DR;
    DMA1_Stream6->CR = ((uint32_t)UART_DMA_CHANNEL << DMA_SxCR_CHSEL_Pos) |
                       DMA_SxCR_DIR_0 |        /* memory -> peripheral (01) */
                       DMA_SxCR_MINC |
                       DMA_SxCR_TCIE |
                       DMA_SxCR_TEIE;

    NVIC_SetPriority(DMA1_Stream6_IRQn, UART_IRQ_PRIORITY);
    NVIC_EnableIRQ(DMA1_Stream6_IRQn);

    s_txBusy = false;
}

/*********************************************************************
 * @fn                - DrvUart_IsBusy
 * @brief             - บอกว่ากำลังส่งข้อความก่อนหน้าอยู่หรือไม่
 *//////////////////////////////////////////////////////////////////////
bool DrvUart_IsBusy(void)
{
    return s_txBusy;
}

/*********************************************************************
 * @fn                - DrvUart_Send
 * @brief             - เริ่มส่งข้อมูลแบบไม่บล็อก
 *
 * @param[in]         - pData  : ข้อมูลที่จะส่ง
 * @param[in]         - length : จำนวนไบต์ (1 - DRV_UART_TX_BUFFER_SIZE)
 *
 * @return            - true ถ้าเริ่มส่งได้, false ถ้ากำลังส่งอยู่หรือพารามิเตอร์ผิด
 *//////////////////////////////////////////////////////////////////////
bool DrvUart_Send(const uint8_t * const pData, uint16_t length)
{
    bool started = false;
    bool busy = s_txBusy;

    if ((!busy) && (pData != NULL) && (length > 0U) && (length <= DRV_UART_TX_BUFFER_SIZE))
    {
        uint16_t index;

        for (index = 0U; index < length; index++)
        {
            s_txBuffer[index] = pData[index];
        }

        s_txBusy = true;

        Uart_StopDmaStream();
        DMA1_Stream6->M0AR = (uint32_t)s_txBuffer;
        DMA1_Stream6->NDTR = length;
        USART2->SR &= ~USART_SR_TC;
        DMA1_Stream6->CR |= DMA_SxCR_EN;

        started = true;
    }

    return started;
}

/* Private functions ---------------------------------------------------------*/

/* ปิด stream แล้วรอจนปิดจริง พร้อมล้างธงทั้งหมด */
static void Uart_StopDmaStream(void)
{
    uint32_t guard = 0U;

    DMA1_Stream6->CR &= ~DMA_SxCR_EN;
    while (((DMA1_Stream6->CR & DMA_SxCR_EN) != 0U) && (guard < DMA_DISABLE_WAIT_LIMIT))
    {
        guard++;
    }
    DMA1->HIFCR = DMA1_STREAM6_ALL_FLAGS;
}

/* Callback functions --------------------------------------------------------*/

void DMA1_Stream6_IRQHandler(void)
{
    uint32_t status = DMA1->HISR;

    if ((status & DMA_HISR_TEIF6) != 0U)
    {
        DMA1->HIFCR = DMA_HIFCR_CTEIF6;
        DMA1_Stream6->CR &= ~DMA_SxCR_EN;
        s_txBusy = false;
    }

    if ((status & DMA_HISR_TCIF6) != 0U)
    {
        DMA1->HIFCR = DMA_HIFCR_CTCIF6;
        s_txBusy = false;
    }
}
