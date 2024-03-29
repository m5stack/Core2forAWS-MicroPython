# Set location of base MicroPython directory.
get_filename_component(MICROPY_DIR ${PROJECT_DIR}/../.. ABSOLUTE)

# Include core source components.
include(${MICROPY_DIR}/py/py.cmake)
include(${MICROPY_DIR}/extmod/extmod.cmake)

set(MICROPY_QSTRDEFS_PORT
    ${PROJECT_DIR}/qstrdefsport.h
)

set(MICROPY_SOURCE_EXTMOD_EXTRA
    ${MICROPY_DIR}/extmod/modonewire.c
)

set(MICROPY_SOURCE_LIB
    ${MICROPY_DIR}/lib/littlefs/lfs1.c
    ${MICROPY_DIR}/lib/littlefs/lfs1_util.c
    ${MICROPY_DIR}/lib/littlefs/lfs2.c
    ${MICROPY_DIR}/lib/littlefs/lfs2_util.c
    ${MICROPY_DIR}/lib/mbedtls_errors/mp_mbedtls_errors.c
    ${MICROPY_DIR}/lib/mp-readline/readline.c
    ${MICROPY_DIR}/lib/netutils/netutils.c
    ${MICROPY_DIR}/lib/oofatfs/ff.c
    ${MICROPY_DIR}/lib/oofatfs/ffunicode.c
    ${MICROPY_DIR}/lib/timeutils/timeutils.c
    ${MICROPY_DIR}/lib/utils/interrupt_char.c
    ${MICROPY_DIR}/lib/utils/stdout_helpers.c
    ${MICROPY_DIR}/lib/utils/sys_stdio_mphal.c
    ${MICROPY_DIR}/lib/utils/pyexec.c
)

set(MICROPY_SOURCE_DRIVERS
    ${MICROPY_DIR}/drivers/bus/softspi.c
    ${MICROPY_DIR}/drivers/dht/dht.c
)

set(MICROPY_SOURCE_PORT
    ${PROJECT_DIR}/main.c
    ${PROJECT_DIR}/uart.c
    ${PROJECT_DIR}/gccollect.c
    ${PROJECT_DIR}/mphalport.c
    ${PROJECT_DIR}/fatfs_port.c
    ${PROJECT_DIR}/help.c
    ${PROJECT_DIR}/modutime.c
    ${PROJECT_DIR}/moduos.c
    ${PROJECT_DIR}/machine_timer.c
    ${PROJECT_DIR}/machine_pin.c
    ${PROJECT_DIR}/machine_touchpad.c
    ${PROJECT_DIR}/machine_adc.c
    ${PROJECT_DIR}/machine_dac.c
    ${PROJECT_DIR}/machine_i2c.c
    ${PROJECT_DIR}/machine_pwm.c
    ${PROJECT_DIR}/machine_uart.c
    ${PROJECT_DIR}/modmachine.c
    ${PROJECT_DIR}/modnetwork.c
    ${PROJECT_DIR}/network_lan.c
    ${PROJECT_DIR}/network_ppp.c
    ${PROJECT_DIR}/mpnimbleport.c
    ${PROJECT_DIR}/modsocket.c
    ${PROJECT_DIR}/modesp.c
    ${PROJECT_DIR}/esp32_nvs.c
    ${PROJECT_DIR}/esp32_partition.c
    ${PROJECT_DIR}/esp32_rmt.c
    ${PROJECT_DIR}/esp32_ulp.c
    ${PROJECT_DIR}/modesp32.c
    ${PROJECT_DIR}/espneopixel.c
    ${PROJECT_DIR}/machine_hw_spi.c
    ${PROJECT_DIR}/machine_wdt.c
    ${PROJECT_DIR}/mpthreadport.c
    ${PROJECT_DIR}/machine_rtc.c
    ${PROJECT_DIR}/machine_sdcard.c
)

set(MICROPY_SOURCE_QSTR
    ${MICROPY_SOURCE_PY}
    ${MICROPY_SOURCE_EXTMOD}
    ${MICROPY_SOURCE_EXTMOD_EXTRA}
    ${MICROPY_SOURCE_LIB}
    ${MICROPY_SOURCE_PORT}
)

