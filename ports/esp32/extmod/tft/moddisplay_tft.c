#include "sdkconfig.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <sys/time.h>

#include "py/mperrno.h"
#include "py/runtime.h"
#include "py/obj.h"

#include "qrcode.h"
#include "tfthost.h"
#include "screen/tft.h"
#include "gfxfont/gfx.h"

const mp_obj_type_t display_tft_type;
mp_obj_t* setBrightness_fun = NULL;

typedef struct _display_tft_obj_t {
    mp_obj_base_t base;
    tft_host_t* tft_host;

    mp_obj_t setBrightness;
} display_tft_obj_t;

static void setBrightness(tft_host_t* host, uint8_t brightness) {
    if (setBrightness_fun != NULL) {
        mp_call_function_1(*setBrightness_fun, mp_obj_new_int(brightness));
    }
}

//-----------------------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_setBrightness,    MP_ARG_OBJ, {.u_obj = mp_const_none} },
    };

    // parse args
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    display_tft_obj_t *self = m_new_obj(display_tft_obj_t);
    self->base.type = &display_tft_type;
    if (args[0].u_obj != mp_const_none) {
        self->setBrightness = args[0].u_obj;
        setBrightness_fun = &self->setBrightness;
    }
    // self->tft_host = coreScreenInit();
    self->tft_host = core2ScreenInit(setBrightness);
    return MP_OBJ_FROM_PTR(self);
}

//-----------------------------------------------------------------------------------------------
STATIC void display_tft_printinfo(const mp_print_t *print, mp_obj_t self_in, mp_print_kind_t kind)
{
    mp_printf(print, "TFT   (Color mode: 16-bit, Clk=40000000 Hz)\n");
}

//--------------------------------------
#define intToColor(xx) (((((xx) >> 19) & 0x1f) << 11) | ((((xx) >> 10) & 0x3f) << 5) | (((xx) >> 3) & 0x1f))

//-------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawPixel(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,               MP_ARG_INT, { .u_int = -1 } },
    };

	mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    uint32_t color = host->bg_color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
    if (args[2].u_int >= 0) {
    	color = intToColor(args[2].u_int);
    }
    self->tft_host->drawPixel(self->tft_host, x, y, color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawPixel_obj, 2, display_tft_drawPixel);

//------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawLine(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_x1,    MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y1,    MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                   MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x0 = args[0].u_int;
    mp_int_t y0 = args[1].u_int;
	mp_int_t x1 = args[2].u_int;
    mp_int_t y1 = args[3].u_int;
    if (args[4].u_int >= 0) {
    	color = intToColor(args[4].u_int);
    }
    host->drawLine(host, x0, y0, x1, y1, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawLine_obj, 4, display_tft_drawLine);

//-------------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawLineByAngle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_start,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_length, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_angle,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
	mp_int_t start = args[2].u_int;
    mp_int_t len = args[3].u_int;
    mp_int_t angle = args[4].u_int;
    if (args[5].u_int >= 0) {
    	color = intToColor(args[5].u_int);
    }
    
    host->drawLineByAngle(host, x, y, start, len, angle, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawLineByAngle_obj, 5, display_tft_drawLineByAngle);

//----------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawTriangle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_x1,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y1,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_x2,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y2,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_fillcolor,                MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x0 = args[0].u_int;
    mp_int_t y0 = args[1].u_int;
	mp_int_t x1 = args[2].u_int;
    mp_int_t y1 = args[3].u_int;
	mp_int_t x2 = args[4].u_int;
    mp_int_t y2 = args[5].u_int;
    if (args[6].u_int >= 0) {
    	color = intToColor(args[6].u_int);
    }
    if (args[7].u_int >= 0) {
        host->fillTriangle(host, x0, y0, x1, y1, x2, y2, intToColor(args[7].u_int));
    }
    host->drawTriangle(host, x0, y0, x1, y1, x2, y2, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawTriangle_obj, 6, display_tft_drawTriangle);

//----------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_fillTriangle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_x1,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y1,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_x2,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y2,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x0 = args[0].u_int;
    mp_int_t y0 = args[1].u_int;
	mp_int_t x1 = args[2].u_int;
    mp_int_t y1 = args[3].u_int;
	mp_int_t x2 = args[4].u_int;
    mp_int_t y2 = args[5].u_int;
    if (args[6].u_int >= 0) {
    	color = intToColor(args[6].u_int);
    }
    host->fillTriangle(host, x0, y0, x1, y1, x2, y2, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_fillTriangle_obj, 6, display_tft_fillTriangle);

//--------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawCircle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_r,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_fillcolor,                MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
    mp_int_t radius = args[2].u_int;
    if (args[3].u_int >= 0) {
    	color = intToColor(args[3].u_int);
    }
    if (args[4].u_int >= 0) {
        host->fillCircle(host, x, y, radius, intToColor(args[4].u_int));
        if (args[3].u_int != args[4].u_int) host->drawCircle(host, x, y, radius, color);
    }
    else host->drawCircle(host, x, y, radius, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawCircle_obj, 3, display_tft_drawCircle);

