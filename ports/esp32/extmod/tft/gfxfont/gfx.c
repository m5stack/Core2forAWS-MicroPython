#include "gfx.h"

const GFXfont *gfxFont = 0;
uint8_t glyph_ab, glyph_bb;
uint8_t isDigits = 0;
static uint8_t textdatum = TL_DATUM;

const GFXfont* font_map[FONT_MAX];

static int16_t getStringWidth(tft_host_t *host, const char *string) {
    int16_t width = 0;

    while (*string) {
        if (*string >= gfxFont->first && *string <= gfxFont->last) {
            GFXglyph *glyph = gfxFont->glyph + *string - gfxFont->first;

            if (*string || isDigits) {
                width += glyph->xAdvance;
            } else {
                width += glyph->xOffset + glyph->width;
            }
        } else {
            // not found text in font
        }
        string += 1;
    }
    isDigits = 0;
    return width;
}

static int16_t getStringHight(tft_host_t *host, const char *string) {
    return gfxFont->yAdvance;
}

static int16_t drawChar(tft_host_t *host, int16_t x, int16_t y, int16_t c) {
    if (y > host->_hight || x > host->_width) {
        return 0;
    }

    if (c < gfxFont->first || c > gfxFont->last) {
        return 0;
    }

    GFXglyph *glyph = &gfxFont->glyph[c - gfxFont->first];

    host->drawBitmap(host, x + glyph->xOffset, y + glyph->yOffset, glyph->width, glyph->height, 
                        gfxFont->bitmap + glyph->bitmapOffset, host->color, host->color);
    return glyph->xAdvance;
}

static int16_t drawString(tft_host_t *host, int32_t poX, int32_t poY, const char *string) {
    int16_t sumX = 0;
    uint8_t padding = 101, baseline = 0;
    int16_t cwidth = getStringWidth(host, string);
    int16_t cheight = glyph_ab;
    poY += cheight;
    baseline = cheight;

    if ((textdatum == BL_DATUM) || (textdatum == BC_DATUM) || (textdatum == BR_DATUM)) {
        cheight += glyph_bb;
    }

    switch(textdatum) {
        case TC_DATUM:
            poX -= cwidth / 2;
            padding += 1;
            break;
        case TR_DATUM:
            poX -= cwidth;
            padding += 2;
            break;
        case ML_DATUM:
            poY -= cheight / 2;
            // padding += 0;
            break;
        case MC_DATUM:
            poX -= cwidth / 2;
            poY -= cheight / 2;
            padding += 1;
            break;
        case MR_DATUM:
            poX -= cwidth;
            poY -= cheight / 2;
            padding += 2;
            break;
        case BL_DATUM:
            poY -= cheight;
            //padding += 0;
            break;
        case BC_DATUM:
            poX -= cwidth/2;
            poY -= cheight;
            padding += 1;
            break;
        case BR_DATUM:
            poX -= cwidth;
            poY -= cheight;
            padding += 2;
            break;
        case L_BASELINE:
            poY -= baseline;
            padding += 0;
            break;
        case C_BASELINE:
            poX -= cwidth/2;
            poY -= baseline;
            padding += 1;
            break;
        case R_BASELINE:
            poX -= cwidth;
            poY -= baseline;
            padding += 2;
            break;
        default:
            break;
    }

    if (host->bg_color != host->color) {
        const char* st = string;
        cheight = glyph_ab + glyph_bb;
        int8_t xo = 0;
        while (*st) {
            if (*st >= gfxFont->first && *st <= gfxFont->last) {
                GFXglyph *glyph = gfxFont->glyph + *st - gfxFont->first;
                xo = glyph->xOffset;
                if (xo > 0) {
                    xo = 0;
                } else {
                    cwidth -= xo;
                }
                host->fillRect(host, poX + xo, poY - glyph_ab, cwidth, cheight, host->bg_color);
                break ;
            }
            st += 1;
        }
        padding -=100;
    }

    while (*string) {
        sumX += host->drawChar(host, poX + sumX, poY, *string);
        string += 1;
    }

    return sumX;
}

