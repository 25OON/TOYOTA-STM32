/*******************************************************************************
 * File Name    : app_snake.h
 * Description  : ตรรกะตัวงู: ring buffer ของลำตัว + ตารางช่อง (occupancy)
 *                 เดินหนึ่งก้าว ตรวจชน กินอาหาร และวางอาหารใหม่
 * Date         : 2026-10-01
 ******************************************************************************/
#ifndef APP_SNAKE_H
#define APP_SNAKE_H

#include <stdbool.h>
#include <stdint.h>

/* Public define -------------------------------------------------------------*/
#define APP_SNAKE_GRID_W           (32U)
#define APP_SNAKE_GRID_H           (16U)
#define APP_SNAKE_CELLS            (512U)   /* 32 x 16 */
#define APP_SNAKE_START_LENGTH     (3U)

/* Public enum ---------------------------------------------------------------*/
typedef enum
{
    APP_DIR_UP = 0,
    APP_DIR_DOWN,
    APP_DIR_LEFT,
    APP_DIR_RIGHT
} AppDir_t;

/* โหมดการเล่น */
typedef enum
{
    APP_SNAKE_MODE_CLASSIC = 0,   /* กินแล้วยาวขึ้น เดินต่อไปทางเดิม */
    APP_SNAKE_MODE_TWIN,          /* กินแล้วสลับหัว-หาง ต้องบังคับอีกปลายแทน */
    APP_SNAKE_MODE_WALL           /* กินแล้วมีกำแพงโผล่มาขวาง สูงสุด 10 ก้อน */
} AppSnakeMode_t;

#define APP_SNAKE_MAX_WALLS        (10U)

/* สิ่งที่อยู่ในช่องหนึ่งช่องของตาราง */
typedef enum
{
    APP_CELL_EMPTY = 0,
    APP_CELL_BODY,
    APP_CELL_FOOD,
    APP_CELL_WALL      /* ก้อนกำแพงที่โผล่มาในโหมด WALL · ชนแล้วตาย */
} AppCell_t;

/* ผลของการเดินหนึ่งก้าว */
typedef enum
{
    APP_STEP_MOVED = 0,   /* เดินปกติ */
    APP_STEP_ATE,         /* กินอาหาร ตัวยาวขึ้น */
    APP_STEP_DIED,        /* ชนกำแพงหรือชนตัวเอง */
    APP_STEP_WIN          /* เต็มกระดาน ไม่มีที่วางอาหารอีก */
} AppStep_t;

/* Public function prototypes ------------------------------------------------*/

/* ตั้งงูใหม่ความยาว 3 ช่องกลางจอ หันไปทางขวา และวางอาหารเม็ดแรก
 * โหมดไม่ถูกล้าง เพราะผู้เล่นเลือกไว้ก่อนเริ่มเกม */
void AppSnake_Reset(void);

/* เลือกโหมด · มีผลตั้งแต่อาหารเม็ดถัดไป */
void AppSnake_SetMode(AppSnakeMode_t mode);

/* โหมดที่ใช้อยู่ */
AppSnakeMode_t AppSnake_GetMode(void);

/* ขอเปลี่ยนทิศ (ทิศตรงข้ามกับที่วิ่งอยู่จะถูกเมินตอนเดินก้าวถัดไป) */
void AppSnake_RequestDir(AppDir_t dir);

/* เดินหนึ่งก้าว คืนผลที่เกิดขึ้น */
AppStep_t AppSnake_Step(void);

/* ความยาวตัวงูเป็นจำนวนช่อง */
uint16_t AppSnake_GetLength(void);

/* สิ่งที่อยู่ในช่อง (x, y) · นอกตารางคืน APP_CELL_EMPTY */
AppCell_t AppSnake_GetCell(uint8_t x, uint8_t y);

/* ตำแหน่งหัวงู (ใช้วาดสัญลักษณ์หัวให้ต่างจากลำตัว) */
void AppSnake_GetHead(uint8_t * const pX, uint8_t * const pY);

#endif /* APP_SNAKE_H */
