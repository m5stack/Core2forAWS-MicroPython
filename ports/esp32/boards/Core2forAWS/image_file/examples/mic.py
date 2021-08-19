from machine import Pin
from M5Library import I2S
from Edukit import lcd
import struct
import time

audio_in = I2S(I2S.NUM0, bck=Pin(12), ws=Pin(0), sdin=Pin(34), standard=I2S.LSB, mode=I2S.MASTER_RX | I2S.MODE_PDM,
            dataformat=I2S.B16, channelformat=I2S.ONLY_RIGHT, samplerate=500000, dmacount=50, dmalen=256)

mic_samples = bytearray(50)
mic_samples_mv = memoryview(mic_samples)
num_bytes_read_from_mic = audio_in.readinto(mic_samples_mv, timeout=0)


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
