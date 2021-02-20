#pragma once
#include <stdint.h>

#define PORTRAIT	0
#define LANDSCAPE	1
#define PORTRAIT_FLIP	2
#define LANDSCAPE_FLIP	3

#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_MH  0x04

#define TFT_RGB_BGR 0x08

// --- Constants for ellipse function ---
#define TFT_ELLIPSE_UPPER_RIGHT 0x01
#define TFT_ELLIPSE_UPPER_LEFT  0x02
#define TFT_ELLIPSE_LOWER_LEFT  0x04
#define TFT_ELLIPSE_LOWER_RIGHT 0x08

// ==== Display commands constants ====
#define TFT_INVOFF     0x20
#define TFT_INVONN     0x21
#define TFT_DISPOFF    0x28
#define TFT_DISPON     0x29
#define TFT_MADCTL	   0x36
#define TFT_PTLAR 	   0x30
#define TFT_ENTRYM 	   0xB7

#define TFT_CMD_NOP			0x00
#define TFT_CMD_SWRESET		0x01
#define TFT_CMD_RDDID		0x04
#define TFT_CMD_RDDST		0x09

#define TFT_CMD_SLPIN		0x10
#define TFT_CMD_SLPOUT		0x11
#define TFT_CMD_PTLON		0x12
#define TFT_CMD_NORON		0x13

#define TFT_CMD_RDMODE		0x0A
#define TFT_CMD_RDMADCTL	0x0B
#define TFT_CMD_RDPIXFMT	0x0C
#define TFT_CMD_RDIMGFMT	0x0D
#define TFT_CMD_RDSELFDIAG  0x0F

#define TFT_CMD_GAMMASET	0x26

#define TFT_CMD_FRMCTR1		0xB1
#define TFT_CMD_FRMCTR2		0xB2
#define TFT_CMD_FRMCTR3		0xB3
#define TFT_CMD_INVCTR		0xB4
#define TFT_CMD_DFUNCTR		0xB6

#define TFT_CMD_PWCTR1		0xC0
#define TFT_CMD_PWCTR2		0xC1
#define TFT_CMD_PWCTR3		0xC2
#define TFT_CMD_PWCTR4		0xC3
#define TFT_CMD_PWCTR5		0xC4
#define TFT_CMD_VMCTR1		0xC5
#define TFT_CMD_VMCTR2		0xC7

#define TFT_CMD_RDID1		0xDA
#define TFT_CMD_RDID2		0xDB
#define TFT_CMD_RDID3		0xDC
#define TFT_CMD_RDID4		0xDD

#define TFT_CMD_GMCTRP1		0xE0
#define TFT_CMD_GMCTRN1		0xE1

#define TFT_CMD_POWERA		0xCB
#define TFT_CMD_POWERB		0xCF
#define TFT_CMD_POWER_SEQ	0xED
#define TFT_CMD_DTCA		0xE8
#define TFT_CMD_DTCB		0xEA
#define TFT_CMD_PRC			0xF7
#define TFT_CMD_3GAMMA_EN	0xF2

#define TFT_CMD_DELAY	0x80

#define TFT_CASET		0x2A
#define TFT_PASET		0x2B
#define TFT_RAMWR		0x2C
#define TFT_RAMRD		0x2E
#define TFT_CMD_PIXFMT	0x3A

#define TFT_COLOR_BITS_24	0x66
#define TFT_COLOR_BITS_16	0x55

// color
#define TFT_BLACK       0x0000      /*   0,   0,   0 */
#define TFT_NAVY        0x000F      /*   0,   0, 128 */
#define TFT_DARKGREEN   0x03E0      /*   0, 128,   0 */
#define TFT_DARKCYAN    0x03EF      /*   0, 128, 128 */
#define TFT_MAROON      0x7800      /* 128,   0,   0 */
#define TFT_PURPLE      0x780F      /* 128,   0, 128 */
#define TFT_OLIVE       0x7BE0      /* 128, 128,   0 */
#define TFT_LIGHTGREY   0xD69A      /* 211, 211, 211 */
#define TFT_DARKGREY    0x7BEF      /* 128, 128, 128 */
#define TFT_BLUE        0x001F      /*   0,   0, 255 */
#define TFT_GREEN       0x07E0      /*   0, 255,   0 */
#define TFT_CYAN        0x07FF      /*   0, 255, 255 */
#define TFT_RED         0xF800      /* 255,   0,   0 */
#define TFT_MAGENTA     0xF81F      /* 255,   0, 255 */
#define TFT_YELLOW      0xFFE0      /* 255, 255,   0 */
#define TFT_WHITE       0xFFFF      /* 255, 255, 255 */
#define TFT_ORANGE      0xFDA0      /* 255, 180,   0 */
#define TFT_GREENYELLOW 0xB7E0      /* 180, 255,   0 */
#define TFT_PINK        0xFE19      /* 255, 192, 203 */ //Lighter pink, was 0xFC9F      
#define TFT_BROWN       0x9A60      /* 150,  75,   0 */
#define TFT_GOLD        0xFEA0      /* 255, 215,   0 */
#define TFT_SILVER      0xC618      /* 192, 192, 192 */
#define TFT_SKYBLUE     0x867D      /* 135, 206, 235 */
#define TFT_VIOLET      0x915C      /* 180,  46, 226 */

