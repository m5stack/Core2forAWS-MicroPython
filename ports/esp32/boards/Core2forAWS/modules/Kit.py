import axp192
pmu = axp192.Axp192()
pmu.powerAll()

import M5Library
lcd = M5Library.TFT(pmu.setLCDBrightness)

import touch as _touch
touch = _touch.touch()
btn_left = _touch.touch_area(0, 240, 106, 60)
btn_middle = _touch.touch_area(106, 240, 106, 60)
btn_right = _touch.touch_area(212, 240, 106, 60)
