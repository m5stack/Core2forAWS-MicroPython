from Edukit import lcd
import time

# fun(a, b, [x, y]) mean: 
#    a, b required
#    x, y, has default value, not must be required

# Color: RGB888
# lcd.BLACK lcd.NAVY lcd.DARKGREEN lcd.DARKCYAN lcd.MAROON lcd.PURPLE lcd.OLIVE lcd.LIGHTGREY 
# lcd.DARKGREY lcd.BLUE lcd.GREEN lcd.CYAN lcd.RED lcd.MAGENTA lcd.YELLOW lcd.WHITE lcd.ORANGE lcd.GREENYELLOW lcd.PINK

# 0 ~ 100
# lcd.setBrightness(brightness)
# lcd.clear([color])
# lcd.fill([color])
# lcd.screensize() -> (width, hight)
# rot: 0, 1, 2, 3
# lcd.setRotation(rot) 
# lcd.fillScreen(color)

# font: lcd.FONT_DejaVu14 lcd.FONT_DejaVu16 lcd.FONT_DejaVu18 lcd.FONT_DejaVu24
#       lcd.FONT_DejaVu26 lcd.FONT_DejaVu40

# textdatum: lcd.DATUM_TL lcd.DATUM_TC lcd.DATUM_TR lcd.DATUM_ML lcd.DATUM_CL lcd.DATUM_MC 
#            lcd.DATUM_CC lcd.DATUM_MR lcd.DATUM_CR lcd.DATUM_BL lcd.DATUM_BC lcd.DATUM_BR 
#            lcd.BASELINE_L lcd.BASELINE_C lcd.BASELINE_R

# lcd.font(font)
# lcd.set_fg(color)
# lcd.set_bg(color)
# lcd.setColor(fg_color, bg_color)
# lcd.textWidth("asdasd") -> width
# lcd.fontHight("asdasd") -> font hight

# lcd.setTextDatum(textdatum)
# lcd.text(x, y, text, [color, bgcolor, textdatum])
# lcd.setCursor(x, y)
# lcd.getCursor() -> (x, y)
# lcd.print(text, [x, y, color, color, bg_color])
# lcd.println(text, [x, y, color, color, bg_color])

# only support jpeg now
# lcd.image(x, y, file, [scale])
# lcd.qrcode(text, [x, y, width, version])

# lcd.drawPixel(x, y, [color])
# lcd.drawLine(x, y, x1, y1, [color])
# lcd.drawRect(x, y, width, height, [color, fillcolor])
# lcd.fillRect(x, y, width, height, [color])
# lcd.drawCircle(x, y, r, [color, fillcolor])
# lcd.fillCircle(x, y, r, [color])
# lcd.drawTriangle(x, y, x1, y1, x2, y2, [color, fillcolor])
# lcd.fillTriangle(x, y, x1, y1, x2, y2, [color])
# lcd.drawRoundRect(x, y, width, height, r, [color, fillcolor])
# lcd.fillRoundRect(x, y, width, height, r, [color])


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
