/*******************************************************************************
 * File Name    : app_main.c
 * Description  : ตัวเชื่อมทุกอย่างของเกม Snake
 *                 - เปิด driver ทุกตัวตามลำดับที่ปลอดภัย
 *                 - ดึง tick 25 ms จาก TIM2 มาป้อนเกม
 *                 - ส่งค่า ADC ให้ชั้นแปลงคำสั่งผู้เล่น และปุ่มให้ state machine
 *                 - คุม LED 4 ดวงเป็นสัญญาณสถานะ และเตะ watchdog
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "App/app_main.h"
#include "App/app_game.h"
#include "App/app_input.h"
#include "Driver/drv_adc.h"
#include "Driver/drv_input.h"
#include "Driver/drv_led.h"
#include "Driver/drv_seg.h"
#include "Driver/drv_system.h"
#include "Driver/drv_tick.h"
#include "Driver/drv_uart.h"
#include "Driver/drv_wdg.h"

/* Private define ------------------------------------------------------------*/
#define APP_BLINK_SLOW_TICKS         (20U)   /* 20 x 25 ms = สลับทุก 500 ms */
#define APP_BLINK_FAST_TICKS         (4U)    /* สลับทุก 100 ms */
#define APP_FOOD_FLASH_TICKS         (6U)    /* ไฟฟ้าแวบ 150 ms ตอนกินอาหาร */

/* Private constants ---------------------------------------------------------*/
/* ESC[2J ล้างจอทั้งหมด, ESC[H กลับมุมบนซ้าย, ESC[?25l ซ่อนเคอร์เซอร์
 * ส่งครั้งเดียวตอนบูต หลังจากนี้ทุกเฟรมใช้แค่ ESC[H ภาพจึงไม่กะพริบ */
static const char s_bootSequence[] = "\x1B[2J\x1B[H\x1B[?25l";
static const char s_noticeWatchdog[] = "NOTE: last reset came from IWDG";

/* Private variables ---------------------------------------------------------*/
static bool s_bootPending = false;
static uint8_t s_blinkSlowTicks = 0U;
static bool s_blinkSlowOn = false;
static uint8_t s_blinkFastTicks = 0U;
static bool s_blinkFastOn = false;
static uint8_t s_foodFlashTicks = 0U;
static uint8_t s_lastSpeedShown = 0U;   /* 0 = ยังไม่เคยเขียนลง 7-segment */

/* Private function prototypes -----------------------------------------------*/
static void App_SendBootSequence(void);
static void App_FeedAdcToInput(void);
static void App_HandleTicks(void);
static void App_HandleButtons(void);
static void App_UpdateLeds(void);
static void App_UpdateSevenSegment(void);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - App_Init
 * @brief             - เปิด driver ทุกตัว เตรียมเกม แล้วเริ่มฐานเวลา
 *
 * @Note              - ลำดับสำคัญ 3 จุด
 *                       1) DrvWdg_Init() ต้องอ่าน RCC->CSR ก่อนใครไปล้างธง
 *                       2) DrvTick_Start() เป็นอย่างสุดท้าย เพราะ TRGO
 *                          จะสั่ง ADC ทันทีที่ TIM2 เริ่มนับ
 *                       3) AppGame_Init() ต้องมาก่อน เพื่อให้มีฉากพร้อมวาด
 *//////////////////////////////////////////////////////////////////////
void App_Init(void)
{
    DrvSystem_Init();
    DrvWdg_Init();
    DrvLed_Init();
    DrvSeg_Init();
    DrvInput_Init();
    DrvUart_Init();
    DrvAdc_Init();
    DrvTick_Init();

    AppGame_Init();

    if (DrvWdg_WasWatchdogReset())
    {
        AppGame_SetNotice(s_noticeWatchdog);
    }
    else
    {
        AppGame_SetNotice(NULL);
    }

    s_bootPending = true;
    s_blinkSlowTicks = 0U;
    s_blinkSlowOn = false;
    s_blinkFastTicks = 0U;
    s_blinkFastOn = false;
    s_foodFlashTicks = 0U;
    s_lastSpeedShown = 0U;

    DrvTick_Start();
}

/*********************************************************************
 * @fn                - App_Run
 * @brief             - งาน 1 รอบของ main loop (ไม่มีการรอหรือ delay)
 *
 * @Note              - ทุกอย่างในรอบนี้ต้องจบเร็วกว่าคาบ watchdog 500 ms
 *                       ถ้าอยากทดสอบ watchdog ให้ใส่ for(;;) ค้างไว้ตรงนี้
 *                       บอร์ดต้องรีเซ็ตเองแล้วขึ้นข้อความ NOTE ที่บรรทัดล่าง
 *//////////////////////////////////////////////////////////////////////