static void setPrintArea(tft_host_t* host, int16_t x1, int16_t y1, int16_t x2, int16_t y2) {
    host->text_cursor.x1 = x1;
    host->text_cursor.y1 = y1;
    host->text_cursor.x2 = x2;
    host->text_cursor.y2 = y2;
}

static void gfxPrint(tft_host_t* host, const char* string) {
    const char* st = string;
    text_cursor_t* cursor = &host->text_cursor;
    
    if (cursor->y > host->_hight) {
        return ;
    }
    
    if (cursor->x < host->text_cursor.x1) {
        cursor->x = host->text_cursor.x1;
    }

    while(*st) {
        if (*st == '\n') {
            if (host->color != host->bg_color) {
                host->fillRect(host, cursor->x, cursor->y, cursor->x2 - cursor->x, gfxFont->yAdvance, host->bg_color);
            }
            cursor->x = cursor->x1;
            cursor->y += gfxFont->yAdvance;
            if (cursor->y > host->_hight) {
                return ;
            }
        } else if (*st >= gfxFont->first && *st <= gfxFont->last) {
            GFXglyph *glyph = &gfxFont->glyph[*st - gfxFont->first];
            if(glyph->xAdvance + cursor->x > cursor->x2 ) {
                // clear bg color to x2 
                if (host->color != host->bg_color) {
                    host->fillRect(host, cursor->x, cursor->y, cursor->x2 - cursor->x, gfxFont->yAdvance, host->bg_color);
                }
                cursor->x = cursor->x1;
                cursor->y += gfxFont->yAdvance;
                if (cursor->y > host->_hight) {
                    return ;
                }
            }

            // clear bg color
            if (host->color != host->bg_color) {
                host->fillRect(host, cursor->x, cursor->y, glyph->xAdvance, gfxFont->yAdvance, host->bg_color);
            }

            host->drawBitmap(host, cursor->x + glyph->xOffset, cursor->y + glyph->yOffset + glyph_ab, glyph->width, glyph->height, 
                        gfxFont->bitmap + glyph->bitmapOffset, host->color, host->color);
            cursor->x += glyph->xAdvance;
        }
        st += 1;
    }
}

static void setTextDatum(tft_host_t *host, uint8_t datum) {
    textdatum = datum;
}

static void selectFont(tft_host_t *host, const void *font_in) {
    uint32_t font_num = (uint32_t)font_in;
    const GFXfont* font;

    if (font_num < FONT_MAX) {
        font = font_map[font_num];
    } else {
        font = (GFXfont*)font_in;
    }

    if (font != gfxFont) {
        glyph_ab = 0;
        glyph_bb = 0;
        uint16_t numChars = font->last - font->first;

        // Find the biggest above and below baseline offsets
        for (uint8_t c = 0; c < numChars; c++) {
            GFXglyph *glyph1 = &font->glyph[c];
            int8_t ab = 0 - glyph1->yOffset;
            if (ab > glyph_ab) {
                glyph_ab = ab;
            }
            int8_t bb = glyph1->height - ab;
            if (bb > glyph_bb) {
                glyph_bb = bb;
            }
        }
    }

    gfxFont = font;
}

void LcdHostInitFontGfx(tft_host_t *host) {
    font_map[FONT_DEJAVUSANS_14] = &DejaVuSans14pt7b;
    font_map[FONT_DEJAVUSANS_16] = &DejaVuSans16pt7b;
    font_map[FONT_DEJAVUSANS_18] = &DejaVuSans18pt7b;
    font_map[FONT_DEJAVUSANS_24] = &DejaVuSans24pt7b;
    font_map[FONT_DEJAVUSANS_26] = &DejaVuSans26pt7b;
    font_map[FONT_DEJAVUSANS_40] = &DejaVuSans40pt7b;

    host->setTextFont = selectFont;
    host->drawChar = drawChar;
    host->drawString = drawString;
    host->getStringHight = getStringHight;
    host->getStringWidth = getStringWidth;
    host->setTextDatum = setTextDatum;
    host->print = gfxPrint;
    host->setPrintArea = setPrintArea;

    if (gfxFont == 0) {
        host->setTextFont(host, (void *)FONT_DEJAVUSANS_16);
        host->bg_color = TFT_BLACK;
        host->color = TFT_BLACK;
    }
}
