import network
from umqtt import robust
import time
import atecc608x
import binascii
import json

atecc608x.init()
atecc608x.get_serial_number()

client_id = binascii.hexlify(atecc608x.get_serial_number())
host_url = "a31klw4qph0psl-ats.iot.ap-southeast-1.amazonaws.com"

wlan = network.WLAN(network.STA_IF)
wlan.active(True)
wlan.connect("cam", "88888888")

with open("res/aws-root-ca.pem", 'r') as fo:
    root_ca = fo.read()

mqtt = robust.MQTTClient(client_id, host_url, port=8883, keepalive=180, atecc_ssl=True, ssl_params={"ca_certs": root_ca, "server_hostname": host_url})

while not wlan.isconnected():
    time.sleep(0.1)

print("Wi-Fi Connected success, Start conenct aws mqtt, id:", client_id)

mqtt.connect()
print("Connect to aws successed")

while True:
    mqtt.publish(client_id + "/echo", json.dumps({"message": "Hi AWS !!!", "from": "M5 Edukit"}) )
    time.sleep_ms(2000)