//--------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_fillCircle(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_r,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
    mp_int_t radius = args[2].u_int;
    if (args[3].u_int >= 0) {
    	color = intToColor(args[3].u_int);
    }
    host->fillCircle(host, x, y, radius, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_fillCircle_obj, 3, display_tft_fillCircle);

//---------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawEllipse(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_rx,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_ry,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_opt,                      MP_ARG_INT, { .u_int = 15 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_fillcolor,                MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
	mp_int_t rx = args[2].u_int;
    mp_int_t ry = args[3].u_int;
    mp_int_t opt = args[4].u_int & 0x0F;
    if (args[5].u_int >= 0) {
    	color = intToColor(args[5].u_int);
    }
    if (args[6].u_int >= 0) {
        host->fillEllipse(host, x, y, rx, ry, intToColor(args[6].u_int), opt);
    }
    host->drawEllipse(host, x, y, rx, ry, color, opt);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawEllipse_obj, 4, display_tft_drawEllipse);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawArc(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_r,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_thick,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_start,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_end,    MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 15 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_fillcolor,                MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
    uint32_t fill_color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
    mp_int_t r = args[2].u_int;
	mp_int_t th = args[3].u_int;
    mp_int_t start = args[4].u_int;
    mp_int_t end = args[5].u_int;
    if (args[6].u_int >= 0) {
    	color = intToColor(args[6].u_int);
    }
    if (args[7].u_int >= 0) {
    	fill_color = intToColor(args[7].u_int);
    }
    host->drawArc(host, x, y, r, th, start, end, color, fill_color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawArc_obj, 6, display_tft_drawArc);

//------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawRect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_width,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_fillcolor,                MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
	mp_int_t w = args[2].u_int;
    mp_int_t h = args[3].u_int;
    if (args[4].u_int >= 0) {
    	color = intToColor(args[4].u_int);
    }
    if (args[5].u_int >= 0) {
        host->fillRect(host, x, y, w, h, intToColor(args[5].u_int));
    }
    host->drawRect(host, x, y, w, h, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawRect_obj, 4, display_tft_drawRect);

//------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_fillRect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_width,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
	mp_int_t w = args[2].u_int;
    mp_int_t h = args[3].u_int;
    uint32_t color = host->color;
    if (args[4].u_int >= 0) {
    	color = intToColor(args[4].u_int);
    }
     host->fillRect(host, x, y, w, h, color);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_fillRect_obj, 4, display_tft_fillRect);
