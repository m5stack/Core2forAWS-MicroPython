import M5Library
import machine
from micropython import const

LEFT  = const(0)
RIGHT = const(1)
ALL   = const(2)

class SK6812:
    def __init__(self):
        self._np = M5Library.Neopixel(machine.Pin(25), 10)

    def set_side_color(self, color, side=ALL):
        _start = 6 if side == LEFT else 1
        _num = 10 if side == ALL else 5
        self._np.set(_start, color, num=_num)

    def set_color(self, pos, color, update=True):
        self._np.set(pos, color, update=update)

    def set_brightness(self, brightness):
        self._np.brightness(brightness)

    def show(self):
        self._np.show()
