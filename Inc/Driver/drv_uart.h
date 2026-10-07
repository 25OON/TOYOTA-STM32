/*******************************************************************************
 * File Name    : drv_uart.h
 * Description  : USART2 ส่งข้อมูลอย่างเดียวผ่าน DMA1 Stream6 (ไม่บล็อก, ไม่ polling)
 *                 115200 8N1 ออกทาง ST-Link Virtual COM Port
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef DRV_UART_H
#define DRV_UART_H

#include <stdbool.h>
#include <stdint.h>

/* Public define -------------------------------------------------------------*/
/* ต้องใหญ่พอใส่ทั้งเฟรมของเกม (817 ไบต์) ในการส่งครั้งเดียว */
#define DRV_UART_TX_BUFFER_SIZE    (1024U)

/* Public function prototypes ------------------------------------------------*/
void DrvUart_Init(void);
bool DrvUart_IsBusy(void);
bool DrvUart_Send(const uint8_t * const pData, uint16_t length);

#endif /* DRV_UART_H */
