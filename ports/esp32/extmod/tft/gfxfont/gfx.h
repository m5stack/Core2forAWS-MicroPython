#pragma once

#include "stdint.h"
#include "../tfthost.h"

//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0	  // Top left (default)
#define TC_DATUM 1	  // Top centre
#define TR_DATUM 2	  // Top right
#define ML_DATUM 3	  // Middle left
#define CL_DATUM 3	  // Centre left, same as above
#define MC_DATUM 4	  // Middle centre
#define CC_DATUM 4	  // Centre centre, same as above
#define MR_DATUM 5	  // Middle right
#define CR_DATUM 5	  // Centre right, same as above
#define BL_DATUM 6	  // Bottom left
#define BC_DATUM 7	  // Bottom centre
#define BR_DATUM 8	  // Bottom right
#define L_BASELINE 9  // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

typedef struct
{							 // Data stored PER GLYPH
	uint32_t bitmapOffset;	 // Pointer into GFXfont->bitmap
	uint8_t width, height;	 // Bitmap dimensions in pixels
	uint8_t xAdvance;		 // Distance to advance cursor (x axis)
	int8_t xOffset, yOffset; // Dist from cursor pos to UL corner
} GFXglyph;

typedef struct
{						  // Data stored for FONT AS A WHOLE:
	uint8_t *bitmap;	  // Glyph bitmaps, concatenated
	GFXglyph *glyph;	  // Glyph array
	uint16_t first, last; // ASCII extents
	uint8_t yAdvance;	  // Newline distance (y axis)
} GFXfont;

extern const GFXfont DejaVuSans14pt7b;
extern const GFXfont DejaVuSans16pt7b;
extern const GFXfont DejaVuSans18pt7b;
extern const GFXfont DejaVuSans24pt7b;
extern const GFXfont DejaVuSans26pt7b;
extern const GFXfont DejaVuSans40pt7b;

enum {
	FONT_DEJAVUSANS_14 = 0x00,
	FONT_DEJAVUSANS_16,
	FONT_DEJAVUSANS_18,
	FONT_DEJAVUSANS_24,
	FONT_DEJAVUSANS_26,
	FONT_DEJAVUSANS_40,
	FONT_MAX,
};

void LcdHostInitFontGfx(tft_host_t *host);
