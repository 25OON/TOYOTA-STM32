/*******************************************************************************
 * File Name    : app_game.c
 * Description  : ผังสถานะ
 *                   IDLE --D2--> PLAYING --ชน--> GAME_OVER --D3--> PLAYING
 *                                   |  ^
 *                                 D4|  |D4
 *                                   v  |
 *                                  PAUSED --D3--> PLAYING
 *                 ชีวิตเดียว: ชนกำแพงหรือชนตัวเองเข้า GAME_OVER ทันที
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "App/app_game.h"
#include "App/app_input.h"
#include "App/app_render.h"
#include "App/app_snake.h"
#include "Driver/drv_adc.h"
#include "Driver/drv_rand.h"

/* Private define ------------------------------------------------------------*/
#define GAME_SCORE_PER_FOOD        (10U)
#define GAME_SCORE_LIMIT           (9999U)  /* จอมีที่ให้คะแนน 4 หลัก */
#define GAME_REDRAW_TICKS          (10U)    /* วาดซ้ำทุก 250 ms แม้งูไม่ขยับ */

/* Private constants ---------------------------------------------------------*/
static const char s_textIdle[]    = "PRESS D2 TO START";
static const char s_textPlaying[] = "PLAYING";
static const char s_textPaused[]  = "PAUSED - D4 RESUME";
static const char s_textOver[]    = "GAME OVER - D3 RESTART";
static const char s_textWin[]     = "YOU WIN - D3 RESTART";
static const char s_textClassic[] = "CLASSIC";
static const char s_textTwin[]    = "TWIN";
static const char s_textWall[]    = "WALL";

/* Private variables ---------------------------------------------------------*/
static AppGameState_t s_state = APP_GAME_IDLE;
static uint16_t s_score = 0U;
static uint16_t s_best = 0U;
static uint8_t s_stepTicks = 0U;
static uint8_t s_redrawTicks = 0U;
static bool s_renderPending = true;
static bool s_foodEaten = false;
static const char *s_pNotice = NULL;

/* Private function prototypes -----------------------------------------------*/
static void Game_StartNewGame(void);
static void Game_AdvanceSnake(void);
static void Game_EnterState(AppGameState_t state);
static const char * Game_StateText(AppGameState_t state);
static const char * Game_ModeText(void);
static void Game_ToggleMode(void);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - AppGame_Init
 * @brief             - เตรียมฉากและเข้าสถานะ IDLE
 *
 * @Note              - วางงูไว้ให้เห็นบนจอตั้งแต่ยังไม่เริ่มเล่น
 *//////////////////////////////////////////////////////////////////////
void AppGame_Init(void)
{
    AppRender_Init();
    AppSnake_Reset();

    s_state = APP_GAME_IDLE;
    s_score = 0U;
    s_best = 0U;
    s_stepTicks = 0U;
    s_redrawTicks = 0U;
    s_renderPending = true;
    s_foodEaten = false;
}

/*********************************************************************
 * @fn                - AppGame_SetNotice
 * @brief             - ตั้งข้อความแจ้งเตือนบรรทัดล่างสุด
 *
 * @param[in]         - pText : ข้อความคงที่ (NULL = ไม่แสดง)
 *//////////////////////////////////////////////////////////////////////
void AppGame_SetNotice(const char * const pText)
{
    s_pNotice = pText;
    s_renderPending = true;
}

/*********************************************************************
 * @fn                - AppGame_OnTick
 * @brief             - นับหนึ่ง tick (25 ms) แล้วเดินงูเมื่อครบจังหวะ
 *
 * @Note              - จำนวน tick ต่อก้าวมาจาก pot จึงปรับความเร็วได้ระหว่างเล่น
 *//////////////////////////////////////////////////////////////////////
void AppGame_OnTick(void)
{
    if (s_redrawTicks < GAME_REDRAW_TICKS)
    {
        s_redrawTicks++;
    }
    else
    {
        s_redrawTicks = 0U;
        s_renderPending = true;
    }

    if (s_state == APP_GAME_PLAYING)
    {
        s_stepTicks++;

        if (s_stepTicks >= AppInput_GetStepTicks())
        {
            s_stepTicks = 0U;
            Game_AdvanceSnake();
        }
    }
}

