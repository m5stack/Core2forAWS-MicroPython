import sk6812 as _sk6812
import time

sk6812 = _sk6812.SK6812()
sk6812.set_brightness(20)
sk6812.set_color(1, 0xff0000)

sk6812.set_color(2, 0x00ff00, update=False)
sk6812.set_color(3, 0x0000ff, update=False)
time.sleep(0.5)
sk6812.show()

time.sleep(0.5)
sk6812.set_side_color(0xff00ff, sk6812.LEFT)
sk6812.set_side_color(0x00ff00, sk6812.RIGHT)