#define iTFT_BLACK       0
#define iTFT_NAVY        128
#define iTFT_DARKGREEN   32768
#define iTFT_DARKCYAN    32896
#define iTFT_MAROON      8388608
#define iTFT_PURPLE      8388736
#define iTFT_OLIVE       8421376
#define iTFT_LIGHTGREY   12632256
#define iTFT_DARKGREY    8421504
#define iTFT_BLUE        255
#define iTFT_GREEN       65280
#define iTFT_CYAN        65535
#define iTFT_RED         16515072
#define iTFT_MAGENTA     16515327
#define iTFT_YELLOW      16579584
#define iTFT_WHITE       16579836
#define iTFT_ORANGE      16557056
#define iTFT_GREENYELLOW 11336748
#define iTFT_PINK        16564426

typedef struct _tft_host_t tft_host_t;

typedef struct _tft_base_t {
    uint16_t width;
    uint16_t hight;

    int miso;
    int mosi;
    int clk;
    int cs;
    int dc;
    int rst;
    int bckl;
} tft_base_t;

typedef struct {
    int16_t x;
    int16_t y;

    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} text_cursor_t;

typedef struct _tft_host_t {
    tft_base_t base;

    uint16_t _width;
    uint16_t _hight;

    uint32_t color;
    uint32_t bg_color;

    // used to send dma 
    uint8_t* trans_cline;
    text_cursor_t text_cursor;

    void (*selectDC)(tft_host_t* host, uint8_t level);
    void (*writeCmd)(tft_host_t* host, uint8_t cmd);
    void (*writeCmdData)(tft_host_t* host, uint8_t cmd, const uint8_t* data, uint16_t len);
    void (*writeCmdList)(tft_host_t* host, const uint8_t *addr);

    void (*transferAddrWin)(tft_host_t* host, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2);
    void (*setRotation)(tft_host_t* host, uint8_t rot);
    void (*pushColor)(tft_host_t* host, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint32_t len, uint8_t wait);
    void (*pushColorBuffer)(tft_host_t* host, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t* color, uint32_t len);

    void (*setBrightness)(tft_host_t* host, uint8_t brightness);

    void (*drawPixel)(tft_host_t* host, int16_t x, int16_t y, uint32_t color);
    void (*drawFastVLine)(tft_host_t* host, int16_t x, int16_t y, int16_t h, uint32_t color);
    void (*drawFastHLine)(tft_host_t* host, int16_t x, int16_t y, int16_t w, uint32_t color);
    void (*drawLine)(tft_host_t* host, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color);

    void (*drawLineByAngle)(tft_host_t* host, uint16_t x, uint16_t y, uint16_t start, uint16_t len, uint16_t angle, uint32_t color);
    void (*fillRect)(tft_host_t* host, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color);
    void (*drawRect)(tft_host_t* host, uint16_t x1,uint16_t y1,uint16_t w,uint16_t h, uint32_t color);
    void (*drawRoundRect)(tft_host_t* host, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color);
    void (*fillRoundRect)(tft_host_t* host, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color);
    void (*fillScreen)(tft_host_t* host, uint32_t color);
    void (*drawTriangle)(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
    void (*fillTriangle)(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color);
    void (*drawCircle)(tft_host_t* host, int16_t x, int16_t y, int radius, uint32_t color);
    void (*fillCircle)(tft_host_t* host, int16_t x, int16_t y, int radius, uint32_t color);
    void (*drawEllipse)(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint32_t color, uint8_t option);
    void (*fillEllipse)(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint32_t color, uint8_t option);
    void (*drawArc)(tft_host_t* host, uint16_t cx, uint16_t cy, uint16_t r, uint16_t th, float start, float end, uint32_t color, uint32_t fillcolor);
    void (*drawBitmap)(tft_host_t* host, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t* bitmap, uint32_t color, uint32_t bg_color);

    // text
    void (*setCursor)(tft_host_t* host, int16_t x, int16_t y);
    void (*setPrintArea)(tft_host_t* host, int16_t x1, int16_t y1, int16_t x2, int16_t y2);
    void (*getCursor)(tft_host_t* host, int16_t* x, int16_t* y);
    void (*print)(tft_host_t* host, const char* string);

    void (*setColor)(tft_host_t *host, uint32_t color, uint32_t bg_color);
    void (*setTextFont)(tft_host_t *host, const void* font);
    void (*setTextDatum)(tft_host_t *host, uint8_t datum);
    int16_t (*drawChar)(tft_host_t* host, int16_t x, int16_t y, int16_t c);
    int16_t (*drawString)(tft_host_t *host, int32_t poX, int32_t poY, const char *string);
    int16_t (*getStringWidth)(tft_host_t *host, const char* string);
    int16_t (*getStringHight)(tft_host_t *host, const char* string);
    
} tft_host_t;

void LcdHostInitDefault(tft_host_t* host);
