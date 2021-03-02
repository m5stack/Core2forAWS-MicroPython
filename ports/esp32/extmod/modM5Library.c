#include "py/obj.h"

extern const mp_obj_type_t display_tft_type;
extern const mp_obj_type_t neopixel_type;

//===============================================================
STATIC const mp_rom_map_elem_t m5_library_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_ROM_QSTR(MP_QSTR_M5Library) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_TFT),         MP_ROM_PTR(&display_tft_type) },
    { MP_OBJ_NEW_QSTR(MP_QSTR_Neopixel),    MP_ROM_PTR(&neopixel_type) },
};

//===============================================================================
STATIC MP_DEFINE_CONST_DICT(m5_library_module_globals, m5_library_module_globals_table);

const mp_obj_module_t mp_module_M5Library = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&m5_library_module_globals,
};

MP_REGISTER_MODULE(MP_QSTR_M5Library, mp_module_M5Library, 1);
