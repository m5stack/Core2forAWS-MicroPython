#include <stdio.h>
#include "stdatomic.h"

#include "py/runtime.h"
#include "py/mphal.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "py/mperrno.h"
#include "mphalport.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "esp_err.h"
#include "esp_log.h"

#include "esp_task.h"
#include "i2c_bus/i2c_device.h"

#define TOUCH_INTR_PIN 39
#define TOUCH_ADDR 0x38

STATIC const char TAG[] = "[touch]";

// touch -> true  
// read -> false
// distouch -> 20ms -> false
typedef struct _touch_area_info_t {
    uint16_t x1;
    uint16_t y1;
    uint16_t x2;
    uint16_t y2;
    atomic_bool status;
    atomic_bool pressed;
    atomic_uint_fast32_t touch_time;
    // touch_area_info_t* -> next
    atomic_uint_fast32_t next;
    /* data */
} touch_area_info_t;

typedef struct _touch_area_t {
    mp_obj_base_t base;
    // touch_area_info_t* -> touch_area_info
    atomic_uint_fast32_t touch_area_info;
} touch_area_t;

typedef struct _touch_info_t {
    atomic_short x;
    atomic_short y;
    atomic_short area;
    atomic_bool touched;
} touch_info_t;

static touch_info_t touch_info;
static xTaskHandle touch_task_handle;
static I2CDevice_t i2c_device;

typedef struct _touch_t {
    mp_obj_base_t base;
    i2c_port_t i2c_num;
    uint8_t    i2c_addr;
} touch_t;

typedef struct _ft6336_hw_i2c_obj_t {
    mp_obj_base_t base;
    uint8_t pos;
    uint8_t port;
} ft6336_hw_i2c_obj_t;

STATIC void IRAM_ATTR touch_isr_handler(void* arg);
STATIC void touch_task(void* arg);

// ============================================= touch area =========================================================================
static touch_area_info_t touch_area_ahead;
static touch_area_info_t* create_new_touch_area(int16_t x, int16_t y, int16_t w, int16_t h) {
    touch_area_info_t* touch_area = (touch_area_info_t*)malloc(sizeof(touch_area_info_t));
    touch_area->x1 = x;
    touch_area->y1 = y;
    touch_area->x2 = x + w;
    touch_area->y2 = y + h;
    atomic_store(&touch_area->touch_time, 0);
    atomic_store(&touch_area->next, 0);
    atomic_store(&touch_area->status, false);
    atomic_store(&touch_area->pressed, false);

    touch_area_info_t* touch_area_end = &touch_area_ahead;
    while ((touch_area_info_t*)atomic_load(&touch_area_end->next) != 0) {
        touch_area_end = (touch_area_info_t*)atomic_load(&touch_area_end->next);
    }
    atomic_store(&touch_area_end->next, (uint32_t)touch_area);
    return touch_area;
}

// Todo
// static void delete_touch_area(touch_area_info_t* touch_area) {

// }

static void update_touch_area(uint16_t x, uint16_t y, uint8_t press) {
    static uint8_t last_press = 0;
    uint64_t time_now = xTaskGetTickCount();

    if (press == last_press && last_press == 0) {
        return ;
    }


    touch_area_info_t* touch_area = (touch_area_info_t*)atomic_load(&touch_area_ahead.next);

    if (press == 0) {
        while (touch_area != NULL) {
            atomic_store(&touch_area->status, false);
            atomic_store(&touch_area->touch_time, 0);
            touch_area = (touch_area_info_t*)atomic_load(&touch_area->next);
        }
    } else {
        while (touch_area != NULL) {
            if (x < touch_area->x1 || x > touch_area->x2 || y < touch_area->y1 || y > touch_area->y2) {
                atomic_store(&touch_area->status, false);
            } else {
                atomic_store(&touch_area->status, true);
                if (last_press == 0) {
                    atomic_store(&touch_area->pressed, true);
                    atomic_store(&touch_area->touch_time, time_now);
                }
            }
            touch_area = (touch_area_info_t*)atomic_load(&touch_area->next);
        }
    }

    last_press = press;
}

STATIC mp_obj_t touch_area_was_pressed(mp_obj_t self_in) {
    touch_area_info_t* touch_area = (touch_area_info_t*)atomic_load(&((touch_area_t *)self_in)->touch_area_info);
    uint8_t status = atomic_load(&touch_area->pressed);
    if (status) {
        atomic_store(&touch_area->pressed, false);
        return mp_const_true;
    } else {
        return mp_const_false;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch_area_was_pressed_obj, touch_area_was_pressed);

STATIC mp_obj_t touch_area_get_touch_time(mp_obj_t self_in) {
    touch_area_info_t* touch_area = (touch_area_info_t*)atomic_load(&((touch_area_t *)self_in)->touch_area_info);
    uint64_t time_touch = atomic_load(&touch_area->touch_time);
    uint64_t time_now = xTaskGetTickCount();

    if (time_touch == 0) {
        return mp_obj_new_int(0);
    } else {
        return mp_obj_new_int(time_now - time_touch);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch_area_get_touch_time_obj, touch_area_get_touch_time);

STATIC mp_obj_t touch_area_get_touch_status(mp_obj_t self_in) {
    touch_area_info_t* touch_area = (touch_area_info_t*)atomic_load(&((touch_area_t *)self_in)->touch_area_info);
    return mp_obj_new_bool(atomic_load(&touch_area->status));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(touch_area_get_touch_status_obj, touch_area_get_touch_status);

STATIC mp_obj_t touch_area_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args) {
    static const mp_arg_t allowed_args[] = {
        { MP_QSTR_x,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_y,        MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_width,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
        { MP_QSTR_hight,    MP_ARG_REQUIRED | MP_ARG_INT, {.u_int = -1} },
    };
    mp_arg_val_t args[MP_ARRAY_SIZE(allowed_args)];
    mp_arg_parse_all_kw_array(n_args, n_kw, all_args, MP_ARRAY_SIZE(allowed_args), allowed_args, args);

    touch_area_t *self = (touch_area_t *)m_new_obj(touch_area_t);
    self->base.type = type;
    atomic_store(&self->touch_area_info, (uint32_t)create_new_touch_area(args[0].u_int, args[1].u_int, args[2].u_int, args[3].u_int));
    return MP_OBJ_FROM_PTR(self);
}

STATIC const mp_rom_map_elem_t touch_area_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_was_pressed), MP_ROM_PTR(&touch_area_was_pressed_obj) },
    { MP_ROM_QSTR(MP_QSTR_touch_time),  MP_ROM_PTR(&touch_area_get_touch_time_obj) },
    { MP_ROM_QSTR(MP_QSTR_status),      MP_ROM_PTR(&touch_area_get_touch_status_obj) },
};

