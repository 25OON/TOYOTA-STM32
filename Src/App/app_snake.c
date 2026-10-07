/*******************************************************************************
 * File Name    : app_snake.c
 * Description  : ตัวงูเก็บข้อมูลสองชุดคู่กัน
 *                 - s_body      : ring buffer บอก "ลำดับ" ของช่อง (หางไหนหายก่อน)
 *                 - s_occupancy : ตารางบอก "ช่องนั้นมีอะไร" ทำให้เช็กชนตัวเอง O(1)
 *                 เปลืองแรม 512 ไบต์ แลกกับไม่ต้องไล่ดูลำตัวที่ยาวได้ถึง 512 ช่อง
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "App/app_snake.h"
#include "Driver/drv_rand.h"

/* Private define ------------------------------------------------------------*/
#define SNAKE_START_X              (5U)     /* หัวงูตอนเริ่ม (ต้อง >= ความยาวเริ่มต้น) */
#define SNAKE_START_Y              (8U)
/* กำแพงห้ามโผล่ใกล้หัวเกินไป ไม่งั้นผู้เล่นหลบไม่ทันไม่ว่าความเร็วไหน
 * ระยะ 3 ช่องแบบแมนฮัตตันให้เวลาอย่างน้อย 3 ก้าวในการเลี้ยว */
#define SNAKE_WALL_MIN_GAP         (3)

/* Private struct ------------------------------------------------------------*/
typedef struct
{
    uint8_t x;
    uint8_t y;
} SnakeCell_t;

/* Private variables ---------------------------------------------------------*/
static SnakeCell_t s_body[APP_SNAKE_CELLS];        /* 1024 ไบต์ */
static uint8_t     s_occupancy[APP_SNAKE_CELLS];   /* 512 ไบต์  */
static uint16_t    s_headIndex = 0U;
static uint16_t    s_length = 0U;
static AppDir_t    s_dir = APP_DIR_RIGHT;
static AppDir_t    s_pendingDir = APP_DIR_RIGHT;
static AppSnakeMode_t s_mode = APP_SNAKE_MODE_CLASSIC;
static uint16_t    s_wallCount = 0U;

/* Private function prototypes -----------------------------------------------*/
static uint16_t Snake_CellIndex(uint8_t x, uint8_t y);
static uint16_t Snake_TailBodyIndex(void);
static bool Snake_IsOpposite(AppDir_t a, AppDir_t b);
static bool Snake_PlaceFood(void);
static void Snake_SwapEnds(void);
static bool Snake_IsWallSpot(uint16_t index);
static bool Snake_PlaceWall(void);
static AppDir_t Snake_DirBetween(uint16_t fromBody, uint16_t toBody);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - AppSnake_Reset
 * @brief             - เริ่มเกมใหม่: ล้างตาราง วางงูยาว 3 ช่อง และอาหารเม็ดแรก
 *
 * @Note              - ลำตัวเรียงจากหางไปหัวในทิศทางที่วิ่ง (ไปทางขวา)
 *//////////////////////////////////////////////////////////////////////
void AppSnake_Reset(void)
{
    uint16_t index;

    for (index = 0U; index < APP_SNAKE_CELLS; index++)
    {
        s_occupancy[index] = (uint8_t)APP_CELL_EMPTY;
    }

    s_length = APP_SNAKE_START_LENGTH;
    s_wallCount = 0U;
    s_dir = APP_DIR_RIGHT;
    s_pendingDir = APP_DIR_RIGHT;

    for (index = 0U; index < APP_SNAKE_START_LENGTH; index++)
    {
        uint8_t x = (uint8_t)(SNAKE_START_X - (APP_SNAKE_START_LENGTH - 1U) + index);

        s_body[index].x = x;
        s_body[index].y = (uint8_t)SNAKE_START_Y;
        s_occupancy[Snake_CellIndex(x, (uint8_t)SNAKE_START_Y)] = (uint8_t)APP_CELL_BODY;
    }

    s_headIndex = (uint16_t)(APP_SNAKE_START_LENGTH - 1U);

    (void)Snake_PlaceFood();
}