/*********************************************************************
 * @fn                - AppGame_OnButtons
 * @brief             - เปลี่ยนสถานะตามปุ่มที่ถูกกด
 *
 * @param[in]         - pEvents : ปุ่มที่ถูกกดตั้งแต่รอบก่อน
 *//////////////////////////////////////////////////////////////////////
void AppGame_OnButtons(const DrvButtonEvents_t * const pEvents)
{
    if (pEvents != NULL)
    {
        if (s_state == APP_GAME_IDLE)
        {
            if (pEvents->start)
            {
                Game_StartNewGame();
            }
            else if (pEvents->mode)
            {
                Game_ToggleMode();
            }
            else
            {
                /* สถานะ IDLE สนใจเฉพาะปุ่มเริ่มเกมกับปุ่มเลือกโหมด */
            }
        }
        else if (s_state == APP_GAME_PLAYING)
        {
            if (pEvents->pause)
            {
                Game_EnterState(APP_GAME_PAUSED);
            }
            else
            {
                /* ระหว่างเล่นเปลี่ยนทิศด้วย joystick ไม่ใช้ปุ่ม */
            }
        }
        else if (s_state == APP_GAME_PAUSED)
        {
            if (pEvents->pause)
            {
                Game_EnterState(APP_GAME_PLAYING);
            }
            else if (pEvents->restart)
            {
                Game_StartNewGame();
            }
            else
            {
                /* ไม่มีปุ่มอื่นที่ใช้ตอนพักเกม */
            }
        }
        else
        {
            /* GAME_OVER และ WIN: เริ่มใหม่ได้ด้วย D3 หรือ D2 */
            if (pEvents->restart || pEvents->start)
            {
                Game_StartNewGame();
            }
            else if (pEvents->mode)
            {
                Game_ToggleMode();
            }
            else
            {
                /* รอปุ่มเริ่มใหม่ */
            }
        }
    }
}

/*********************************************************************
 * @fn                - AppGame_Service
 * @brief             - ส่งเฟรมล่าสุดถ้ามีอะไรเปลี่ยนและ UART ว่าง
 *
 * @Note              - ถ้าส่งไม่ได้ตอนนี้ ธง s_renderPending ยังค้างไว้
 *                       รอบถัดไปของ main loop จะลองส่งอีกครั้งเอง
 *//////////////////////////////////////////////////////////////////////
void AppGame_Service(void)
{
    if (s_renderPending)
    {
        AppRenderInfo_t info;
        DrvAdcSample_t raw;

        AppInput_GetRaw(&raw);
        info.joyXRaw = raw.joyXRaw;
        info.joyYRaw = raw.joyYRaw;
        info.potRaw = raw.potRaw;
        info.showAdc = (s_state == APP_GAME_IDLE);
        info.score = s_score;
        info.best = s_best;
        info.length = AppSnake_GetLength();
        info.speedLevel = AppInput_GetSpeedLevel();
        info.pStateText = Game_StateText(s_state);
        info.pModeText = Game_ModeText();
        info.pNoticeText = s_pNotice;

        if (AppRender_SendFrame(&info))
        {
            s_renderPending = false;
        }
    }
}

/*********************************************************************
 * @fn                - AppGame_GetState
 * @brief             - สถานะปัจจุบันของเกม
 *//////////////////////////////////////////////////////////////////////
AppGameState_t AppGame_GetState(void)
{
    return s_state;
}

/*********************************************************************
 * @fn                - AppGame_TakeFoodEaten
 * @brief             - รับเหตุการณ์ "กินอาหาร" แล้วล้างทิ้ง
 *
 * @return            - true ถ้ามีการกินตั้งแต่เรียกครั้งก่อน
 *//////////////////////////////////////////////////////////////////////
bool AppGame_TakeFoodEaten(void)
{
    bool eaten = s_foodEaten;

    s_foodEaten = false;

    return eaten;
}