set(IDF_COMPONENTS
    app_update
    bootloader_support
    bt
    driver
    esp32
    esp_common
    esp_eth
    esp_event
    esp_ringbuf
    esp_rom
    esp_wifi
    freertos
    heap
    log
    lwip
    mbedtls
    mdns
    newlib
    nvs_flash
    sdmmc
    soc
    spi_flash
    tcpip_adapter
    ulp
    vfs
    xtensa
)

if(IDF_VERSION_MINOR GREATER_EQUAL 1)
    list(APPEND IDF_COMPONENTS esp_netif)
endif()

if(IDF_VERSION_MINOR GREATER_EQUAL 2)
    list(APPEND IDF_COMPONENTS esp_system)
    list(APPEND IDF_COMPONENTS esp_timer)
endif()

if(IDF_VERSION_MINOR GREATER_EQUAL 3)
    list(APPEND IDF_COMPONENTS esp_hw_support)
    list(APPEND IDF_COMPONENTS esp_pm)
    list(APPEND IDF_COMPONENTS hal)
endif()

include(${MICROPY_DIR}/ports/esp32/extmod/m5extmod.cmake)

# Register the main IDF component.
idf_component_register(
    SRCS
        ${MICROPY_SOURCE_PY}
        ${MICROPY_SOURCE_EXTMOD}
        ${MICROPY_SOURCE_EXTMOD_EXTRA}
        ${MICROPY_SOURCE_LIB}
        ${MICROPY_SOURCE_DRIVERS}
        ${MICROPY_SOURCE_PORT}
    INCLUDE_DIRS
        ${MICROPY_DIR}
        ${MICROPY_PORT_DIR}
        ${MICROPY_BOARD_DIR}
        ${CMAKE_BINARY_DIR}
    REQUIRES
        ${IDF_COMPONENTS}
)

# Set the MicroPython target as the current (main) IDF component target.
set(MICROPY_TARGET ${COMPONENT_TARGET})

# Define mpy-cross flags, for use with frozen code.
set(MICROPY_CROSS_FLAGS -march=xtensawin)

# Set compile options for this port.
target_compile_definitions(${MICROPY_TARGET} PUBLIC
    MICROPY_ESP_IDF_4=1
    MICROPY_VFS_FAT=1
    MICROPY_VFS_LFS2=1
    FFCONF_H=\"${MICROPY_OOFATFS_DIR}/ffconf.h\"
    LFS1_NO_MALLOC LFS1_NO_DEBUG LFS1_NO_WARN LFS1_NO_ERROR LFS1_NO_ASSERT
    LFS2_NO_MALLOC LFS2_NO_DEBUG LFS2_NO_WARN LFS2_NO_ERROR LFS2_NO_ASSERT
)

# Disable some warnings to keep the build output clean.
target_compile_options(${MICROPY_TARGET} PUBLIC
    -Wno-clobbered
    -Wno-deprecated-declarations
    -Wno-missing-field-initializers
)

# Collect all of the include directories and compile definitions for the IDF components.
foreach(comp ${IDF_COMPONENTS})
    get_target_property(type __idf_${comp} TYPE)
    set(_inc OFF)
    set(_def OFF)
    if(${type} STREQUAL STATIC_LIBRARY)
        get_target_property(_inc __idf_${comp} INCLUDE_DIRECTORIES)
        get_target_property(_def __idf_${comp} COMPILE_DEFINITIONS)
    elseif(${type} STREQUAL INTERFACE_LIBRARY)
        get_target_property(_inc __idf_${comp} INTERFACE_INCLUDE_DIRECTORIES)
        get_target_property(_def __idf_${comp} INTERFACE_COMPILE_DEFINITIONS)
    endif()
    if(_inc)
        list(APPEND MICROPY_CPP_INC_EXTRA ${_inc})
    endif()
    if(_def)
        list(APPEND MICROPY_CPP_DEF_EXTRA ${_def})
    endif()
endforeach()

if(IDF_VERSION_MINOR GREATER_EQUAL 2)
    # These paths cannot currently be found by the IDF_COMPONENTS search loop above,
    # so add them explicitly.
    list(APPEND MICROPY_CPP_INC_EXTRA ${IDF_PATH}/components/soc/soc/${IDF_TARGET}/include)
    list(APPEND MICROPY_CPP_INC_EXTRA ${IDF_PATH}/components/soc/soc/include)
endif()

# Include the main MicroPython cmake rules.
include(${MICROPY_DIR}/py/mkrules.cmake)