/*********************************************************************
 * @fn                - AppSnake_SetMode
 * @brief             - เลือกโหมดการเล่น
 *
 * @param[in]         - mode : CLASSIC หรือ TWIN
 *
 * @Note              - ไม่แตะตัวงูที่กำลังเล่นอยู่ มีผลตอนกินเม็ดถัดไป
 *//////////////////////////////////////////////////////////////////////
void AppSnake_SetMode(AppSnakeMode_t mode)
{
    s_mode = mode;
}

/*********************************************************************
 * @fn                - AppSnake_GetMode
 * @brief             - โหมดที่ใช้อยู่
 *//////////////////////////////////////////////////////////////////////
AppSnakeMode_t AppSnake_GetMode(void)
{
    return s_mode;
}

/*********************************************************************
 * @fn                - AppSnake_RequestDir
 * @brief             - จดทิศที่ผู้เล่นขอไว้ ใช้ตอนเดินก้าวถัดไป
 *
 * @param[in]         - dir : ทิศที่ขอ
 *//////////////////////////////////////////////////////////////////////
void AppSnake_RequestDir(AppDir_t dir)
{
    s_pendingDir = dir;
}

/*********************************************************************
 * @fn                - AppSnake_Step
 * @brief             - เดินหนึ่งก้าวตามลำดับใน PROGRESS.md
 *
 * @return            - ผลของก้าวนี้ (เดิน / กิน / ตาย / เต็มกระดาน)
 *
 * @Note              - กับดักสำคัญ: ช่องหางที่กำลังจะหายในก้าวนี้ต้องถือว่าว่าง
 *                       ไม่อย่างนั้นงูจะ "ชนตัวเอง" ทั้งที่ไม่ควรตาย
 *//////////////////////////////////////////////////////////////////////
AppStep_t AppSnake_Step(void)
{
    AppStep_t result = APP_STEP_MOVED;
    int16_t nextX = (int16_t)s_body[s_headIndex].x;
    int16_t nextY = (int16_t)s_body[s_headIndex].y;

    /* 1. รับทิศที่ขอ ถ้าไม่ใช่ทิศตรงข้ามกับที่วิ่งอยู่ */
    if (!Snake_IsOpposite(s_pendingDir, s_dir))
    {
        s_dir = s_pendingDir;
    }

    /* 2. ช่องถัดไปของหัว */
    switch (s_dir)
    {
        case APP_DIR_UP:
            nextY--;
            break;
        case APP_DIR_DOWN:
            nextY++;
            break;
        case APP_DIR_LEFT:
            nextX--;
            break;
        case APP_DIR_RIGHT:
        default:
            nextX++;
            break;
    }

    /* 3. ออกนอกกรอบ = ตาย */
    if ((nextX < 0) || (nextX >= (int16_t)APP_SNAKE_GRID_W) ||
        (nextY < 0) || (nextY >= (int16_t)APP_SNAKE_GRID_H))
    {
        result = APP_STEP_DIED;
    }
    else
    {
        uint8_t  headX = (uint8_t)nextX;
        uint8_t  headY = (uint8_t)nextY;
        uint16_t nextCell = Snake_CellIndex(headX, headY);
        uint16_t tailBody = Snake_TailBodyIndex();
        uint16_t tailCell = Snake_CellIndex(s_body[tailBody].x, s_body[tailBody].y);
        bool willGrow = (s_occupancy[nextCell] == (uint8_t)APP_CELL_FOOD);
        bool tailLeaves = ((!willGrow) && (nextCell == tailCell));
        bool hitSelf = ((s_occupancy[nextCell] == (uint8_t)APP_CELL_BODY) && (!tailLeaves));
        bool hitWall = (s_occupancy[nextCell] == (uint8_t)APP_CELL_WALL);

        /* 4. ชนลำตัวตัวเอง หรือชนก้อนกำแพง = ตาย
         *    (ยกเว้นช่องหางที่กำลังจะหายในก้าวนี้) */
        if (hitSelf || hitWall)
        {
            result = APP_STEP_DIED;
        }
        else
        {
            /* 5. กินอาหารแล้วยาวขึ้น ไม่กินก็ลบหาง */
            if (willGrow)
            {
                if (s_length < APP_SNAKE_CELLS)
                {
                    s_length++;
                }
                result = APP_STEP_ATE;
            }
            else
            {
                s_occupancy[tailCell] = (uint8_t)APP_CELL_EMPTY;
            }

            /* 6. เขียนหัวใหม่ลง ring buffer และตารางช่อง */
            s_headIndex = (uint16_t)((s_headIndex + 1U) % APP_SNAKE_CELLS);
            s_body[s_headIndex].x = headX;
            s_body[s_headIndex].y = headY;
            s_occupancy[nextCell] = (uint8_t)APP_CELL_BODY;

            if (willGrow)
            {
                /* วางกำแพงก่อนอาหารเสมอ ไม่งั้นตัวนับช่องว่างของอาหาร
                 * จะไม่เห็นก้อนที่เพิ่งวาง แล้วเลือกช่องผิดตำแหน่ง */
                if ((s_mode == APP_SNAKE_MODE_WALL) && (s_wallCount < APP_SNAKE_MAX_WALLS))
                {
                    (void)Snake_PlaceWall();
                }

                if (!Snake_PlaceFood())
                {
                    result = APP_STEP_WIN;   /* ไม่มีช่องว่างเหลือแล้ว */
                }

                /* โหมด TWIN: กินเสร็จแล้วสลับหัวกับหาง
                 * ต้องทำหลังเขียนหัวใหม่และวางอาหารแล้ว เพื่อให้ลำตัว
                 * ครบถ้วนก่อนกลับด้าน · ตารางช่องไม่ต้องแก้เลย เพราะ
                 * ช่องที่งูครองอยู่เป็นชุดเดิม เปลี่ยนแค่ลำดับหัว-หาง */
                if (s_mode == APP_SNAKE_MODE_TWIN)
                {
                    Snake_SwapEnds();
                }
            }
        }
    }

    return result;
}

