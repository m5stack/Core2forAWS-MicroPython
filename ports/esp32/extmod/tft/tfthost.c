#include "stdlib.h"
#include "math.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "esp32_hal_spi.h"
#include "tfthost.h"

#define TRANS_MAX_SIZE (10 * 1024)

#define DEG_TO_RAD 0.01745329252
#define PI 3.14159265359
#define swap(a, b) { int16_t t = a; a = b; b = t; }

static float _angleOffset = -90;

extern spi_device_handle_t spi_handle;
QueueHandle_t spi_lock;

/* ------------------------------low method------------------------------- */
static void host_select(tft_host_t* host) {
	xSemaphoreTake(spi_lock, portMAX_DELAY);
}

static void host_deselect(tft_host_t* host) {
	xSemaphoreGive(spi_lock);
}

static void selectDC(tft_host_t* host, uint8_t level) {
    if (host->base.dc < 0) {
        return ;
    }
    gpio_set_level(host->base.dc, level);
}

static void writeCmd(tft_host_t* host, uint8_t cmd) {
    host->selectDC(host, 0);
    spiWriteU8(spi_handle, cmd);
}

static void writeCmdData(tft_host_t* host, uint8_t cmd, const uint8_t* data, uint16_t len) {
    host->selectDC(host, 0);
    spiWriteU8(spi_handle, cmd);
    if (len > 0) {
        host->selectDC(host, 1);
        spiWriteBytes(spi_handle, data, len, 1);
    }
}

static void writeCmdList(tft_host_t* host, const uint8_t *addr) {
    uint8_t  numCommands, numArgs, cmd;
    uint16_t ms;

    numCommands = *addr++;				// Number of commands to follow
    while(numCommands--) {				// For each command...
        cmd = *addr++;						// save command
        numArgs  = *addr++;					// Number of args to follow
        ms       = numArgs & TFT_CMD_DELAY;	// If high bit set, delay follows args
        numArgs &= ~TFT_CMD_DELAY;			// Mask out delay bit

        host->writeCmdData(host, cmd, (uint8_t *)addr, numArgs);

        addr += numArgs;

        if(ms) {
            ms = *addr++;              // Read post-command delay time (ms)
            if(ms == 255) ms = 500;    // If 255, delay for 500 ms
            vTaskDelay(ms / portTICK_RATE_MS);
        }
    }
}

static void transferAddrWin(tft_host_t* host, uint16_t x1, uint16_t x2, uint16_t y1, uint16_t y2) {
  	uint32_t wd;

	wd = (uint32_t)(x1 >> 8);
	wd |= (uint32_t)(x1 & 0xff) << 8;
	wd |= (uint32_t)(x2 >> 8) << 16;
	wd |= (uint32_t)(x2 & 0xff) << 24;
	host->writeCmdData(host, TFT_CASET, (uint8_t *)&wd, 4);

	wd = (uint32_t)(y1 >> 8);
	wd |= (uint32_t)(y1 & 0xff) << 8;
	wd |= (uint32_t)(y2 >> 8) << 16;
	wd |= (uint32_t)(y2 & 0xff) << 24;
	host->writeCmdData(host, TFT_PASET, (uint8_t *)&wd, 4);  
}

static void setRotation(tft_host_t* host, uint8_t rot) {
    uint8_t madctl = 0;
	host->select(host);

    if (rot & 0x01) {
        host->_width = host->base.hight;
        host->_hight = host->base.width;
    } else {
        host->_width = host->base.width;
        host->_hight = host->base.hight;
    }
	
	host->setPrintArea(host, 0, 0, host->_width - 1, host->_hight - 1);

    switch (rot) {
        case PORTRAIT:
            madctl = (MADCTL_MY | MADCTL_MX | TFT_RGB_BGR);
            break;
        case LANDSCAPE:
            madctl = (MADCTL_MY | MADCTL_MV | TFT_RGB_BGR);
            break;
        case PORTRAIT_FLIP:
            madctl = (TFT_RGB_BGR);
            break;
        case LANDSCAPE_FLIP:
            madctl = (MADCTL_MX | MADCTL_MV | TFT_RGB_BGR);
            break;
    }

    host->writeCmdData(host, TFT_MADCTL, &madctl, 1);
	host->deselect(host);
}

