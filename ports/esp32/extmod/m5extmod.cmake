# CMake fragment for MicroPython m5 extmod component

set(MICROPY_M5_EXTMOD_DIR "${MICROPY_DIR}/ports/esp32/extmod")

file(GLOB_RECURSE DISPLAY_FILE ${MICROPY_M5_EXTMOD_DIR}/tft/*.c)

set(MICROPY_M5_SOURCE_EXTMOD
    ${MICROPY_M5_EXTMOD_DIR}/moddisplay.c
    ${DISPLAY_FILE}
)
