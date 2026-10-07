/*******************************************************************************
 * File Name    : app_main.h
 * Description  : จุดเข้าใช้งานของชั้น Application
 * Date         : 2026-09-15
 ******************************************************************************/
#ifndef APP_MAIN_H
#define APP_MAIN_H

/* เรียกครั้งเดียวก่อนเข้า main loop */
void App_Init(void);

/* เรียกซ้ำใน main loop (ไม่บล็อก) */
void App_Run(void);

#endif /* APP_MAIN_H */