static void drawPixel(tft_host_t* host, int16_t x, int16_t y, uint32_t color) {
    if ((x > host->_width) || (y > host->_hight) || (x < 0) || (y < 0)) { return; }
	host->select(host);
    host->transferAddrWin(host, x, x + 1, y, y + 1);
    color = ((color >> 8) & 0xff) | ((color & 0xff) << 8);
    host->writeCmdData(host, TFT_RAMWR, (uint8_t *)&color, 2);
	host->deselect(host);
}

static void pushColor(tft_host_t* host, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color, uint32_t len, uint8_t wait) {
    if (len == 0) {
        return ;
    }
	host->select(host);
    host->transferAddrWin(host, x1, x2, y1, y2);

    host->writeCmd(host, TFT_RAMWR);
    
    host->selectDC(host, 1);

    uint16_t lens = (len * 2) > TRANS_MAX_SIZE ? TRANS_MAX_SIZE : (len * 2);

    for (uint16_t i = 0; i < lens; i += 2) {
        host->trans_cline[i] = (color >> 8) & 0xff;
        host->trans_cline[i + 1] = color & 0xff;
    }

    int32_t to_sends = len * 2;
    while (to_sends > 0) {
        spiWriteBytesDMA(spi_handle, host->trans_cline, to_sends > lens ? lens : to_sends);
        to_sends -= lens;
    }

    if (wait) {
        spiWaitFinish(spi_handle);
		host->deselect(host);
	}
}

static void pushColorBuffer(tft_host_t* host, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint8_t* color, uint32_t len) {

}

static void setBrightness(tft_host_t* host, uint8_t brightness) {

}

