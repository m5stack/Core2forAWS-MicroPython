import network
from umqtt import robust
import time

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect("cam", "88888888")


with open("res/cert.pem", 'r') as fo:
    cert = fo.read()

with open("res/private.key", 'r') as fo:
    key = fo.read()

mqtt = robust.MQTTClient("Uiflow_test", "a31klw4qph0psl-ats.iot.ap-southeast-1.amazonaws.com", 
                            port=8883, keepalive=360, ssl=True, ssl_params={"cert": cert, "key": key})

while not wlan.isconnect():
    time.sleep(0.1)

mqtt.connect()
