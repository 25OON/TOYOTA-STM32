/*******************************************************************************
 * File Name    : app_input.c
 * Description  : joystick ของชุด 37-in-1 ให้แรงดันกลางราว VDD/2 (ADC ~2048)
 *                 ถือว่า "ไม่สั่ง" เมื่อยังอยู่ในวง deadzone +-700 นับ
 *                 ออกนอก deadzone แล้วเอาแกนที่เบนมากกว่าเป็นทิศที่สั่ง
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "App/app_input.h"

/* Private define ------------------------------------------------------------*/
#define INPUT_CENTER_RAW           (2048)   /* ค่ากลางของ joystick (12 บิต) */
#define INPUT_DEADZONE             (700)    /* ต้องเบนเกินเท่านี้จึงนับเป็นทิศ */
#define INPUT_SPEED_SCALE          (4096U)  /* ตัวหารของสูตรแบ่ง pot เป็นระดับ */

/* ---- ผังแกนของ joystick: วัดจากบอร์ดจริงเมื่อ 2026-10-05 ----
 * อ่านค่าดิบตอนดันก้างค้างไว้ทีละทิศ ได้ผลดังนี้
 *     ดันขึ้น  -> PB0 (joyXRaw) ลงไป 0   · PC0 นิ่งที่ ~2000
 *     ดันขวา  -> PC0 (joyYRaw) ลงไป 0   · PB0 นิ่งที่ ~1990
 * แปลว่าโมดูลวางหันด้านที่ทำให้ PB0 = แกนตั้ง และ PC0 = แกนนอน
 * ซึ่งสลับกับที่ตั้งชื่อไว้ตอนแรก และขั้วก็กลับด้วย (ดันขวาแล้วค่า "ลด")
 *
 * แก้ด้วยการตั้งค่าสามตัวนี้ ไม่ต้องถอดสาย:
 *   SWAP  = 1U  สลับว่าช่องไหนเป็นแกนนอน/แกนตั้ง
 *   INVERT_X = 1U  กลับขั้วแกนนอน (ดันขวาแล้วค่าลด)
 *   INVERT_Y = 0U  แกนตั้งขั้วถูกอยู่แล้ว (ดันขึ้นแล้วค่าลด = ขึ้น)
 * ถ้าหมุนโมดูลใหม่แล้วทิศเพี้ยน ให้กลับมาวัดด้วยบรรทัด PB0/PC0 บนจอ IDLE
 * แล้วปรับสามตัวนี้ ไม่ต้องแก้ที่อื่น */
#define INPUT_SWAP_AXES            (1U)
#define INPUT_INVERT_X             (1U)
#define INPUT_INVERT_Y             (0U)

/* Private constants ---------------------------------------------------------*/
/* จำนวน tick 25 ms ต่อหนึ่งก้าว: ระดับ 1 = 250 ms ... ระดับ 5 = 100 ms
 * (งูเดิน 4-10 ก้าวต่อวินาที ครอบช่วงที่เล่นสนุกทั้งหมด) */
static const uint8_t s_stepTicks[APP_INPUT_SPEED_LEVELS] = { 10U, 8U, 6U, 5U, 4U };

/* Private variables ---------------------------------------------------------*/
static AppDir_t s_pendingDir = APP_DIR_RIGHT;
static bool s_hasPendingDir = false;
static uint8_t s_speedLevel = 3U;
static uint32_t s_seedSource = 0U;
static DrvAdcSample_t s_lastSample = { 0U, 0U, 0U };

/* Private function prototypes -----------------------------------------------*/
static void Input_UpdateDirection(uint16_t rawX, uint16_t rawY);
static void Input_UpdateSpeed(uint16_t rawPot);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - AppInput_Reset
 * @brief             - ทิ้งทิศที่ค้างอยู่ (กันงูเลี้ยวตามคำสั่งเก่าของเกมก่อน)
 *//////////////////////////////////////////////////////////////////////
void AppInput_Reset(void)
{
    s_hasPendingDir = false;
}

/*********************************************************************
 * @fn                - AppInput_Update
 * @brief             - แปลงค่า ADC ชุดใหม่เป็นทิศและระดับความเร็ว
 *
 * @param[in]         - pSample : ค่าดิบ 3 ช่องจาก DMA
 *//////////////////////////////////////////////////////////////////////
void AppInput_Update(const DrvAdcSample_t * const pSample)
{
    if (pSample != NULL)
    {
        s_lastSample = *pSample;
        s_seedSource = ((uint32_t)pSample->joyXRaw << 20U) ^
                       ((uint32_t)pSample->joyYRaw << 10U) ^
                       (uint32_t)pSample->potRaw;

#if (INPUT_SWAP_AXES == 1U)
        /* joyYRaw (PC0) คือแกนนอน · joyXRaw (PB0) คือแกนตั้ง */
        Input_UpdateDirection(pSample->joyYRaw, pSample->joyXRaw);
#else
        Input_UpdateDirection(pSample->joyXRaw, pSample->joyYRaw);
#endif
        Input_UpdateSpeed(pSample->potRaw);
    }
}