/*********************************************************************
 * @fn                - AppSnake_GetLength
 * @brief             - ความยาวตัวงูเป็นจำนวนช่อง
 *//////////////////////////////////////////////////////////////////////
uint16_t AppSnake_GetLength(void)
{
    return s_length;
}

/*********************************************************************
 * @fn                - AppSnake_GetCell
 * @brief             - สิ่งที่อยู่ในช่อง (x, y)
 *
 * @param[in]         - x : 0 .. APP_SNAKE_GRID_W-1
 * @param[in]         - y : 0 .. APP_SNAKE_GRID_H-1
 *
 * @return            - ชนิดของช่อง (นอกตารางคืน APP_CELL_EMPTY)
 *//////////////////////////////////////////////////////////////////////
AppCell_t AppSnake_GetCell(uint8_t x, uint8_t y)
{
    AppCell_t cell = APP_CELL_EMPTY;

    if ((x < APP_SNAKE_GRID_W) && (y < APP_SNAKE_GRID_H))
    {
        cell = (AppCell_t)s_occupancy[Snake_CellIndex(x, y)];
    }

    return cell;
}

/*********************************************************************
 * @fn                - AppSnake_GetHead
 * @brief             - ตำแหน่งหัวงูในตาราง
 *
 * @param[out]        - pX : ช่องแนวนอน
 * @param[out]        - pY : ช่องแนวตั้ง
 *//////////////////////////////////////////////////////////////////////
void AppSnake_GetHead(uint8_t * const pX, uint8_t * const pY)
{
    if ((pX != NULL) && (pY != NULL))
    {
        *pX = s_body[s_headIndex].x;
        *pY = s_body[s_headIndex].y;
    }
}

/* Private functions ---------------------------------------------------------*/

/* แปลงพิกัดเป็นดัชนีของตารางช่อง */
static uint16_t Snake_CellIndex(uint8_t x, uint8_t y)
{
    return (uint16_t)(((uint16_t)y * APP_SNAKE_GRID_W) + (uint16_t)x);
}