/* ----------------------------------------------------------------------------- */
// Used to do circles and roundrects
static void fillCircleHelper(tft_host_t* host, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, int16_t delta, uint32_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;
	int16_t ylm = x0 - r;

	while (x < y) {
		if (f >= 0) {
			if (cornername & 0x1) host->drawFastVLine(host, x0 + y, y0 - x, 2 * x + 1 + delta, color);
			if (cornername & 0x2) host->drawFastVLine(host, x0 - y, y0 - x, 2 * x + 1 + delta, color);
			ylm = x0 - y;
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;

		if ((x0 - x) > ylm) {
			if (cornername & 0x1) host->drawFastVLine(host, x0 + x, y0 - y, 2 * y + 1 + delta, color);
			if (cornername & 0x2) host->drawFastVLine(host, x0 - x, y0 - y, 2 * y + 1 + delta, color);
		}
	}
}

static void drawCircleHelper(tft_host_t* host, int16_t x0, int16_t y0, int16_t r, uint8_t cornername, uint32_t color) {
	int16_t f = 1 - r;
	int16_t ddF_x = 1;
	int16_t ddF_y = -2 * r;
	int16_t x = 0;
	int16_t y = r;

	while (x < y) {
		if (f >= 0) {
			y--;
			ddF_y += 2;
			f += ddF_y;
		}
		x++;
		ddF_x += 2;
		f += ddF_x;
		if (cornername & 0x4) {
			host->drawPixel(host, x0 + x, y0 + y, color);
			host->drawPixel(host, x0 + y, y0 + x, color);
		}
		if (cornername & 0x2) {
			host->drawPixel(host, x0 + x, y0 - y, color);
			host->drawPixel(host, x0 + y, y0 - x, color);
		}
		if (cornername & 0x8) {
			host->drawPixel(host, x0 - y, y0 + x, color);
			host->drawPixel(host, x0 - x, y0 + y, color);
		}
		if (cornername & 0x1) {
			host->drawPixel(host, x0 - y, y0 - x, color);
			host->drawPixel(host, x0 - x, y0 - y, color);
		}
	}
}

static void drawEllipseSection(tft_host_t* host, uint16_t x, uint16_t y, uint16_t x0, uint16_t y0, uint32_t color, uint8_t option) {
    // upper right
    if ( option & TFT_ELLIPSE_UPPER_RIGHT ) host->drawPixel(host, x0 + x, y0 - y, color);
    // upper left
    if ( option & TFT_ELLIPSE_UPPER_LEFT ) host->drawPixel(host, x0 - x, y0 - y, color);
    // lower right
    if ( option & TFT_ELLIPSE_LOWER_RIGHT ) host->drawPixel(host, x0 + x, y0 + y, color);
    // lower left
    if ( option & TFT_ELLIPSE_LOWER_LEFT ) host->drawPixel(host, x0 - x, y0 + y, color);
}

static void fillEllipseSection(tft_host_t* host, uint16_t x, uint16_t y, uint16_t x0, uint16_t y0, uint32_t color, uint8_t option) {
    // upper right
    if ( option & TFT_ELLIPSE_UPPER_RIGHT ) host->drawFastVLine(host, x0+x, y0-y, y+1, color);
    // upper left
    if ( option & TFT_ELLIPSE_UPPER_LEFT ) host->drawFastVLine(host, x0-x, y0-y, y+1, color);
    // lower right
    if ( option & TFT_ELLIPSE_LOWER_RIGHT ) host->drawFastVLine(host, x0+x, y0, y+1, color);
    // lower left
    if ( option & TFT_ELLIPSE_LOWER_LEFT ) host->drawFastVLine(host, x0-x, y0, y+1, color);
}

static void fillArcOffsetted(tft_host_t* host, uint16_t cx, uint16_t cy, uint16_t radius, uint16_t thickness, float start, float end, uint32_t color) {
	//float sslope = (float)cos_lookup(start) / (float)sin_lookup(start);
	//float eslope = (float)cos_lookup(end) / (float)sin_lookup(end);
	float sslope = (cos(start/360 * 2 * PI) * 360) / (sin(start/360 * 2 * PI) * 360) ;
	float eslope = (cos(end/360 * 2 * PI) * 360) / (sin(end/360 * 2 * PI) * 360);

	if (end == 360) eslope = -1000000;

	int ir2 = (radius - thickness) * (radius - thickness);
	int or2 = radius * radius;

	for (int x = -radius; x <= radius; x++) {
		for (int y = -radius; y <= radius; y++) {
			int x2 = x * x;
			int y2 = y * y;

			if (
				(x2 + y2 < or2 && x2 + y2 >= ir2) &&
				(
				(y > 0 && start < 180 && x <= y * sslope) ||
				(y < 0 && start > 180 && x >= y * sslope) ||
				(y < 0 && start <= 180) ||
				(y == 0 && start <= 180 && x < 0) ||
				(y == 0 && start == 0 && x > 0)
				) &&
				(
				(y > 0 && end < 180 && x >= y * eslope) ||
				(y < 0 && end > 180 && x <= y * eslope) ||
				(y > 0 && end >= 180) ||
				(y == 0 && end >= 180 && x < 0) ||
				(y == 0 && start == 0 && x > 0)
				)
				)
				host->drawPixel(host, cx + x, cy + y, color);
		}
	}
}

/* -------------------------- draw method ----------------------------- */
static void drawFastVLine(tft_host_t* host, int16_t x, int16_t y, int16_t h, uint32_t color) {
	if ((x < 0) || (x > host->_width) || (y > host->_hight)) { return ; }
    if (y < 0) { h += y; y = 0; }
	if (h <= 0) { return; }
	if ((y + h) > (host->_hight+1)) { h = host->_hight - y + 1; }
    host->pushColor(host, x,  y, x, y + h + 1, color, h, 1);
}

static void drawFastHLine(tft_host_t* host, int16_t x, int16_t y, int16_t w, uint32_t color) {
    // clipping
	if ((y < 0) || (x > host->_width) || (y > host->_hight)) { return; }
	if (x < 0) { w -= (0 - x); x = 0; }
	if (w <= 0) { return; }
	if ((x + w) > (host->_width+1)) { w = host->_width - x + 1; }
	host->pushColor(host, x, y, x+w-1, y, color, (uint32_t)w, 1);
}

static void drawLine(tft_host_t* host, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint32_t color) {
    if (x0 == x1) {
        if (y0 <= y1) {
            host->drawFastVLine(host, x0, y0, y1-y0+1, color); 
        } else {
            host->drawFastVLine(host, x0, y1, y0-y1+1, color);
        }
        return;
    }

    if (y0 == y1) {
        if (x0 <= x1) {
            host->drawFastHLine(host, x0, y0, x1-x0+1, color);
        } else {
            host->drawFastHLine(host, x1, y0, x0-x1+1, color);
        }
        return;
    }

    int steep = 0;
    if (abs(y1 - y0) > abs(x1 - x0)) { steep = 1; }
    if (steep) {
        swap(x0, y0);
        swap(x1, y1);
    }
    if (x0 > x1) {
        swap(x0, x1);
        swap(y0, y1);
    }

    int16_t dx = x1 - x0, dy = abs(y1 - y0);
    int16_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

    if (y0 < y1) { ystep = 1; }

    // Split into steep and not steep for FastH/V separation
    if (steep) {
        for (; x0 <= x1; x0++) {
            dlen++;
            err -= dy;
            if (err < 0) {
                err += dx;
                if (dlen == 1) { 
                    host->drawPixel(host, y0, xs, color); 
                } else {
                    host->drawFastVLine(host, y0, xs, dlen, color);
                }
                dlen = 0; y0 += ystep; xs = x0 + 1;
            }
        }
        if (dlen) { host->drawFastVLine(host, y0, xs, dlen, color); }
        return ;
    }

    for (; x0 <= x1; x0++) {
        dlen++;
        err -= dy;
        if (err < 0) {
            err += dx;
            if (dlen == 1) {
                host->drawPixel(host, xs, y0, color);
            } else {
                host->drawFastHLine(host, xs, y0, dlen, color);
            } 
            dlen = 0; y0 += ystep; xs = x0 + 1;
        }
    }
    if (dlen) { host->drawFastHLine(host, xs, y0, dlen, color); }
}

static void drawLineByAngle(tft_host_t* host, uint16_t x, uint16_t y, uint16_t start, uint16_t len, uint16_t angle, uint32_t color) {
    host->drawLine(host, x, y, x + len * cos((angle - 90) * DEG_TO_RAD), y + len * sin((angle - 90) * DEG_TO_RAD), color);
}

static void fillRect(tft_host_t* host, int16_t x, int16_t y, int16_t w, int16_t h, uint32_t color) {
    if ((x >= host->_width) || (y > host->_hight)) return;
	if (x < 0) { w += x; x = 0; }
	if (y < 0) { h += y; y = 0; }
	if ((w <= 0) || (h <= 0)) { return; }
	if ((x + w) > (host->_width+1)) { w = host->_width - x + 1; }
	if ((y + h) > (host->_hight+1)) { h = host->_hight - y + 1; }

    host->pushColor(host, x, y, x+w-1, y+h-1, color, (uint32_t)(h*w), 1);
}

static void drawRect(tft_host_t* host, uint16_t x1,uint16_t y1,uint16_t w,uint16_t h, uint32_t color) {
    host->drawFastHLine(host, x1, y1, w, color);
    host->drawFastVLine(host, x1 + w - 1, y1, h, color);
    host->drawFastHLine(host, x1, y1 + h - 1, w, color);
    host->drawFastVLine(host, x1, y1, h, color);
}

static void drawRoundRect(tft_host_t* host, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color) {
	// smarter version
	host->drawFastHLine(host, x + r, y, w - 2 * r, color);			// Top
	host->drawFastHLine(host, x + r, y + h - 1, w - 2 * r, color);	// Bottom
	host->drawFastVLine(host, x, y + r, h - 2 * r, color);			// Left
	host->drawFastVLine(host, x + w - 1, y + r, h - 2 * r, color);	// Right

	// draw four corners
	drawCircleHelper(host, x + r, y + r, r, 1, color);
	drawCircleHelper(host, x + w - r - 1, y + r, r, 2, color);
	drawCircleHelper(host, x + w - r - 1, y + h - r - 1, r, 4, color);
	drawCircleHelper(host, x + r, y + h - r - 1, r, 8, color);
}

static void fillRoundRect(tft_host_t* host, int16_t x, int16_t y, uint16_t w, uint16_t h, uint16_t r, uint32_t color) {
    host->fillRect(host, x + r, y, w - 2 * r, h, color);

	// draw four corners
	fillCircleHelper(host, x + w - r - 1, y + r, r, 1, h - 2 * r - 1, color);
	fillCircleHelper(host, x + r, y + r, r, 2, h - 2 * r - 1, color);
}

static void fillScreen(tft_host_t* host, uint32_t color) {
    host->pushColor(host, 0, 0, host->_width - 1, host->_hight - 1, color, host->_width * host->_hight, 1);
}

static void drawTriangle(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    host->drawLine(host, x0, y0, x1, y1, color);
	host->drawLine(host, x1, y1, x2, y2, color);
	host->drawLine(host, x2, y2, x0, y0, color);
}

static void fillTriangle(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, uint32_t color) {
    int16_t a, b, y, last;

    // Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1) {
        swap(y0, y1); swap(x0, x1);
    }
    if (y1 > y2) {
        swap(y2, y1); swap(x2, x1);
    }
    if (y0 > y1) {
        swap(y0, y1); swap(x0, x1);
    }

    if(y0 == y2) { // Handle awkward all-on-same-line case as its own thing
        a = b = x0;
        if(x1 < a) {
            a = x1;
        } else if(x1 > b) {
            b = x1;
        }

        if(x2 < a) {
            a = x2;
        } else if(x2 > b) {
            b = x2;
        }
        host->drawFastHLine(host, a, y0, b-a+1, color);
        return;
    }

    int16_t
        dx01 = x1 - x0,
        dy01 = y1 - y0,
        dx02 = x2 - x0,
        dy02 = y2 - y0,
        dx12 = x2 - x1,
        dy12 = y2 - y1;
    int32_t
        sa   = 0,
        sb   = 0;

    // For upper part of triangle, find scanline crossings for segments
    // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
    // is included here (and second loop will be skipped, avoiding a /0
    // error there), otherwise scanline y1 is skipped here and handled
    // in the second loop...which also avoids a /0 error here if y0=y1
    // (flat-topped triangle).
    if(y1 == y2) last = y1;   // Include y1 scanline
    else         last = y1-1; // Skip it

    for(y=y0; y<=last; y++) {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
        /* longhand:
        a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) swap(a, b);
        host->drawFastHLine(host, a, y, b-a+1, color);
    }

    // For lower part of triangle, find scanline crossings for segments
    // 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++) {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
        /* longhand:
        a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
        b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
        */
        if(a > b) swap(a,b);
        host->drawFastHLine(host, a, y, b-a+1, color);
    }
}