//------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_qrcode(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_text,     MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
        { MP_QSTR_x,        MP_ARG_INT, { .u_int = 50 } },
        { MP_QSTR_y,        MP_ARG_INT, { .u_int = 10 } },
        { MP_QSTR_width,    MP_ARG_INT, { .u_int = 220 } },
        { MP_QSTR_version,  MP_ARG_INT, { .u_int = 6 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    char *text = (char *)mp_obj_str_get_str(args[0].u_obj);
	mp_int_t x = args[1].u_int;
    mp_int_t y = args[2].u_int;
	mp_int_t width = args[3].u_int;
	mp_int_t version = args[4].u_int;

    // Create the QR code
    QRCode qrcode;
    uint8_t qrcodeData[qrcode_getBufferSize(version)];
    qrcode_initText(&qrcode, qrcodeData, version, 0, text);
    
    // Top quiet zone
    uint8_t thickness = width / qrcode.size;
    uint16_t lineLength = qrcode.size * thickness;
    uint8_t xOffset = x + (width-lineLength)/2;
    uint8_t yOffset = y + (width-lineLength)/2;
    host->fillRect(host, x, y, width, width, TFT_WHITE);

    for (uint8_t y = 0; y < qrcode.size; y++) {
        for (uint8_t x = 0; x < qrcode.size; x++) {
            uint8_t q = qrcode_getModule(&qrcode, x, y);
            if (q) {
                host->fillRect(host, x * thickness + xOffset, y * thickness + yOffset, thickness, thickness, TFT_BLACK);
            }
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_qrcode_obj, 1, display_tft_qrcode);

//-----------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_drawRoundRect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_width,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_r,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_fillcolor,                MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
	mp_int_t w = args[2].u_int;
    mp_int_t h = args[3].u_int;
    mp_int_t r = args[4].u_int;
    if (args[5].u_int >= 0) {
    	color = intToColor(args[5].u_int);
    }
    if (args[6].u_int >= 0) {
        host->fillRoundRect(host, x, y, w, h, r, intToColor(args[6].u_int));
    }
    host->drawRoundRect(host, x, y, w, h, r, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_drawRoundRect_obj, 5, display_tft_drawRoundRect);

//-----------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_fillRoundRect(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_width,  MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_height, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_r,      MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color = host->color;
	mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;
	mp_int_t w = args[2].u_int;
    mp_int_t h = args[3].u_int;
    mp_int_t r = args[4].u_int;
    if (args[5].u_int >= 0) {
    	color = intToColor(args[5].u_int);
    }
    host->fillRoundRect(host, x, y, w, h, r, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_fillRoundRect_obj, 5, display_tft_fillRoundRect);

//--------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_fillScreen(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t color;
    if (args[0].u_int >= 0) {
    	color = intToColor(args[0].u_int);
    } else {
        color = TFT_BLACK;
    }
    host->fillScreen(host, color);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_fillScreen_obj, 0, display_tft_fillScreen);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_setFont(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;

    const mp_arg_t allowed_args[] = {
        { MP_QSTR_font,         MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    mp_int_t font = mp_obj_get_int(args[0].u_obj);
    host->setTextFont(host, (const void *)font);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_setFont_obj, 1, display_tft_setFont);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_setRot(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;

    const mp_arg_t allowed_args[] = {
        { MP_QSTR_rot, MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = PORTRAIT } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

  	mp_int_t rot = args[0].u_int;
  	if ((rot < 0) || (rot > 3)) rot = 0;

  	host->setRotation(host, rot);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_setRot_obj, 1, display_tft_setRot);

//---------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_text(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_text,         MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
        { MP_QSTR_color,                          MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_bgcolor,      MP_ARG_KW_ONLY  | MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_textdatum,    MP_ARG_KW_ONLY  | MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t old_fg = host->color;
    uint32_t old_bg = host->bg_color;

    mp_int_t x = args[0].u_int;
    mp_int_t y = args[1].u_int;

    if (args[3].u_int >= 0) host->color =  intToColor(args[3].u_int);
    if (args[4].u_int >= 0) host->bg_color = intToColor(args[4].u_int);
    if (args[5].u_int >= 0) host->setTextDatum(host, args[5].u_int);

    char *st = (char *)mp_obj_str_get_str(args[2].u_obj);
    host->drawString(host, x, y, st);
    
    host->color =  old_fg;
    host->bg_color = old_bg;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_text_obj, 3, display_tft_text);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_setTextDatum(mp_obj_t self_in, mp_obj_t datum_in)
{
    display_tft_obj_t *self = (display_tft_obj_t *)self_in;
    tft_host_t* host = self->tft_host;

    host->setTextDatum(host, mp_obj_get_int(datum_in));
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(display_tft_setTextDatum_obj, display_tft_setTextDatum);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_setCursor(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    host->text_cursor.x = args[0].u_int;
    host->text_cursor.y = args[1].u_int;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_setCursor_obj, 2, display_tft_setCursor);

// //-------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_getCursor(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    mp_obj_t tuple[2];

    tuple[0] = mp_obj_new_int(host->text_cursor.x);
    tuple[1] = mp_obj_new_int(host->text_cursor.y);

    return mp_obj_new_tuple(2, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_getCursor_obj, 0, display_tft_getCursor);

STATIC mp_obj_t display_tft_setPrintArea(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_x1,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y1,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_x2,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y2,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    host->setPrintArea(host, args[0].u_int, args[1].u_int, args[2].u_int, args[3].u_int);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_setPrintArea_obj, 4, display_tft_setPrintArea);

// //---------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_print(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_text,         MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
        { MP_QSTR_x,                              MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_y,                              MP_ARG_INT, { .u_int = 0 } },
        { MP_QSTR_color,                          MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_bg_color,                       MP_ARG_INT, { .u_int = -1 } },
    };
    
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    uint32_t old_fg = host->color;
    uint32_t old_bg = host->bg_color;
    
    if (n_args > 2) {
        host->text_cursor.x = args[1].u_int;
        host->text_cursor.y = args[2].u_int;
    }

    if (args[3].u_int >= 0) {
    	host->color =  intToColor(args[3].u_int);
    }
    
    if (args[4].u_int >= 0) {
    	host->bg_color =  intToColor(args[4].u_int);
    }

    char* st;
    size_t len;

    if (MP_OBJ_IS_STR(args[0].u_obj)) {
        st = (char *)mp_obj_str_get_data(args[0].u_obj, &len);
    } else {
        char str[32];
        if (mp_obj_is_integer(args[0].u_obj)) {
            int num = mp_obj_get_int(args[0].u_obj);
            sprintf(str, "%d", num);
        } else if (mp_obj_is_float(args[0].u_obj)) {
            float f = mp_obj_float_get(args[0].u_obj);
            sprintf(str, "%f", f);
        }
        st = str;
        len = strlen(st);
    }

    host->print(host, st);

    host->color = old_fg;
    host->bg_color = old_bg;

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_print_obj, 1, display_tft_print);

// ---------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_println(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    display_tft_print(n_args, pos_args, kw_args);
    host->print(host, "\r\n");
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_println_obj, 1, display_tft_println);

//---------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_stringWidth(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_text,  MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_int_t w = 0;

    char *st = (char *)mp_obj_str_get_str(args[0].u_obj);
    w = host->getStringWidth(host, st);

    return mp_obj_new_int(w);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_stringWidth_obj, 1, display_tft_stringWidth);

STATIC mp_obj_t display_tft_stringHight(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
  display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_text,  MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
    };

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    mp_int_t w = 0;

    char *st = (char *)mp_obj_str_get_str(args[0].u_obj);
    w = host->getStringHight(host, st);

    return mp_obj_new_int(w);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_stringHight_obj, 1, display_tft_stringHight);

STATIC mp_obj_t display_tft_Image_buff(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args) 
{
    // const mp_arg_t allowed_args[] = {
	// 	{ MP_QSTR_x,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
	// 	{ MP_QSTR_y,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
    //     { MP_QSTR_file,  MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
    //     { MP_QSTR_scale,                   MP_ARG_INT, { .u_int = 0 } },
    //     { MP_QSTR_type,                    MP_ARG_INT, { .u_int = -1 } },
    //     { MP_QSTR_debug, MP_ARG_KW_ONLY  | MP_ARG_INT, { .u_int = 0 } },
    // };
    
    // mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    // mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    // image_debug = (uint8_t)args[5].u_bool;
    // mp_buffer_info_t bufinfo;
    // mp_get_buffer_raise(args[2].u_obj, &bufinfo, MP_BUFFER_READ);
    
    // if (bufinfo.len < 20) {
    //     return mp_const_none;
    // }

    // uint8_t* buf_ptr = (uint8_t *)bufinfo.buf;
    // if (buf_ptr[0] == 0xff && buf_ptr[1] == 0xd8 && buf_ptr[6] == 'J' && buf_ptr[7] == 'F') {
    //     host->jpg_image(host, args[0].u_int, args[1].u_int, args[3].u_int, NULL, bufinfo.buf, bufinfo.len);
    // } else if(buf_ptr[0] == 'B' && buf_ptr[1] == 'M') {
    //     host->bmp_image(host, args[0].u_int, args[1].u_int, args[3].u_int, NULL, bufinfo.buf, bufinfo.len);
    // }else if(buf_ptr[1] == 'P' && buf_ptr[2] == 'N'){
    //     host->png_image(host, args[0].u_int, args[1].u_int, args[3].u_int, NULL, bufinfo.buf, bufinfo.len);
    // }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_Image_buff_obj, 3, display_tft_Image_buff);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_Image(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    // display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    // tft_host_t* host = self->tft_host;
    // const mp_arg_t allowed_args[] = {
	// 	{ MP_QSTR_x,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
	// 	{ MP_QSTR_y,     MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
    //     { MP_QSTR_file,  MP_ARG_REQUIRED | MP_ARG_OBJ, { .u_obj = mp_const_none } },
    //     { MP_QSTR_scale,                   MP_ARG_INT, { .u_int = 0 } },
    //     { MP_QSTR_type,                    MP_ARG_INT, { .u_int = -1 } },
    //     { MP_QSTR_debug, MP_ARG_KW_ONLY  | MP_ARG_INT, { .u_int = 0 } },
    // };
    
    // mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    // mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);
    
    // if(!mp_obj_is_str(args[2].u_obj)) {
    //     return display_tft_Image_buff(n_args, pos_args, kw_args);
    // }

    // char *fname = NULL;
    // char fullname[128] = {'\0'};
    // int img_type = args[4].u_int;
    
    // fname = (char *)mp_obj_str_get_str(args[2].u_obj);

    // int res = 0; //physicalPath(fname, fullname);
    // strcpy(fullname, fname);
    // if ((res != 0) || (strlen(fullname) == 0)) {
    //     mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("Error resolving file name"));
    // }

    // if (img_type < 0) {
    // 	// try to determine image type
    //     char upr_fname[128];
    //     strcpy(upr_fname, fname);
    //     for (int i=0; i < strlen(upr_fname); i++) {
    //       upr_fname[i] = toupper((unsigned char) upr_fname[i]);
    //     }
    //     if (strstr(upr_fname, ".JPG") != NULL) img_type = IMAGE_TYPE_JPG;
    //     else if (strstr(upr_fname, ".BMP") != NULL) img_type = IMAGE_TYPE_BMP;
    //     else if (strstr(upr_fname, ".PNG") != NULL) img_type = IMAGE_TYPE_PNG;
    //     if (img_type < 0) {
    //     	mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("Cannot determine image type"));
    //     }
    // }

    // image_debug = (uint8_t)args[5].u_bool;
    // if (img_type == IMAGE_TYPE_BMP) {
    // 	host->bmp_image(host, args[0].u_int, args[1].u_int, args[3].u_int, fullname, NULL, 0);
    // }
    // else if (img_type == IMAGE_TYPE_JPG) {
    // 	host->jpg_image(host, args[0].u_int, args[1].u_int, args[3].u_int, fullname, NULL, 0);
    // }
    // else if (img_type == IMAGE_TYPE_PNG)
    // {
    //     host->png_image(host, args[0].u_int, args[1].u_int, args[3].u_int, fullname, NULL, 0);
    // }
    // else {
    //     mp_raise_msg_varg(&mp_type_OSError, MP_ERROR_TEXT("Unsupported image type"));
    // }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_Image_obj, 3, display_tft_Image);

//------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_getSize(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    mp_obj_t tuple[2];
    tuple[0] = mp_obj_new_int(host->_width);
    tuple[1] = mp_obj_new_int(host->_hight);

    return mp_obj_new_tuple(2, tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_getSize_obj, 0, display_tft_getSize);

//-------------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_setColor(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_color,                    MP_ARG_INT, { .u_int = -1 } },
        { MP_QSTR_bcolor,                   MP_ARG_INT, { .u_int = -1 } },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    if (args[0].u_int >= 0) {
    	host->color =  intToColor(args[0].u_int);
    } else {
        host->color =  TFT_WHITE;
    }

    if (args[1].u_int >= 0) {
    	host->bg_color = intToColor(args[1].u_int);
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_setColor_obj, 0, display_tft_setColor);

//-----------------------------------------------------------------------------------------------
STATIC mp_obj_t display_tft_setBrightness(size_t n_args, const mp_obj_t *pos_args, mp_map_t *kw_args)
{
    display_tft_obj_t *self = MP_OBJ_TO_PTR(pos_args[0]);
    tft_host_t* host = self->tft_host;
    const mp_arg_t allowed_args[] = {
        { MP_QSTR_duty,            MP_ARG_REQUIRED | MP_ARG_INT, { .u_int = 0 } },
    };
    

    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all(n_args - 1, pos_args + 1, kw_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

  	mp_int_t dperc = args[0].u_int;
    host->setBrightness(host, dperc);

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_KW(display_tft_setBrightness_obj, 0, display_tft_setBrightness);

// //--------------------------------------------------
// STATIC mp_obj_t display_tft_get_bg(mp_obj_t self_in)
// {
//     int icolor = (int)((_bg.r << 16) | (_bg.g << 8) | _bg.b);
//     return mp_obj_new_int(icolor);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(display_tft_get_bg_obj, display_tft_get_bg);

// //--------------------------------------------------
// STATIC mp_obj_t display_tft_get_fg(mp_obj_t self_in)
// {
//     int icolor = (int)((host->color.r << 16) | (host->color.g << 8) | host->color.b);
//     return mp_obj_new_int(icolor);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(display_tft_get_fg_obj, display_tft_get_fg);

//---------------------------------------------------------------------
STATIC mp_obj_t display_tft_set_bg(mp_obj_t self_in, mp_obj_t color_in)
{
    display_tft_obj_t *self = (display_tft_obj_t *)self_in;
    tft_host_t* host = self->tft_host;
    uint32_t color = intToColor(mp_obj_get_int(color_in));
    host->bg_color = color;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(display_tft_set_bg_obj, display_tft_set_bg);

//---------------------------------------------------------------------
STATIC mp_obj_t display_tft_set_fg(mp_obj_t self_in, mp_obj_t color_in)
{
    display_tft_obj_t *self = (display_tft_obj_t *)self_in;
    tft_host_t* host = self->tft_host;
    uint32_t color = intToColor(mp_obj_get_int(color_in));
    host->color =  color;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(display_tft_set_fg_obj, display_tft_set_fg);

// //-------------------------------------------------
// STATIC mp_obj_t display_tft_get_X(mp_obj_t self_in)
// {
// 	int x = TFT_X;
//     return mp_obj_new_int(x);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(display_tft_get_X_obj, display_tft_get_X);

// //-------------------------------------------------
// STATIC mp_obj_t display_tft_get_Y(mp_obj_t self_in)
// {
// 	int y = TFT_Y;
//     return mp_obj_new_int(y);
// }
// STATIC MP_DEFINE_CONST_FUN_OBJ_1(display_tft_get_Y_obj, display_tft_get_Y);


//================================================================
STATIC const mp_rom_map_elem_t display_tft_locals_dict_table[] = {
    // instance methods
    // { MP_ROM_QSTR(MP_QSTR_readPixel),			MP_ROM_PTR(&display_tft_readPixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_pixel),				MP_ROM_PTR(&display_tft_drawPixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_line),				MP_ROM_PTR(&display_tft_drawLine_obj) },
    { MP_ROM_QSTR(MP_QSTR_lineByAngle),			MP_ROM_PTR(&display_tft_drawLineByAngle_obj) },
    { MP_ROM_QSTR(MP_QSTR_triangle),			MP_ROM_PTR(&display_tft_drawTriangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_circle),				MP_ROM_PTR(&display_tft_drawCircle_obj) },
    { MP_ROM_QSTR(MP_QSTR_ellipse),				MP_ROM_PTR(&display_tft_drawEllipse_obj) },
    { MP_ROM_QSTR(MP_QSTR_arc),					MP_ROM_PTR(&display_tft_drawArc_obj) },
    // { MP_ROM_QSTR(MP_QSTR_polygon),				MP_ROM_PTR(&display_tft_drawPoly_obj) },
    { MP_ROM_QSTR(MP_QSTR_rect),				MP_ROM_PTR(&display_tft_drawRect_obj) },
    // { MP_ROM_QSTR(MP_QSTR_readScreen),			MP_ROM_PTR(&display_tft_readScreen_obj) },
    { MP_ROM_QSTR(MP_QSTR_roundrect),			MP_ROM_PTR(&display_tft_drawRoundRect_obj) },
    { MP_ROM_QSTR(MP_QSTR_clear),				MP_ROM_PTR(&display_tft_fillScreen_obj) },
    { MP_ROM_QSTR(MP_QSTR_fill),				MP_ROM_PTR(&display_tft_fillScreen_obj) },
    // { MP_ROM_QSTR(MP_QSTR_clearwin),			MP_ROM_PTR(&display_tft_fillWin_obj) },
    { MP_ROM_QSTR(MP_QSTR_font),				MP_ROM_PTR(&display_tft_setFont_obj) },
    // { MP_ROM_QSTR(MP_QSTR_fontSize),			MP_ROM_PTR(&display_tft_getFontSize_obj) },
    { MP_ROM_QSTR(MP_QSTR_text),				MP_ROM_PTR(&display_tft_text_obj) },
    { MP_ROM_QSTR(MP_QSTR_orient),				MP_ROM_PTR(&display_tft_setRot_obj) },
    { MP_ROM_QSTR(MP_QSTR_textWidth),			MP_ROM_PTR(&display_tft_stringWidth_obj) },
    { MP_ROM_QSTR(MP_QSTR_fontHight),			MP_ROM_PTR(&display_tft_stringHight_obj) },
    // { MP_ROM_QSTR(MP_QSTR_textClear),			MP_ROM_PTR(&display_tft_clearStringRect_obj) },
    // { MP_ROM_QSTR(MP_QSTR_attrib7seg),			MP_ROM_PTR(&display_tft_7segAttrib_obj) },
    { MP_ROM_QSTR(MP_QSTR_image),				MP_ROM_PTR(&display_tft_Image_obj) },
    { MP_ROM_QSTR(MP_QSTR_image_buff),			MP_ROM_PTR(&display_tft_Image_buff_obj) },
    // { MP_ROM_QSTR(MP_QSTR_gettouch),			MP_ROM_PTR(&display_tft_getTouch_obj) },
    // { MP_ROM_QSTR(MP_QSTR_compileFont),			MP_ROM_PTR(&display_tft_compileFont_obj) },
    // { MP_ROM_QSTR(MP_QSTR_hsb2rgb),				MP_ROM_PTR(&display_tft_HSBtoRGB_obj) },
    // { MP_ROM_QSTR(MP_QSTR_setwin),				MP_ROM_PTR(&display_tft_setclipwin_obj) },
    // { MP_ROM_QSTR(MP_QSTR_resetwin),			MP_ROM_PTR(&display_tft_resetclipwin_obj) },
    // { MP_ROM_QSTR(MP_QSTR_savewin),				MP_ROM_PTR(&display_tft_saveclipwin_obj) },
    // { MP_ROM_QSTR(MP_QSTR_restorewin),			MP_ROM_PTR(&display_tft_restoreclipwin_obj) },
    { MP_ROM_QSTR(MP_QSTR_screensize),			MP_ROM_PTR(&display_tft_getSize_obj) },
    // { MP_ROM_QSTR(MP_QSTR_winsize),				MP_ROM_PTR(&display_tft_getWinSize_obj) },
    // { MP_ROM_QSTR(MP_QSTR_get_fg),				MP_ROM_PTR(&display_tft_get_fg_obj) },
    // { MP_ROM_QSTR(MP_QSTR_get_bg),				MP_ROM_PTR(&display_tft_get_bg_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_fg),				MP_ROM_PTR(&display_tft_set_fg_obj) },
    { MP_ROM_QSTR(MP_QSTR_set_bg),				MP_ROM_PTR(&display_tft_set_bg_obj) },
    // { MP_ROM_QSTR(MP_QSTR_text_x),				MP_ROM_PTR(&display_tft_get_X_obj) },
    // { MP_ROM_QSTR(MP_QSTR_text_y),				MP_ROM_PTR(&display_tft_get_Y_obj) },
    { MP_ROM_QSTR(MP_QSTR_setColor),            MP_ROM_PTR(&display_tft_setColor_obj) },

    // Adafruit API
    { MP_ROM_QSTR(MP_QSTR_print),               MP_ROM_PTR(&display_tft_print_obj) },
    { MP_ROM_QSTR(MP_QSTR_println),             MP_ROM_PTR(&display_tft_println_obj) },
    { MP_ROM_QSTR(MP_QSTR_setRotation),         MP_ROM_PTR(&display_tft_setRot_obj) },
    { MP_ROM_QSTR(MP_QSTR_setTextDatum),        MP_ROM_PTR(&display_tft_setTextDatum_obj) },
    { MP_ROM_QSTR(MP_QSTR_setPrintArea),        MP_ROM_PTR(&display_tft_setPrintArea_obj) },

    { MP_ROM_QSTR(MP_QSTR_setCursor),           MP_ROM_PTR(&display_tft_setCursor_obj) },
    { MP_ROM_QSTR(MP_QSTR_getCursor),           MP_ROM_PTR(&display_tft_getCursor_obj) },

    { MP_ROM_QSTR(MP_QSTR_fillScreen),			MP_ROM_PTR(&display_tft_fillScreen_obj) },
    { MP_ROM_QSTR(MP_QSTR_drawPixel),			MP_ROM_PTR(&display_tft_drawPixel_obj) },
    { MP_ROM_QSTR(MP_QSTR_drawLine),			MP_ROM_PTR(&display_tft_drawLine_obj) },
    { MP_ROM_QSTR(MP_QSTR_drawRect),		    MP_ROM_PTR(&display_tft_drawRect_obj) },
    { MP_ROM_QSTR(MP_QSTR_fillRect),		    MP_ROM_PTR(&display_tft_fillRect_obj) },
    { MP_ROM_QSTR(MP_QSTR_drawCircle),		    MP_ROM_PTR(&display_tft_drawCircle_obj) },
    { MP_ROM_QSTR(MP_QSTR_fillCircle),		    MP_ROM_PTR(&display_tft_fillCircle_obj) },
    // { MP_ROM_QSTR(MP_QSTR_drawSwitchBtn),	    MP_ROM_PTR(&display_tft_drawSwitchBtn_obj) },
    { MP_ROM_QSTR(MP_QSTR_drawTriangle),		MP_ROM_PTR(&display_tft_drawTriangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_fillTriangle),		MP_ROM_PTR(&display_tft_fillTriangle_obj) },
    { MP_ROM_QSTR(MP_QSTR_drawRoundRect),		MP_ROM_PTR(&display_tft_drawRoundRect_obj) },
    { MP_ROM_QSTR(MP_QSTR_fillRoundRect),		MP_ROM_PTR(&display_tft_fillRoundRect_obj) },

    { MP_ROM_QSTR(MP_QSTR_setBrightness),       MP_ROM_PTR(&display_tft_setBrightness_obj) },
    { MP_ROM_QSTR(MP_QSTR_backlight),           MP_ROM_PTR(&display_tft_setBrightness_obj) },
    { MP_ROM_QSTR(MP_QSTR_qrcode),              MP_ROM_PTR(&display_tft_qrcode_obj) },
    
    //These enumerate the text plotting alignment (reference datum point)
    { MP_ROM_QSTR(MP_QSTR_DATUM_TL),            MP_ROM_INT(TL_DATUM)},	  // Top left (default)
    { MP_ROM_QSTR(MP_QSTR_DATUM_TC),            MP_ROM_INT(TC_DATUM)},	  // Top centre
    { MP_ROM_QSTR(MP_QSTR_DATUM_TR),            MP_ROM_INT(TR_DATUM)},	  // Top right
    { MP_ROM_QSTR(MP_QSTR_DATUM_ML),            MP_ROM_INT(ML_DATUM)},	  // Middle left
    { MP_ROM_QSTR(MP_QSTR_DATUM_CL),            MP_ROM_INT(CL_DATUM)},	  // Centre left, same as above
    { MP_ROM_QSTR(MP_QSTR_DATUM_MC),            MP_ROM_INT(MC_DATUM)},	  // Middle centre
    { MP_ROM_QSTR(MP_QSTR_DATUM_CC),            MP_ROM_INT(CC_DATUM)},	  // Centre centre, same as above
    { MP_ROM_QSTR(MP_QSTR_DATUM_MR),            MP_ROM_INT(MR_DATUM)},	  // Middle right
    { MP_ROM_QSTR(MP_QSTR_DATUM_CR),            MP_ROM_INT(CR_DATUM)},	  // Centre right, same as above
    { MP_ROM_QSTR(MP_QSTR_DATUM_BL),            MP_ROM_INT(BL_DATUM)},	  // Bottom left
    { MP_ROM_QSTR(MP_QSTR_DATUM_BC),            MP_ROM_INT(BC_DATUM)},	  // Bottom centre
    { MP_ROM_QSTR(MP_QSTR_DATUM_BR),            MP_ROM_INT(BR_DATUM)},	  // Bottom right
    { MP_ROM_QSTR(MP_QSTR_BASELINE_L),          MP_ROM_INT(L_BASELINE)},  // Left character baseline (Line the 'A' character would sit on)
    { MP_ROM_QSTR(MP_QSTR_BASELINE_C),          MP_ROM_INT(C_BASELINE)}, // Centre character baseline
    { MP_ROM_QSTR(MP_QSTR_BASELINE_R),          MP_ROM_INT(R_BASELINE)}, // Right character baseline

    { MP_ROM_QSTR(MP_QSTR_PORTRAIT),			MP_ROM_INT(PORTRAIT) },
    { MP_ROM_QSTR(MP_QSTR_LANDSCAPE),			MP_ROM_INT(LANDSCAPE) },
    { MP_ROM_QSTR(MP_QSTR_PORTRAIT_FLIP),		MP_ROM_INT(PORTRAIT_FLIP) },
    { MP_ROM_QSTR(MP_QSTR_LANDSCAPE_FLIP),		MP_ROM_INT(LANDSCAPE_FLIP) },

    { MP_ROM_QSTR(MP_QSTR_FONT_DejaVu14),		MP_ROM_INT(FONT_DEJAVUSANS_14) },
    { MP_ROM_QSTR(MP_QSTR_FONT_DejaVu16),		MP_ROM_INT(FONT_DEJAVUSANS_16) },
    { MP_ROM_QSTR(MP_QSTR_FONT_DejaVu18),		MP_ROM_INT(FONT_DEJAVUSANS_18) },
    { MP_ROM_QSTR(MP_QSTR_FONT_DejaVu24),		MP_ROM_INT(FONT_DEJAVUSANS_24) },
    { MP_ROM_QSTR(MP_QSTR_FONT_DejaVu26),		MP_ROM_INT(FONT_DEJAVUSANS_26) },
    { MP_ROM_QSTR(MP_QSTR_FONT_DejaVu40),		MP_ROM_INT(FONT_DEJAVUSANS_40) },

	{ MP_ROM_QSTR(MP_QSTR_BLACK),				MP_ROM_INT(iTFT_BLACK) },
	{ MP_ROM_QSTR(MP_QSTR_NAVY),				MP_ROM_INT(iTFT_NAVY) },
	{ MP_ROM_QSTR(MP_QSTR_DARKGREEN),			MP_ROM_INT(iTFT_DARKGREEN) },
	{ MP_ROM_QSTR(MP_QSTR_DARKCYAN),			MP_ROM_INT(iTFT_DARKCYAN) },
	{ MP_ROM_QSTR(MP_QSTR_MAROON),				MP_ROM_INT(iTFT_MAROON) },
	{ MP_ROM_QSTR(MP_QSTR_PURPLE),				MP_ROM_INT(iTFT_PURPLE) },
	{ MP_ROM_QSTR(MP_QSTR_OLIVE),				MP_ROM_INT(iTFT_OLIVE) },
	{ MP_ROM_QSTR(MP_QSTR_LIGHTGREY),			MP_ROM_INT(iTFT_LIGHTGREY) },
	{ MP_ROM_QSTR(MP_QSTR_DARKGREY),			MP_ROM_INT(iTFT_DARKGREY) },
	{ MP_ROM_QSTR(MP_QSTR_BLUE),				MP_ROM_INT(iTFT_BLUE) },
	{ MP_ROM_QSTR(MP_QSTR_GREEN),				MP_ROM_INT(iTFT_GREEN) },
	{ MP_ROM_QSTR(MP_QSTR_CYAN),				MP_ROM_INT(iTFT_CYAN) },
	{ MP_ROM_QSTR(MP_QSTR_RED),					MP_ROM_INT(iTFT_RED) },
	{ MP_ROM_QSTR(MP_QSTR_MAGENTA),				MP_ROM_INT(iTFT_MAGENTA) },
	{ MP_ROM_QSTR(MP_QSTR_YELLOW),				MP_ROM_INT(iTFT_YELLOW) },
	{ MP_ROM_QSTR(MP_QSTR_WHITE),				MP_ROM_INT(iTFT_WHITE) },
	{ MP_ROM_QSTR(MP_QSTR_ORANGE),				MP_ROM_INT(iTFT_ORANGE) },
	{ MP_ROM_QSTR(MP_QSTR_GREENYELLOW),			MP_ROM_INT(iTFT_GREENYELLOW) },
	{ MP_ROM_QSTR(MP_QSTR_PINK),				MP_ROM_INT(iTFT_PINK) },

	{ MP_ROM_QSTR(MP_QSTR_COLOR_BITS16),		MP_ROM_INT(16) },
	{ MP_ROM_QSTR(MP_QSTR_COLOR_BITS24),		MP_ROM_INT(24) },

	// { MP_ROM_QSTR(MP_QSTR_JPG),					MP_ROM_INT(IMAGE_TYPE_JPG) },
	// { MP_ROM_QSTR(MP_QSTR_BMP),					MP_ROM_INT(IMAGE_TYPE_BMP) },

	// { MP_ROM_QSTR(MP_QSTR_HSPI),				MP_ROM_INT(HSPI_HOST) },
	// { MP_ROM_QSTR(MP_QSTR_VSPI),				MP_ROM_INT(VSPI_HOST) },
};
STATIC MP_DEFINE_CONST_DICT(display_tft_locals_dict, display_tft_locals_dict_table);

//======================================
const mp_obj_type_t display_tft_type = {
    { &mp_type_type },
    .name = MP_QSTR_TFT,
    .print = display_tft_printinfo,
    .make_new = display_tft_make_new,
    .locals_dict = (mp_obj_t)&display_tft_locals_dict,
};