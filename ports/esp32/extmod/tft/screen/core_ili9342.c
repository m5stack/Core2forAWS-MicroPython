#include "string.h"
#include "esp_heap_caps.h"
#include "driver/gpio.h"
#include "driver/mcpwm.h"
#include "soc/mcpwm_reg.h"
#include "soc/mcpwm_struct.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../gfxfont/gfx.h"
#include "../esp32_hal_spi.h"

#define MOSI_PIN 23
#define MISO_PIN 19
#define CLK_PIN 18
#define CS_PIN 14
#define DC_PIN 27
#define RST_PIN 33
#define BCKL_PIN 32

static tft_host_t* core_tft_host = NULL;

static const uint8_t ili9342_reg_init[] = {
  17,                   					        // 22 commands in list
  0xC8, 3, 0xFF, 0x93, 0x42,
  0xC5, 1, 0xf0,   //F5 E8   ///1
  0xC0, 2, 0x12, 0x12,
  0xC1, 1, 0x03,
  0x2A, 4, 0x00, 0x00, 0x01, 0x3f,
  0x2B, 4, 0x00, 0x00, 0x00, 0xef,
  0xb0, 1, 0xe0,
  0xf6, 3, 0x00, 0x01, 0x01,
  0xb6, 4, 0x0a, 0xe0, 0x1d, 0x04,
  
  0xC2, 1, 0xA3,
  0xB4, 1, 0x02,
  0xB1, 2, 0x00, 0x1B,

  0xE0,15,0x00,0x0C,0x11,0x04,0x11,0x08,0x37,0x89,0x4C,0x06,0x0C,0x0A,0x2E,0x34,0x0F,
  0xE1,15,0x00,0x0B,0x11,0x05,0x13,0x09,0x33,0x67,0x48,0x07,0x0E,0x0B,0x2E,0x33,0x0F,

  ///0xB6, 2, 0x0A, 0x40,  ///2
  0x11, TFT_CMD_DELAY, 6,
  0x29, TFT_CMD_DELAY, 1,
  0x36, 1, 0x08,
};

static void pinInit(tft_host_t* host) {
    gpio_pad_select_gpio(host->base.cs);
    gpio_set_direction(host->base.cs, GPIO_MODE_OUTPUT);

    gpio_pad_select_gpio(host->base.dc);
    gpio_set_direction(host->base.dc, GPIO_MODE_OUTPUT);

    if (host->base.rst >= 0) {
        gpio_pad_select_gpio(host->base.rst);
        gpio_set_direction(host->base.rst, GPIO_MODE_OUTPUT);
    }
}

static void setBrightness(tft_host_t* host, uint8_t brightness) {
    static uint8_t brightness_init = false;
    if (brightness_init == false) {
        mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, host->base.bckl);
        mcpwm_config_t pwm_config;
        pwm_config.frequency = 24000;    //frequency = 50Hz, i.e. for every servo motor time period should be 20ms
        pwm_config.cmpr_a = 0;    //duty cycle of PWMxA = 0
        pwm_config.cmpr_b = 0;    //duty cycle of PWMxb = 0
        pwm_config.counter_mode = MCPWM_UP_COUNTER;
        pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
        mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &pwm_config);    //Configure PWM0A & PWM0B with above settings
        brightness_init = true;
    }

    if (brightness > 100) {
        brightness = 100;
    }

    mcpwm_set_duty_in_us(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, brightness);
}

tft_host_t* coreScreenInit() {
    if (core_tft_host == NULL) {
        core_tft_host = (tft_host_t*)heap_caps_calloc(1, sizeof(tft_host_t), MALLOC_CAP_DEFAULT | MALLOC_CAP_INTERNAL);
        core_tft_host->base.mosi = MOSI_PIN;
        core_tft_host->base.miso = MISO_PIN;
        core_tft_host->base.cs = CS_PIN;
        core_tft_host->base.clk = CLK_PIN;
        core_tft_host->base.dc = DC_PIN;
        core_tft_host->base.rst = RST_PIN;
        core_tft_host->base.bckl = BCKL_PIN;
        core_tft_host->base.width = 320;
        core_tft_host->base.hight = 240;

        spi_device_handle_t spi_handle = spiBusInit(MOSI_PIN, MISO_PIN, CLK_PIN, CS_PIN);
        spi_device_acquire_bus(spi_handle, portMAX_DELAY);
        LcdHostInitDefault(core_tft_host);
        LcdHostInitFontGfx(core_tft_host);
        pinInit(core_tft_host);
        core_tft_host->setBrightness = setBrightness;
    }

    gpio_set_level(core_tft_host->base.rst, 0);
    vTaskDelay(pdMS_TO_TICKS(1));
    gpio_set_level(core_tft_host->base.rst, 1);
    vTaskDelay(pdMS_TO_TICKS(60));

    core_tft_host->writeCmdList(core_tft_host, ili9342_reg_init);
    uint8_t bpc = TFT_COLOR_BITS_16;
    core_tft_host->writeCmdData(core_tft_host, TFT_CMD_PIXFMT, &bpc, 1);
    core_tft_host->writeCmd(core_tft_host, 0x21);
    core_tft_host->setRotation(core_tft_host, PORTRAIT);
    core_tft_host->fillScreen(core_tft_host, TFT_BLACK);
    core_tft_host->setBrightness(core_tft_host, 20);
    core_tft_host->setColor(core_tft_host, TFT_WHITE, TFT_WHITE);
    return core_tft_host;
}