static void drawCircle(tft_host_t* host, int16_t x, int16_t y, int radius, uint32_t color) {
    int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int x1 = 0;
	int y1 = radius;

	host->drawPixel(host, x, y + radius, color);
	host->drawPixel(host, x, y - radius, color);
	host->drawPixel(host, x + radius, y, color);
	host->drawPixel(host, x - radius, y, color);
	while(x1 < y1) {
		if (f >= 0) {
			y1--;
			ddF_y += 2;
			f += ddF_y;
		}
		x1++;
		ddF_x += 2;
		f += ddF_x;
		host->drawPixel(host, x + x1, y + y1, color);
		host->drawPixel(host, x - x1, y + y1, color);
		host->drawPixel(host, x + x1, y - y1, color);
		host->drawPixel(host, x - x1, y - y1, color);
		host->drawPixel(host, x + y1, y + x1, color);
		host->drawPixel(host, x - y1, y + x1, color);
		host->drawPixel(host, x + y1, y - x1, color);
		host->drawPixel(host, x - y1, y - x1, color);
	}
}

static void fillCircle(tft_host_t* host, int16_t x, int16_t y, int radius, uint32_t color) {
    host->drawFastVLine(host, x, y - radius, 2 * radius + 1, color);
	fillCircleHelper(host, x, y, radius, 3, 0, color);
}

