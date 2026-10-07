/*******************************************************************************
 * File Name    : app_render.c
 * Description  : หนึ่งเฟรม = ESC[H + 24 บรรทัด บรรทัดละ 35 ตัวอักษร + CR LF
 *                 = 895 ไบต์ ส่ง 78 ms ที่ 115200 (สั้นกว่าก้าวที่เร็วสุด 100 ms)
 *                 ทุกบรรทัดถูกเติมช่องว่างให้เต็มความกว้างเสมอ ของเดิมบนจอ
 *                 จึงถูกทับหมดโดยไม่ต้องล้างจอ
 * Date         : 2026-10-01
 ******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include <stddef.h>
#include "App/app_render.h"
#include "App/app_snake.h"
#include "Driver/drv_uart.h"

/* Private define ------------------------------------------------------------*/
#define RENDER_TEXT_W              (35U)    /* 1 ขอบซ้าย + กำแพง 34 ตัว */
#define RENDER_LINE_BYTES          (RENDER_TEXT_W + 2U)        /* + CR LF */
/* ทุกเฟรมขึ้นต้นด้วย ESC[?25l (ซ่อนเคอร์เซอร์) แล้วตามด้วย ESC[H (กลับมุมบนซ้าย)
 * ที่ต้องส่งซ้ำทุกเฟรม เพราะถ้าเปิด terminal หลังบอร์ดบูตไปแล้ว session นั้น
 * จะไม่เคยได้รับรหัสซ่อนเคอร์เซอร์ที่ส่งตอนบูตครั้งเดียว แล้วเคอร์เซอร์จะค้าง
 * เป็นก้อนทึบท้ายเฟรม · รหัสนี้ส่งซ้ำได้ ไม่มีผลข้างเคียง เปลือง 6 ไบต์ต่อเฟรม */
#define RENDER_PREFIX_BYTES        (9U)        /* ESC [ ? 2 5 l + ESC [ H */
#define RENDER_ROWS                (24U)
/* บรรทัดสุดท้ายไม่ใส่ CR LF: เฟรมสูง 24 บรรทัดเท่ากับหน้าต่าง PuTTY พอดี
 * ถ้าปิดท้ายด้วย CR LF เคอร์เซอร์จะตกไปบรรทัดที่ 25 แล้วดันจอเลื่อนขึ้นทุกเฟรม */
#define RENDER_FRAME_SIZE          ((RENDER_PREFIX_BYTES + (RENDER_ROWS * RENDER_LINE_BYTES)) - 2U)

/* ลำดับบรรทัดบนจอ */
#define RENDER_ROW_HEAD1           (0U)
#define RENDER_ROW_HEAD2           (1U)
#define RENDER_ROW_TOP             (2U)
#define RENDER_ROW_GRID            (3U)
#define RENDER_ROW_BOTTOM          (RENDER_ROW_GRID + APP_SNAKE_GRID_H)
#define RENDER_ROW_LEGEND          (RENDER_ROW_BOTTOM + 1U)
#define RENDER_ROW_KEYS            (RENDER_ROW_LEGEND + 1U)
#define RENDER_ROW_NOTICE          (RENDER_ROW_KEYS + 1U)
#define RENDER_ROW_ADC             (RENDER_ROW_NOTICE + 1U)

/* ตำแหน่งแนวนอน */
#define RENDER_COL_BORDER_L        (1U)
#define RENDER_COL_GRID            (2U)
#define RENDER_COL_BORDER_R        (RENDER_COL_GRID + APP_SNAKE_GRID_W)
#define RENDER_COL_LABEL           (1U)
#define RENDER_COL_SCORE_VALUE     (7U)
#define RENDER_COL_LEN_LABEL       (13U)
#define RENDER_COL_LEN_VALUE       (17U)
#define RENDER_COL_SPEED_LABEL     (22U)
#define RENDER_COL_SPEED_VALUE     (26U)
#define RENDER_COL_MODE            (28U)   /* 28..34 = 7 ตัวอักษรพอดีขอบขวา */
#define RENDER_COL_STATE           (13U)