STATIC MP_DEFINE_CONST_DICT(touch_area_locals_dict, touch_area_locals_dict_table);

STATIC const mp_obj_type_t touch_area_type = {
    { &mp_type_type },
    .name = MP_QSTR_touch_area,
    .make_new = touch_area_make_new,
    .locals_dict = (mp_obj_dict_t*)&touch_area_locals_dict,
};


// ================================================ touch ===========================================================================
STATIC mp_obj_t touch_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *all_args)
{
    touch_t *self = m_new_obj(touch_t);
    self->base.type = type;

    if (touch_task_handle != NULL) {
        return MP_OBJ_FROM_PTR(self);
    }

    i2c_device =  i2c_malloc_device(I2C_NUM_1, 21, 22, 400000, 10000, TOUCH_ADDR);
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_NEGEDGE;
    io_conf.pin_bit_mask = ( 1ULL << TOUCH_INTR_PIN );
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pull_up_en = 1;
    gpio_config(&io_conf);
    
    BaseType_t xReturned = xTaskCreatePinnedToCore(touch_task, "touch task", 2*1024, NULL, (ESP_TASK_PRIO_MIN + 1), &touch_task_handle, !MP_TASK_COREID);
    if (xReturned != pdPASS) {
        ESP_LOGE(TAG, "Failed createing touch task!");
        vTaskDelete(touch_task_handle);
        mp_raise_msg(&mp_type_RuntimeError, MP_ERROR_TEXT("Failed creating touch task"));
    }

    gpio_isr_handler_add(TOUCH_INTR_PIN, touch_isr_handler, NULL);

    ESP_LOGD(TAG, "touch Initialized");

    return MP_OBJ_FROM_PTR(self);
}

STATIC mp_obj_t touch_read(mp_obj_t self_in)
{
    mp_obj_t tuple[3] = {
        mp_obj_new_int(atomic_load(&touch_info.x)),
        mp_obj_new_int(atomic_load(&touch_info.y)),
        mp_obj_new_bool(atomic_load(&touch_info.touched)),
    };
    return mp_obj_new_tuple(MP_ARRAY_SIZE(tuple), tuple);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_read_obj, touch_read);

STATIC mp_obj_t touch_status(mp_obj_t self_in)
{
    return mp_obj_new_bool(atomic_load(&touch_info.touched));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(mp_status_obj, touch_status);

STATIC void IRAM_ATTR touch_isr_handler(void* arg)
{
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    xTaskResumeFromISR(touch_task_handle);
    if (xHigherPriorityTaskWoken) portYIELD_FROM_ISR();
}

STATIC void touch_task(void* arg)
{
    uint8_t buff[5] = {0x00, 0x00, 0x00, 0x00, 0x00};
    bool _pressed;
    int16_t _x = 0, _y = 0;

    for (;;) {
        i2c_read_bytes(i2c_device, 0x02, buff, 5);
        _pressed = buff[0] ? true : false;
        _x = ((buff[1] & 0x0f) << 8) | buff[2];
        _y = ((buff[3] & 0x0f) << 8) | buff[4];
        atomic_store(&touch_info.x, _x);
        atomic_store(&touch_info.y, _y);
        atomic_store(&touch_info.touched, _pressed);
        update_touch_area(_x, _y, _pressed);

        if (_pressed == false) {
            vTaskSuspend(NULL);
        } else {
            vTaskDelay(pdMS_TO_TICKS(20));
        }
    }
}

STATIC const mp_rom_map_elem_t touch_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_read),      MP_ROM_PTR(&mp_read_obj) },
    { MP_ROM_QSTR(MP_QSTR_status),    MP_ROM_PTR(&mp_status_obj) },
};

STATIC MP_DEFINE_CONST_DICT(touch_locals_dict, touch_locals_dict_table);

STATIC const mp_obj_type_t touch_type = {
    { &mp_type_type },
    .name = MP_QSTR_touch,
    .make_new = touch_make_new,
    .locals_dict = (mp_obj_dict_t*)&touch_locals_dict,
};

STATIC const mp_rom_map_elem_t touch_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_touch) },
    { MP_ROM_QSTR(MP_QSTR_touch), (mp_obj_t)&touch_type},
    { MP_ROM_QSTR(MP_QSTR_touch_area), (mp_obj_t)&touch_area_type},
};
STATIC MP_DEFINE_CONST_DICT(mp_module_touch_globals, touch_globals_table);

const mp_obj_module_t mp_module_touch = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_touch_globals
};

MP_REGISTER_MODULE(MP_QSTR_touch, mp_module_touch, 1);