/* Private functions ---------------------------------------------------------*/

/* เริ่มเกมใหม่: สุ่มใหม่จากค่า ADC ล่าสุด ตั้งงูและคะแนนกลับไปที่จุดเริ่ม */
static void Game_StartNewGame(void)
{
    DrvRand_Seed(AppInput_GetSeedSource());
    AppInput_Reset();
    AppSnake_Reset();

    s_score = 0U;
    s_stepTicks = 0U;
    s_foodEaten = false;

    Game_EnterState(APP_GAME_PLAYING);
}

/* เดินงูหนึ่งก้าวแล้วจัดการผลที่ได้ */
static void Game_AdvanceSnake(void)
{
    AppDir_t dir;
    AppStep_t step;

    if (AppInput_TakeDirection(&dir))
    {
        AppSnake_RequestDir(dir);
    }

    step = AppSnake_Step();

    switch (step)
    {
        case APP_STEP_ATE:
            if (s_score <= (uint16_t)(GAME_SCORE_LIMIT - GAME_SCORE_PER_FOOD))
            {
                s_score = (uint16_t)(s_score + GAME_SCORE_PER_FOOD);
            }
            s_foodEaten = true;
            break;

        case APP_STEP_DIED:
            Game_EnterState(APP_GAME_OVER);
            break;

        case APP_STEP_WIN:
            Game_EnterState(APP_GAME_WIN);
            break;

        case APP_STEP_MOVED:
        default:
            /* เดินปกติ ไม่มีอะไรต้องทำนอกจากวาดใหม่ */
            break;
    }

    s_renderPending = true;
}

/* เปลี่ยนสถานะ พร้อมอัปเดตคะแนนสูงสุดเมื่อจบเกม */
static void Game_EnterState(AppGameState_t state)
{
    if ((state == APP_GAME_OVER) || (state == APP_GAME_WIN))
    {
        if (s_score > s_best)
        {
            s_best = s_score;
        }
    }

    s_state = state;
    s_renderPending = true;
}

/*  สลับโหมด CLASSIC <-> TWIN
 *  เรียกได้เฉพาะตอน IDLE / GAME_OVER / WIN เท่านั้น
 *  ห้ามเปลี่ยนกลางเกม เพราะกติกาจะเปลี่ยนทั้งที่งูกำลังวิ่งอยู่
 */
static void Game_ToggleMode(void)
{
    AppSnakeMode_t next;

    switch (AppSnake_GetMode())
    {
        case APP_SNAKE_MODE_CLASSIC:
            next = APP_SNAKE_MODE_TWIN;
            break;
        case APP_SNAKE_MODE_TWIN:
            next = APP_SNAKE_MODE_WALL;
            break;
        case APP_SNAKE_MODE_WALL:
        default:
            next = APP_SNAKE_MODE_CLASSIC;
            break;
    }

    AppSnake_SetMode(next);
    s_renderPending = true;
}

/* ชื่อโหมดที่แสดงมุมขวาบน */
static const char * Game_ModeText(void)
{
    const char *pText;

    switch (AppSnake_GetMode())
    {
        case APP_SNAKE_MODE_TWIN:
            pText = s_textTwin;
            break;
        case APP_SNAKE_MODE_WALL:
            pText = s_textWall;
            break;
        case APP_SNAKE_MODE_CLASSIC:
        default:
            pText = s_textClassic;
            break;
    }

    return pText;
}

/* ข้อความสถานะที่แสดงบนหัวจอ */
static const char * Game_StateText(AppGameState_t state)
{
    const char *pText;

    switch (state)
    {
        case APP_GAME_PLAYING:
            pText = s_textPlaying;
            break;
        case APP_GAME_PAUSED:
            pText = s_textPaused;
            break;
        case APP_GAME_OVER:
            pText = s_textOver;
            break;
        case APP_GAME_WIN:
            pText = s_textWin;
            break;
        case APP_GAME_IDLE:
        default:
            pText = s_textIdle;
            break;
    }

    return pText;
}