/* สัญลักษณ์บนจอ */
#define RENDER_CHAR_WALL           ('#')
#define RENDER_CHAR_HEAD           ('@')
#define RENDER_CHAR_BODY           ('o')
#define RENDER_CHAR_FOOD           ('*')
#define RENDER_CHAR_BLOCK          ('X')   /* ก้อนกำแพงในโหมด WALL */
#define RENDER_CHAR_SPACE          (' ')

#define RENDER_COL_ADC_X           (5U)
#define RENDER_COL_ADC_Y           (14U)
#define RENDER_COL_ADC_P           (23U)
#define RENDER_DIGITS_RAW          (4U)
#define RENDER_DIGITS_SCORE        (4U)
#define RENDER_DIGITS_LENGTH       (3U)
#define RENDER_DECIMAL_BASE        (10U)
#define RENDER_MAX_DIGITS          (5U)

/* Private constants ---------------------------------------------------------*/
static const char s_digitChars[] = "0123456789";

/* Private variables ---------------------------------------------------------*/
static uint8_t s_frame[RENDER_FRAME_SIZE];

/* Private function prototypes -----------------------------------------------*/
static void Render_ClearText(void);
static void Render_PutChar(uint8_t row, uint8_t col, char value);
static void Render_PutText(uint8_t row, uint8_t col, const char * const pText);
static void Render_PutUInt(uint8_t row, uint8_t col, uint16_t value, uint8_t digits);
static void Render_DrawHeader(const AppRenderInfo_t * const pInfo);
static void Render_DrawBoard(void);
static void Render_DrawFooter(const AppRenderInfo_t * const pInfo);

/* Public functions ----------------------------------------------------------*/

/*********************************************************************
 * @fn                - AppRender_Init
 * @brief             - ใส่ ESC[H และ CR LF ท้ายทุกบรรทัดลง frame buffer
 *
 * @Note              - ส่วนนี้ไม่เปลี่ยนอีกตลอดเกม เขียนครั้งเดียวพอ
 *//////////////////////////////////////////////////////////////////////
void AppRender_Init(void)
{
    uint16_t row;

    s_frame[0] = 0x1BU;      /* ESC */
    s_frame[1] = (uint8_t)'[';
    s_frame[2] = (uint8_t)'?';
    s_frame[3] = (uint8_t)'2';
    s_frame[4] = (uint8_t)'5';
    s_frame[5] = (uint8_t)'l';
    s_frame[6] = 0x1BU;      /* ESC */
    s_frame[7] = (uint8_t)'[';
    s_frame[8] = (uint8_t)'H';

    for (row = 0U; row < (RENDER_ROWS - 1U); row++)
    {
        uint16_t lineEnd = (uint16_t)(RENDER_PREFIX_BYTES + (row * RENDER_LINE_BYTES) + RENDER_TEXT_W);

        s_frame[lineEnd] = (uint8_t)'\r';
        s_frame[lineEnd + 1U] = (uint8_t)'\n';
    }

    Render_ClearText();
}

/*********************************************************************
 * @fn                - AppRender_SendFrame
 * @brief             - วาดฉากทั้งหน้าแล้วส่งออก UART ด้วย DMA
 *
 * @param[in]         - pInfo : คะแนน ความยาว ความเร็ว และข้อความสถานะ
 *
 * @return            - true ถ้าเริ่มส่งแล้ว, false ถ้า UART ยังไม่ว่าง
 *
 * @Note              - ถ้า UART ไม่ว่างให้ทิ้งเฟรมนี้ ห้ามรอ (จะไปขวาง main loop)
 *//////////////////////////////////////////////////////////////////////
bool AppRender_SendFrame(const AppRenderInfo_t * const pInfo)
{
    bool sent = false;
    bool uartBusy = DrvUart_IsBusy();

    if ((pInfo != NULL) && (pInfo->pStateText != NULL) &&
        (pInfo->pModeText != NULL) && (!uartBusy))
    {
        Render_ClearText();
        Render_DrawHeader(pInfo);
        Render_DrawBoard();
        Render_DrawFooter(pInfo);

        sent = DrvUart_Send(s_frame, (uint16_t)RENDER_FRAME_SIZE);
    }

    return sent;
}