void App_Run(void)
{
    App_FeedAdcToInput();
    App_HandleTicks();
    App_HandleButtons();
    App_UpdateLeds();
    App_UpdateSevenSegment();
    App_SendBootSequence();
    AppGame_Service();

    DrvWdg_Reload();
}

/* Private functions ---------------------------------------------------------*/

/* ส่งรหัสล้างจอ/ซ่อนเคอร์เซอร์ครั้งเดียว ก่อนเฟรมแรกของเกม */
static void App_SendBootSequence(void)
{
    if (s_bootPending)
    {
        uint16_t length = (uint16_t)(sizeof(s_bootSequence) - 1U);

        if (DrvUart_Send((const uint8_t *)s_bootSequence, length))
        {
            s_bootPending = false;
        }
    }
}

/* ส่งค่า ADC ชุดใหม่ (ถ้ามี) ให้ชั้นแปลงคำสั่งผู้เล่น */
static void App_FeedAdcToInput(void)
{
    DrvAdcSample_t raw;

    if (DrvAdc_GetSample(&raw))
    {
        AppInput_Update(&raw);
    }
}

/* แจก tick ของ TIM2 ให้เกมและตัวนับไฟกระพริบ */
static void App_HandleTicks(void)
{
    uint32_t ticks = DrvTick_TakePending();

    while (ticks > 0U)
    {
        ticks--;

        s_blinkSlowTicks++;
        if (s_blinkSlowTicks >= APP_BLINK_SLOW_TICKS)
        {
            s_blinkSlowTicks = 0U;
            s_blinkSlowOn = !s_blinkSlowOn;
        }

        s_blinkFastTicks++;
        if (s_blinkFastTicks >= APP_BLINK_FAST_TICKS)
        {
            s_blinkFastTicks = 0U;
            s_blinkFastOn = !s_blinkFastOn;
        }

        if (s_foodFlashTicks > 0U)
        {
            s_foodFlashTicks--;
        }

        AppGame_OnTick();
    }
}

/* อ่านเหตุการณ์ปุ่มจาก EXTI แล้วส่งให้ state machine */
static void App_HandleButtons(void)
{
    DrvButtonEvents_t events;

    DrvInput_TakeButtonEvents(&events);

    if (events.start || events.restart || events.pause || events.mode)
    {
        AppGame_OnButtons(&events);
    }
}

/* LED 4 ดวงเป็นสัญญาณสถานะ ไม่ใช่จอแสดงผล
 *   IDLE      เขียวกระพริบช้า        PLAYING   เขียวนิ่ง
 *   PAUSED    เหลืองนิ่ง             GAME OVER แดงนิ่ง
 *   WIN       เขียว+เหลืองกระพริบ    กินอาหาร  ฟ้าแวบ 150 ms (ซ้อนทับได้) */
static void App_UpdateLeds(void)
{
    AppGameState_t state = AppGame_GetState();
    bool greenOn = false;
    bool yellowOn = false;
    bool redOn = false;

    if (AppGame_TakeFoodEaten())
    {
        s_foodFlashTicks = (uint8_t)APP_FOOD_FLASH_TICKS;
    }

    switch (state)
    {
        case APP_GAME_PLAYING:
            greenOn = true;
            break;
        case APP_GAME_PAUSED:
            yellowOn = true;
            break;
        case APP_GAME_OVER:
            redOn = true;
            break;
        case APP_GAME_WIN:
            greenOn = s_blinkFastOn;
            yellowOn = s_blinkFastOn;
            break;
        case APP_GAME_IDLE:
        default:
            greenOn = s_blinkSlowOn;
            break;
    }

    DrvLed_Set(DRV_LED_GREEN, greenOn);
    DrvLed_Set(DRV_LED_YELLOW, yellowOn);
    DrvLed_Set(DRV_LED_RED, redOn);
    DrvLed_Set(DRV_LED_BLUE, (s_foodFlashTicks > 0U));
}

/* 7-segment โชว์ระดับความเร็วจาก pot (1-5)
 * เขียนเฉพาะตอนค่าเปลี่ยน เพราะ decoder ค้างค่าเดิมไว้เองอยู่แล้ว
 * ไม่ต้องรีเฟรชซ้ำทุกรอบของ main loop */
static void App_UpdateSevenSegment(void)
{
    uint8_t level = AppInput_GetSpeedLevel();

    if (level != s_lastSpeedShown)
    {
        s_lastSpeedShown = level;
        DrvSeg_ShowDigit(level);
    }
}