static void drawEllipse(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint32_t color, uint8_t option) {
	uint16_t x, y;
	int32_t xchg, ychg;
	int32_t err;
	int32_t rxrx2;
	int32_t ryry2;
	int32_t stopx, stopy;

	rxrx2 = rx;
	rxrx2 *= rx;
	rxrx2 *= 2;

	ryry2 = ry;
	ryry2 *= ry;
	ryry2 *= 2;

	x = rx;
	y = 0;

	xchg = 1;
	xchg -= rx;
	xchg -= rx;
	xchg *= ry;
	xchg *= ry;

	ychg = rx;
	ychg *= rx;

	err = 0;

	stopx = ryry2;
	stopx *= rx;
	stopy = 0;

	while( stopx >= stopy ) {
		drawEllipseSection(host, x, y, x0, y0, color, option);
		y++;
		stopy += rxrx2;
		err += ychg;
		ychg += rxrx2;
		if ( 2*err+xchg > 0 ) {
			x--;
			stopx -= ryry2;
			err += xchg;
			xchg += ryry2;
		}
	}

	x = 0;
	y = ry;

	xchg = ry;
	xchg *= ry;

	ychg = 1;
	ychg -= ry;
	ychg -= ry;
	ychg *= rx;
	ychg *= rx;

	err = 0;

	stopx = 0;

	stopy = rxrx2;
	stopy *= ry;

	while( stopx <= stopy ) {
		drawEllipseSection(host, x, y, x0, y0, color, option);
		x++;
		stopx += ryry2;
		err += xchg;
		xchg += ryry2;
		if ( 2*err+ychg > 0 ) {
			y--;
			stopy -= rxrx2;
			err += ychg;
			ychg += rxrx2;
		}
	}
}