/* Private functions ---------------------------------------------------------*/

/* เติมช่องว่างทุกช่องของทุกบรรทัด (ไม่แตะ ESC[H และ CR LF) */
static void Render_ClearText(void)
{
    uint16_t row;

    for (row = 0U; row < RENDER_ROWS; row++)
    {
        uint16_t base = (uint16_t)(RENDER_PREFIX_BYTES + (row * RENDER_LINE_BYTES));
        uint16_t col;

        for (col = 0U; col < RENDER_TEXT_W; col++)
        {
            s_frame[base + col] = (uint8_t)RENDER_CHAR_SPACE;
        }
    }
}

/* เขียนตัวอักษรหนึ่งตัวที่ตำแหน่ง (row, col) ของจอ */
static void Render_PutChar(uint8_t row, uint8_t col, char value)
{
    if ((row < RENDER_ROWS) && (col < RENDER_TEXT_W))
    {
        uint16_t index = (uint16_t)(RENDER_PREFIX_BYTES +
                                    ((uint16_t)row * RENDER_LINE_BYTES) + (uint16_t)col);

        s_frame[index] = (uint8_t)value;
    }
}

/* เขียนข้อความจาก col ไปทางขวา ส่วนที่ล้นขอบจอถูกตัด */
static void Render_PutText(uint8_t row, uint8_t col, const char * const pText)
{
    if (pText != NULL)
    {
        uint8_t offset = 0U;

        while ((pText[offset] != '\0') && (((uint16_t)col + offset) < RENDER_TEXT_W))
        {
            Render_PutChar(row, (uint8_t)(col + offset), pText[offset]);
            offset++;
        }
    }
}

/* เขียนเลขฐานสิบแบบมีศูนย์นำหน้าให้ครบจำนวนหลักที่กำหนด */
static void Render_PutUInt(uint8_t row, uint8_t col, uint16_t value, uint8_t digits)
{
    char temp[RENDER_MAX_DIGITS];
    uint8_t count = digits;
    uint16_t rest = value;
    uint8_t slot;

    if (count > (uint8_t)RENDER_MAX_DIGITS)
    {
        count = (uint8_t)RENDER_MAX_DIGITS;
    }

    for (slot = count; slot > 0U; slot--)
    {
        temp[slot - 1U] = s_digitChars[rest % RENDER_DECIMAL_BASE];
        rest /= RENDER_DECIMAL_BASE;
    }

    for (slot = 0U; slot < count; slot++)
    {
        Render_PutChar(row, (uint8_t)(col + slot), temp[slot]);
    }
}

/* สองบรรทัดบนสุด: คะแนน ความยาว ความเร็ว คะแนนสูงสุด และสถานะ */
static void Render_DrawHeader(const AppRenderInfo_t * const pInfo)
{
    Render_PutText(RENDER_ROW_HEAD1, RENDER_COL_LABEL, "SCORE");
    Render_PutUInt(RENDER_ROW_HEAD1, RENDER_COL_SCORE_VALUE, pInfo->score, (uint8_t)RENDER_DIGITS_SCORE);
    Render_PutText(RENDER_ROW_HEAD1, RENDER_COL_LEN_LABEL, "LEN");
    Render_PutUInt(RENDER_ROW_HEAD1, RENDER_COL_LEN_VALUE, pInfo->length, (uint8_t)RENDER_DIGITS_LENGTH);
    Render_PutText(RENDER_ROW_HEAD1, RENDER_COL_SPEED_LABEL, "SPD");
    Render_PutUInt(RENDER_ROW_HEAD1, RENDER_COL_SPEED_VALUE, (uint16_t)pInfo->speedLevel, 1U);
    Render_PutText(RENDER_ROW_HEAD1, RENDER_COL_MODE, pInfo->pModeText);

    Render_PutText(RENDER_ROW_HEAD2, RENDER_COL_LABEL, "BEST");
    Render_PutUInt(RENDER_ROW_HEAD2, RENDER_COL_SCORE_VALUE, pInfo->best, (uint8_t)RENDER_DIGITS_SCORE);
    Render_PutText(RENDER_ROW_HEAD2, RENDER_COL_STATE, pInfo->pStateText);
}

