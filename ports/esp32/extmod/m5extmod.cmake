# CMake fragment for MicroPython m5 extmod component

set(MICROPY_M5_EXTMOD_DIR "${MICROPY_DIR}/ports/esp32/extmod")

file(GLOB_RECURSE DISPLAY_FILE ${MICROPY_M5_EXTMOD_DIR}/tft/*.c)

set(MICROPY_M5_SOURCE_EXTMOD
    ${MICROPY_M5_EXTMOD_DIR}/i2c_bus/i2c_device.c
    ${MICROPY_M5_EXTMOD_DIR}/moddisplay.c
    ${MICROPY_M5_EXTMOD_DIR}/modtouch.c
    ${MICROPY_M5_EXTMOD_DIR}/machine_i2c.c
    ${DISPLAY_FILE}
)

list(APPEND MICROPY_SOURCE_EXTMOD_EXTRA ${MICROPY_M5_SOURCE_EXTMOD})

# remove micropython esp32 base i2c lib
list(REMOVE_ITEM MICROPY_SOURCE_PORT ${PROJECT_DIR}/machine_i2c.c)