static void fillEllipse(tft_host_t* host, uint16_t x0, uint16_t y0, uint16_t rx, uint16_t ry, uint32_t color, uint8_t option) {
	uint16_t x, y;
	int32_t xchg, ychg;
	int32_t err;
	int32_t rxrx2;
	int32_t ryry2;
	int32_t stopx, stopy;

	rxrx2 = rx;
	rxrx2 *= rx;
	rxrx2 *= 2;

	ryry2 = ry;
	ryry2 *= ry;
	ryry2 *= 2;

	x = rx;
	y = 0;

	xchg = 1;
	xchg -= rx;
	xchg -= rx;
	xchg *= ry;
	xchg *= ry;

	ychg = rx;
	ychg *= rx;

	err = 0;

	stopx = ryry2;
	stopx *= rx;
	stopy = 0;

	while( stopx >= stopy ) {
		fillEllipseSection(host, x, y, x0, y0, color, option);
		y++;
		stopy += rxrx2;
		err += ychg;
		ychg += rxrx2;
		if ( 2*err+xchg > 0 ) {
			x--;
			stopx -= ryry2;
			err += xchg;
			xchg += ryry2;
		}
	}

	x = 0;
	y = ry;

	xchg = ry;
	xchg *= ry;

	ychg = 1;
	ychg -= ry;
	ychg -= ry;
	ychg *= rx;
	ychg *= rx;

	err = 0;

	stopx = 0;

	stopy = rxrx2;
	stopy *= ry;

	while( stopx <= stopy ) {
		fillEllipseSection(host, x, y, x0, y0, color, option);
		x++;
		stopx += ryry2;
		err += xchg;
		xchg += ryry2;
		if ( 2*err+ychg > 0 ) {
			y--;
			stopy -= rxrx2;
			err += ychg;
			ychg += rxrx2;
		}
	}
}

static void drawArc(tft_host_t* host, uint16_t cx, uint16_t cy, uint16_t r, uint16_t th, float start, float end, uint32_t color, uint32_t fillcolor) {
	if (th < 1) th = 1;
	if (th > r) th = r;

	int f = (fillcolor == color);

	float astart = fmodf(start, 360);
	float aend = fmodf(end, 360);

	astart += _angleOffset;
	aend += _angleOffset;

	if (astart < 0) astart += (float)360;
	if (aend < 0) aend += (float)360;

	if (aend == 0) aend = (float)360;

	if (astart > aend) {
		fillArcOffsetted(host, cx, cy, r, th, astart, 360, fillcolor);
		fillArcOffsetted(host, cx, cy, r, th, 0, aend, fillcolor);
		if (f) {
			fillArcOffsetted(host, cx, cy, r, 1, astart, 360, color);
			fillArcOffsetted(host, cx, cy, r, 1, 0, aend, color);
			fillArcOffsetted(host, cx, cy, r-th, 1, astart, 360, color);
			fillArcOffsetted(host, cx, cy, r-th, 1, 0, aend, color);
		}
	}
	else {
		fillArcOffsetted(host, cx, cy, r, th, astart, aend, fillcolor);
		if (f) {
			fillArcOffsetted(host, cx, cy, r, 1, astart, aend, color);
			fillArcOffsetted(host, cx, cy, r-th, 1, astart, aend, color);
		}
	}
	if (f) {
		host->drawLine(host, cx + (r-th) * cos(astart * DEG_TO_RAD), cy + (r-th) * sin(astart * DEG_TO_RAD),
			cx + (r-1) * cos(astart * DEG_TO_RAD), cy + (r-1) * sin(astart * DEG_TO_RAD), color);
		host->drawLine(host, cx + (r-th) * cos(aend * DEG_TO_RAD), cy + (r-th) * sin(aend * DEG_TO_RAD),
			cx + (r-1) * cos(aend * DEG_TO_RAD), cy + (r-1) * sin(aend * DEG_TO_RAD), color);
	}
}

