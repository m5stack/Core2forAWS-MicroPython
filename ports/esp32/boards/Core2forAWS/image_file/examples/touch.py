from Kit import btn_left, touch
import time

while True:
    print(btn_left.status(), btn_left.touch_time(), btn_left.was_pressed())
    print(touch.read(), touch.status())
    time.sleep(0.5)