/* ดัชนีของ "หาง" ใน ring buffer (ช่องที่เก่าที่สุดของลำตัว) */
static uint16_t Snake_TailBodyIndex(void)
{
    uint16_t back = (uint16_t)(s_length - 1U);

    return (uint16_t)((s_headIndex + APP_SNAKE_CELLS - back) % APP_SNAKE_CELLS);
}

/* สองทิศนี้ตรงข้ามกันหรือไม่ */
static bool Snake_IsOpposite(AppDir_t a, AppDir_t b)
{
    bool opposite = false;

    if (((a == APP_DIR_UP) && (b == APP_DIR_DOWN)) ||
        ((a == APP_DIR_DOWN) && (b == APP_DIR_UP)) ||
        ((a == APP_DIR_LEFT) && (b == APP_DIR_RIGHT)) ||
        ((a == APP_DIR_RIGHT) && (b == APP_DIR_LEFT)))
    {
        opposite = true;
    }
    else
    {
        opposite = false;
    }

    return opposite;
}

/*  วางอาหารในช่องว่าง
 *  วิธี: นับช่องว่างก่อน (= จำนวนช่องทั้งหมด - ความยาวงู) แล้วสุ่มว่าจะเอา
 *  ช่องว่างลำดับที่เท่าไหร่ จากนั้นไล่หาช่องนั้น — O(512) ต่อการกิน 1 ครั้ง
 *  ห้ามสุ่มพิกัดซ้ำจนกว่าจะเจอช่องว่าง เพราะตอนงูยาวจะสุ่มไม่จบ
 */
static bool Snake_PlaceFood(void)
{
    bool placed = false;
    /* ช่องว่างจริง = ทั้งกระดาน - ตัวงู - ก้อนกำแพง
     * ถ้าลืมหักกำแพง ตัวเดินหาช่องจะเดินไม่ถึงลำดับที่สุ่มได้ แล้ววางอาหารไม่ลง */
    uint16_t freeCount = (uint16_t)(APP_SNAKE_CELLS - s_length - s_wallCount);

    if (freeCount > 0U)
    {
        uint16_t pick = DrvRand_Below(freeCount);
        uint16_t index = 0U;

        while ((index < APP_SNAKE_CELLS) && (!placed))
        {
            if (s_occupancy[index] == (uint8_t)APP_CELL_EMPTY)
            {
                if (pick == 0U)
                {
                    s_occupancy[index] = (uint8_t)APP_CELL_FOOD;
                    placed = true;
                }
                else
                {
                    pick--;
                }
            }
            index++;
        }
    }

    return placed;
}

/*  สลับหัวกับหางของงู
 *  ring buffer ช่วงเดิม [หาง .. หัว] ถูกกลับลำดับในที่ จึงไม่ต้องย้ายช่วง
 *  ช่อง s_headIndex เลยกลายเป็นพิกัดของหางเดิมโดยอัตโนมัติ
 *  ทิศใหม่ต้องชี้ออกจากลำตัว คำนวณจากช่องถัดเข้าไปมาหาหัวใหม่
 *  ถ้าไม่ตั้ง s_dir ใหม่ งูจะเดินย้อนเข้าตัวเองแล้วตายทันทีที่กินเม็ดแรก
 */
static void Snake_SwapEnds(void)
{
    uint16_t half = (uint16_t)(s_length / 2U);
    uint16_t tail = Snake_TailBodyIndex();
    uint16_t step;

    for (step = 0U; step < half; step++)
    {
        uint16_t front = (uint16_t)((tail + step) % APP_SNAKE_CELLS);
        uint16_t back = (uint16_t)((s_headIndex + APP_SNAKE_CELLS - step) % APP_SNAKE_CELLS);
        SnakeCell_t swap = s_body[front];

        s_body[front] = s_body[back];
        s_body[back] = swap;
    }

    if (s_length >= 2U)
    {
        uint16_t neck = (uint16_t)((s_headIndex + APP_SNAKE_CELLS - 1U) % APP_SNAKE_CELLS);

        s_dir = Snake_DirBetween(neck, s_headIndex);
        s_pendingDir = s_dir;
    }
    else
    {
        /* งูยาวช่องเดียวไม่มีทิศให้คำนวณ (เกิดไม่ได้ เพราะเริ่มที่ 3 ช่อง) */
    }
}

