from micropython import const
import machine

# Registers overview
_ADDR = const(0x51)
_SECONDS = const(0x02)
_MINUTES = const(0x03)
_HOURS = const(0x04)
_DATE = const(0x05)
_WDAY = const(0x06)
_MONTH = const(0x07)
_YEAR = const(0x08)

class BM8563(object):
    def __init__(self, sda=21, scl=22):
        self.i2c = machine.I2C(1, sda=machine.Pin(sda), scl=machine.Pin(scl))
        self._write_to_reg(0x00, 0x00)
        self._write_to_reg(0x01, 0x00)

    def _bcd2dec(self, bcd):
        return (((bcd & 0xf0) >> 4) * 10 + (bcd & 0x0f))

    def _dec2bcd(self, dec):
        tens, units = divmod(dec, 10)
        return (tens << 4) + units

    def _write_to_reg(self, reg, value):
        buf = bytearray(1)
        buf[0] = value
        self.i2c.writeto_mem(_ADDR, reg, buf)

    def datetime(self, datetime=None):
        if datetime is None:
            data = self.i2c.readfrom_mem(_ADDR, _SECONDS, 7)
            ss = self._bcd2dec(data[0] & 0x7F)
            mm = self._bcd2dec(data[1] & 0x7F)
            hh = self._bcd2dec(data[2] & 0x3F)
            dd = self._bcd2dec(data[3] & 0x3F)
            wday = data[4] & 0x07
            MM = self._bcd2dec(data[5] & 0x1F)
            yy = self._bcd2dec(data[6]) + 2000
            return yy, MM, dd, wday, hh, mm, ss, 0

        try:
            (yy, MM, mday, wday, hh, mm, ss, yday) = datetime
        except ValueError:
            raise ValueError("RTC: (Years, Month, Date, Day, Hours, Minutes, Seconds, unused)")

        if ss < 0 or ss > 59:
            raise ValueError('RTC: Seconds is out of range [0,59].')
        if mm < 0 or mm > 59:
            raise ValueError('RTC: Minutes is out of range [0,59].')
        if hh < 0 or hh > 23:
            raise ValueError('RTC: Hours is out of range [0,23].')
        if mday < 1 or mday > 31:
            raise ValueError('RTC: Date is out of range [1,31].')
        if wday < 0 or wday > 6: 
            raise ValueError('RTC: Day is out of range [0,6].')
        if MM < 1 or MM > 12:  
            raise ValueError('RTC: Month is out of range [1,12].')
        if yy < 2000 or yy > 2099:   
            raise ValueError('RTC: Years is out of range [2000,2099].')

        yy = yy - 2000
        self._write_to_reg(_SECONDS, self._dec2bcd(ss))
        self._write_to_reg(_MINUTES, self._dec2bcd(mm))
        self._write_to_reg(_HOURS, self._dec2bcd(hh))   
        self._write_to_reg(_DATE, self._dec2bcd(mday))
        self._write_to_reg(_WDAY, self._dec2bcd(wday))
        self._write_to_reg(_MONTH, self._dec2bcd(MM))
        self._write_to_reg(_YEAR, self._dec2bcd(yy))
