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


from machine import Pin
from M5Library import I2S
audio_in = I2S(I2S.NUM0, bck=Pin(12), ws=Pin(0), sdin=Pin(34), standard=I2S.LSB, mode=I2S.MASTER_RX | I2S.MODE_PDM,
            dataformat=I2S.B16, channelformat=I2S.ONLY_RIGHT, samplerate=500000, dmacount=50, dmalen=256)

mic_samples = bytearray(50)
mic_samples_mv = memoryview(mic_samples)
num_bytes_read_from_mic = audio_in.readinto(mic_samples_mv, timeout=0)

import struct
import time

def got_mic():
    audio_in.readinto(mic_samples_mv, timeout=1000)
    data = sum(struct.unpack("<25H", mic_samples)) // 25
    return data


buffer = [0] * 100
def micro_show(y_offset=120):
    global buffer
    val = got_mic() // 800
    buffer.pop()
    buffer.insert(0, val)
    for i in range(1, 50):
        lcd.line(i * 2 + 44, y_offset + buffer[i + 1], i * 2 + 44 + 2, y_offset + buffer[i + 2], lcd.WHITE)
        lcd.line(i * 2 + 44, y_offset + buffer[i],  i * 2 + 44 + 2, y_offset + buffer[i + 1], lcd.BLACK)

lcd.clear(lcd.WHITE)

while True:
    micro_show()

# print(test())
