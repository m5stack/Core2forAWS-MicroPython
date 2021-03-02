import gc
import uos
from flashbdev import bdev

try:
    if bdev:
        uos.mount(bdev, "/")
except OSError:
    import inisetup

    vfs = inisetup.setup()

gc.collect()

import axp192
pmu = axp192.Axp192()
pmu.powerAll()

import M5Library
lcd = M5Library.TFT(pmu.setLCDBrightness)

import sk6812 as _sk6812
sk6812 = _sk6812.SK6812()

import touch as _touch
touch = _touch.touch()
button_left = _touch.touch_area(0, 240, 106, 60)
button_middle = _touch.touch_area(106, 240, 106, 60)
button_right = _touch.touch_area(212, 240, 106, 60)

import time
lcd.fillScreen(lcd.BLACK)
# Alignment demo
for datum in range(9):
    lcd.fillScreen(lcd.BLACK)

    lcd.font(lcd.FONT_DejaVu26)
    lcd.text(160, 120, "fgG", lcd.WHITE, bgcolor=lcd.WHITE, textdatum=datum)
    
    lcd.font(lcd.FONT_DejaVu14)
    lcd.drawCircle(160, 120, 5, lcd.GREEN)
    lcd.drawPixel(160, 120, lcd.GREEN)
    
    time.sleep(1)

lcd.font(lcd.FONT_DejaVu26)
lcd.setColor(lcd.BLUE, lcd.BLACK)
lcd.text(160, 120, "69", textdatum=lcd.DATUM_MC)

lcd.font(lcd.FONT_DejaVu14)
lcd.drawCircle(160, 120, 5, lcd.GREEN)
lcd.drawPixel(160, 120, lcd.GREEN)

lcd.setCursor(0, 0)
for i in range(6):
    lcd.font(i)
    lcd.print('a')
    time.sleep(0.2)

lcd.font(lcd.FONT_DejaVu14)
lcd.print("print demo\r\n", 0, 0)
lcd.setCursor(1, 40)
lcd.print("hia" * 20)

lcd.drawRect(0, 0, 160, 120, lcd.GREEN)