/* กำแพงรอบสนาม ตัวงู หัวงู และอาหาร */
static void Render_DrawBoard(void)
{
    uint8_t x;
    uint8_t y;
    uint8_t headX = 0U;
    uint8_t headY = 0U;

    for (x = 0U; x < (uint8_t)(APP_SNAKE_GRID_W + 2U); x++)
    {
        Render_PutChar((uint8_t)RENDER_ROW_TOP, (uint8_t)(RENDER_COL_BORDER_L + x), RENDER_CHAR_WALL);
        Render_PutChar((uint8_t)RENDER_ROW_BOTTOM, (uint8_t)(RENDER_COL_BORDER_L + x), RENDER_CHAR_WALL);
    }

    AppSnake_GetHead(&headX, &headY);

    for (y = 0U; y < (uint8_t)APP_SNAKE_GRID_H; y++)
    {
        uint8_t row = (uint8_t)(RENDER_ROW_GRID + y);

        Render_PutChar(row, (uint8_t)RENDER_COL_BORDER_L, RENDER_CHAR_WALL);
        Render_PutChar(row, (uint8_t)RENDER_COL_BORDER_R, RENDER_CHAR_WALL);

        for (x = 0U; x < (uint8_t)APP_SNAKE_GRID_W; x++)
        {
            AppCell_t cell = AppSnake_GetCell(x, y);
            uint8_t col = (uint8_t)(RENDER_COL_GRID + x);

            if (cell == APP_CELL_FOOD)
            {
                Render_PutChar(row, col, RENDER_CHAR_FOOD);
            }
            else if (cell == APP_CELL_WALL)
            {
                Render_PutChar(row, col, RENDER_CHAR_BLOCK);
            }
            else if (cell == APP_CELL_BODY)
            {
                if ((x == headX) && (y == headY))
                {
                    Render_PutChar(row, col, RENDER_CHAR_HEAD);
                }
                else
                {
                    Render_PutChar(row, col, RENDER_CHAR_BODY);
                }
            }
            else
            {
                /* ช่องว่างถูกเติมด้วยช่องว่างไว้แล้วใน Render_ClearText() */
            }
        }
    }
}

/* สามบรรทัดล่าง: คำอธิบายสัญลักษณ์ ปุ่มที่ใช้ และข้อความแจ้งเตือน */
static void Render_DrawFooter(const AppRenderInfo_t * const pInfo)
{
    Render_PutText(RENDER_ROW_LEGEND, RENDER_COL_LABEL, "@ head   o body   * food");
    Render_PutText(RENDER_ROW_KEYS, RENDER_COL_LABEL, "D2 start D3 new D4 pause D5 mode");

    if (pInfo->pNoticeText != NULL)
    {
        Render_PutText(RENDER_ROW_NOTICE, RENDER_COL_LABEL, pInfo->pNoticeText);
    }

    /* บรรทัดค่าดิบของ ADC: โชว์ตอน IDLE เพื่อตรวจว่า joystick เข้าถูกขา
     * อยู่นิ่ง ~2048 ทั้งสองแกน · ดันก้านแล้วตัวเลขต้องวิ่งไป 0 หรือ 4095 */
    if (pInfo->showAdc)
    {
        Render_PutText(RENDER_ROW_ADC, RENDER_COL_LABEL, "PB0=");
        Render_PutUInt(RENDER_ROW_ADC, RENDER_COL_ADC_X, pInfo->joyXRaw, (uint8_t)RENDER_DIGITS_RAW);
        Render_PutText(RENDER_ROW_ADC, RENDER_COL_ADC_Y - 4U, "PC0=");
        Render_PutUInt(RENDER_ROW_ADC, RENDER_COL_ADC_Y, pInfo->joyYRaw, (uint8_t)RENDER_DIGITS_RAW);
        Render_PutText(RENDER_ROW_ADC, RENDER_COL_ADC_P - 4U, "POT=");
        Render_PutUInt(RENDER_ROW_ADC, RENDER_COL_ADC_P, pInfo->potRaw, (uint8_t)RENDER_DIGITS_RAW);
    }
}