static void drawBitmap(tft_host_t* host, int16_t x, int16_t y, int16_t w, int16_t h, uint8_t* bitmap, uint32_t color, uint32_t bg_color) {
	// do not draw bg color
	if (color != bg_color) {
		// host->fillRect(host, x, y, w, h, bg_color);
	}

	uint8_t bit = 0, bits = 0;
	uint8_t hpc = 0;
	uint8_t* bit_ptr = bitmap;
	uint16_t xx, yy;

	for (yy = 0; yy < h; yy++) {
		for (xx = 0; xx < w; xx++) {
			if (bit == 0) {
				bits = *bit_ptr;
				bit_ptr += 1;
				bit = 0x80;
			}
			if (bits & bit) {
				hpc++;
			} else {
				if (hpc) {
					host->drawFastHLine(host, x + xx - hpc, y + yy, hpc, color);
					hpc = 0;
				}
			}
			bit = bit >> 1;
		}

		if (hpc) {
			host->drawFastHLine(host, x + xx - hpc, y  + yy, hpc, color);
			hpc = 0;
		}
	}
}

static void setColor(tft_host_t *host, uint32_t color, uint32_t bg_color) {
    host->color = color;
    host->bg_color = bg_color;
}

void LcdHostInitDefault(tft_host_t* host) {
    host->_width = host->base.width;
    host->_hight = host->base.hight;
	host->text_cursor.x1 = 0;
	host->text_cursor.x2 = host->_width - 1;
	host->text_cursor.y1 = 0;
	host->text_cursor.y2 = host->_hight - 1;
    host->selectDC = selectDC; 
    host->writeCmd = writeCmd;
    host->writeCmdData = writeCmdData;
    host->writeCmdList = writeCmdList;
    host->transferAddrWin = transferAddrWin;
    host->pushColor = pushColor;
    host->pushColorBuffer = pushColorBuffer;
    host->setRotation = setRotation;
    
    host->drawPixel = drawPixel;
    host->setBrightness = setBrightness;
    host->drawFastVLine = drawFastVLine;
    host->drawFastHLine = drawFastHLine;
    host->drawLine = drawLine;
    host->drawLineByAngle = drawLineByAngle;
    host->fillRect = fillRect;
    host->drawRect = drawRect;
    host->drawRoundRect = drawRoundRect;
    host->fillRoundRect = fillRoundRect;
    host->fillScreen = fillScreen;
    host->drawTriangle = drawTriangle;
    host->fillTriangle = fillTriangle;
    host->drawCircle = drawCircle;
    host->fillCircle = fillCircle;
    host->drawEllipse = drawEllipse;
    host->fillEllipse = fillEllipse;
    host->drawArc = drawArc;

	host->drawBitmap = drawBitmap;
    host->setColor = setColor;

	if (host->trans_cline == NULL) {
        host->trans_cline = heap_caps_malloc(TRANS_MAX_SIZE, MALLOC_CAP_DEFAULT | MALLOC_CAP_DMA);
    }

    if (host->trans_cline == NULL) {
		printf("tft memory malloc failed (%dKB)", TRANS_MAX_SIZE / 1024);
        return ;
    }

	spi_lock = xSemaphoreCreateMutex();
	host->select = host_select;
	host->deselect = host_deselect;
}
