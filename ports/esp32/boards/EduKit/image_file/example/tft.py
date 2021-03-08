from Edukit import lcd
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