/*  ทิศจากช่องหนึ่งไปอีกช่องหนึ่งใน ring buffer
 *  สองช่องนี้ติดกันเสมอ เพราะเป็นลำตัวที่ต่อเนื่องกัน
 */
static AppDir_t Snake_DirBetween(uint16_t fromBody, uint16_t toBody)
{
    AppDir_t dir;
    int16_t dx = (int16_t)((int16_t)s_body[toBody].x - (int16_t)s_body[fromBody].x);
    int16_t dy = (int16_t)((int16_t)s_body[toBody].y - (int16_t)s_body[fromBody].y);

    if (dx > 0)
    {
        dir = APP_DIR_RIGHT;
    }
    else if (dx < 0)
    {
        dir = APP_DIR_LEFT;
    }
    else if (dy > 0)
    {
        dir = APP_DIR_DOWN;
    }
    else
    {
        dir = APP_DIR_UP;
    }

    return dir;
}

/*  ช่องนี้วางกำแพงได้ไหม
 *  ต้องว่าง และต้องห่างหัวงูพอให้ผู้เล่นมีเวลาหลบ
 */
static bool Snake_IsWallSpot(uint16_t index)
{
    bool allowed = false;

    if (s_occupancy[index] == (uint8_t)APP_CELL_EMPTY)
    {
        int16_t dx = (int16_t)((int16_t)(index % APP_SNAKE_GRID_W) -
                               (int16_t)s_body[s_headIndex].x);
        int16_t dy = (int16_t)((int16_t)(index / APP_SNAKE_GRID_W) -
                               (int16_t)s_body[s_headIndex].y);

        if (dx < 0)
        {
            dx = (int16_t)(-dx);
        }
        else
        {
            /* บวกอยู่แล้ว */
        }

        if (dy < 0)
        {
            dy = (int16_t)(-dy);
        }
        else
        {
            /* บวกอยู่แล้ว */
        }

        allowed = ((int16_t)(dx + dy) >= (int16_t)SNAKE_WALL_MIN_GAP);
    }
    else
    {
        /* ช่องไม่ว่าง */
    }

    return allowed;
}

/*  วางก้อนกำแพงหนึ่งก้อน
 *  ใช้วิธีเดียวกับการวางอาหาร: นับช่องที่วางได้ก่อน แล้วไล่ไปหาช่องลำดับที่สุ่มได้
 *  ห้ามสุ่มพิกัดซ้ำจนกว่าจะเจอ เพราะตอนกระดานแน่นจะสุ่มไม่จบ
 */
static bool Snake_PlaceWall(void)
{
    bool placed = false;
    uint16_t spots = 0U;
    uint16_t index;

    for (index = 0U; index < APP_SNAKE_CELLS; index++)
    {
        if (Snake_IsWallSpot(index))
        {
            spots++;
        }
        else
        {
            /* ช่องนี้วางไม่ได้ */
        }
    }

    if (spots > 0U)
    {
        uint16_t pick = DrvRand_Below(spots);

        index = 0U;
        while ((index < APP_SNAKE_CELLS) && (!placed))
        {
            if (Snake_IsWallSpot(index))
            {
                if (pick == 0U)
                {
                    s_occupancy[index] = (uint8_t)APP_CELL_WALL;
                    s_wallCount++;
                    placed = true;
                }
                else
                {
                    pick--;
                }
            }
            else
            {
                /* ข้ามช่องที่วางไม่ได้ */
            }
            index++;
        }
    }
    else
    {
        /* ไม่มีช่องที่วางได้ ก็ไม่ต้องวาง */
    }

    return placed;
}