/*********************************************************************
 * @fn                - AppInput_TakeDirection
 * @brief             - รับทิศล่าสุดที่ผู้เล่นสั่งแล้วล้างทิ้ง
 *
 * @param[out]        - pDir : ทิศที่สั่ง (เขียนเมื่อคืน true เท่านั้น)
 *
 * @return            - true ถ้ามีทิศใหม่ตั้งแต่เรียกครั้งก่อน
 *//////////////////////////////////////////////////////////////////////
bool AppInput_TakeDirection(AppDir_t * const pDir)
{
    bool hasDir = false;

    if ((pDir != NULL) && s_hasPendingDir)
    {
        *pDir = s_pendingDir;
        s_hasPendingDir = false;
        hasDir = true;
    }

    return hasDir;
}

/*********************************************************************
 * @fn                - AppInput_GetSpeedLevel
 * @brief             - ระดับความเร็วปัจจุบันจาก pot (1 - 5)
 *//////////////////////////////////////////////////////////////////////
uint8_t AppInput_GetSpeedLevel(void)
{
    return s_speedLevel;
}

/*********************************************************************
 * @fn                - AppInput_GetStepTicks
 * @brief             - จำนวน tick 25 ms ต่อหนึ่งก้าวของงู
 *//////////////////////////////////////////////////////////////////////
uint8_t AppInput_GetStepTicks(void)
{
    uint8_t level = s_speedLevel;

    if (level < 1U)
    {
        level = 1U;
    }
    if (level > APP_INPUT_SPEED_LEVELS)
    {
        level = (uint8_t)APP_INPUT_SPEED_LEVELS;
    }

    return s_stepTicks[level - 1U];
}

/*********************************************************************
 * @fn                - AppInput_GetSeedSource
 * @brief             - ค่าดิบของ ADC ล่าสุด ใช้เป็น seed ของตัวสุ่ม
 *//////////////////////////////////////////////////////////////////////
uint32_t AppInput_GetSeedSource(void)
{
    return s_seedSource;
}

/*********************************************************************
 * @fn                - AppInput_GetRaw
 * @brief             - ค่าดิบ 3 ช่องล่าสุดที่ได้จาก DMA
 *
 * @param[out]        - pSample : ที่เก็บค่าดิบ
 *
 * @Note              - ใช้โชว์บนจอตอน IDLE เพื่อตรวจว่าสาย joystick เข้าถูกขา
 *//////////////////////////////////////////////////////////////////////
void AppInput_GetRaw(DrvAdcSample_t * const pSample)
{
    if (pSample != NULL)
    {
        *pSample = s_lastSample;
    }
}

/* Private functions ---------------------------------------------------------*/

/* เอาแกนที่เบนจากกลางมากกว่าเป็นทิศที่สั่ง ถ้าทั้งสองแกนอยู่ใน deadzone ไม่สั่งอะไร */
static void Input_UpdateDirection(uint16_t rawX, uint16_t rawY)
{
    int16_t offsetX = (int16_t)((int16_t)rawX - (int16_t)INPUT_CENTER_RAW);
    int16_t offsetY = (int16_t)((int16_t)rawY - (int16_t)INPUT_CENTER_RAW);
    int16_t absX;
    int16_t absY;

#if (INPUT_INVERT_X == 1U)
    offsetX = (int16_t)(-offsetX);
#endif
#if (INPUT_INVERT_Y == 1U)
    offsetY = (int16_t)(-offsetY);
#endif

    absX = (offsetX < 0) ? (int16_t)(-offsetX) : offsetX;
    absY = (offsetY < 0) ? (int16_t)(-offsetY) : offsetY;

    if ((absX >= absY) && (absX > INPUT_DEADZONE))
    {
        s_pendingDir = (offsetX > 0) ? APP_DIR_RIGHT : APP_DIR_LEFT;
        s_hasPendingDir = true;
    }
    else if (absY > INPUT_DEADZONE)
    {
        /* ค่า ADC มากขึ้น = ก้านถูกดันไปด้านที่แรงดันสูง ให้ตรงกับ "ลง" บนจอ
         * ถ้าเล่นแล้วรู้สึกกลับทิศ ให้ตั้ง INPUT_INVERT_Y เป็น 1U */
        s_pendingDir = (offsetY > 0) ? APP_DIR_DOWN : APP_DIR_UP;
        s_hasPendingDir = true;
    }
    else
    {
        /* อยู่ใน deadzone: คงทิศเดิมของงูไว้ ไม่ต้องสั่งอะไร */
    }
}

/* แบ่งช่วง pot 0-4095 เป็น 5 ระดับเท่า ๆ กัน */
static void Input_UpdateSpeed(uint16_t rawPot)
{
    uint32_t scaled = ((uint32_t)rawPot * APP_INPUT_SPEED_LEVELS) / INPUT_SPEED_SCALE;

    s_speedLevel = (uint8_t)(scaled + 1U);
}
