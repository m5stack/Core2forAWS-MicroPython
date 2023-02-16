#   AWS IoT connectivity example in MicroPython for the M5Stack Core2 for AWS IoT Kit
#   This example uses the ESP32 to connect to AWS IoT Core using the attached ATECC608
#   Trust&GO secure element for establishing a secure TLS connection.
#
#   Prerequisite — This example assumes you have completed the AWS IoT Kit tutorials
#   1) Getting Started
#   2) Cloud Connected Blinky / Hello World
#
#   The examples requires the thing (device) to be registered with AWS IoT with the 
#   policy included in the tutorials. Your publish topic and subscribe topics must
#   use the <<CLIENT_ID>>/ prefix, where the client Id is the unique serial number of
#   the device's secure element.
#
#   TO RUN THIS EXAMPLE:
#   Update the Wi-Fi credentials with the SSID and password for a 2.4GHz network.
#   Paste in the AWS IoT MQTT broker endpoint address in the spave below. There should
#   not be a protocol (e.g. "mqtt://") prefix or a trailing "/" at the end.

import network
from umqtt import robust
import time
import atecc608x
import binascii
import json

wifi_ssid = "AWSWorkshop"       # Your Wi-Fi network SSID
wifi_pass = "IoTP$AK1t"         # Your Wi-Fi network password
mqtt_endpoint_address = ""      # Retrieve using the command: aws iot describe-endpoint --endpoint-type iot:Data-ATS

atecc608x.init()
atecc608x.get_serial_number()

client_id = binascii.hexlify(atecc608x.get_serial_number())

with open("res/aws-root-ca.pem", 'r') as fo:
        root_ca = fo.read()
mqtt_client = robust.MQTTClient(client_id, mqtt_endpoint_address, port=8883, keepalive=180, atecc_ssl=True, ssl_params={"ca_certs": root_ca, "server_hostname": mqtt_endpoint_address})

def wifi_connect():
    wlan = network.WLAN(network.STA_IF)

    if not wlan.isconnected():
        print("Connecting to 2.4GHz Wi-Fi network: ", wifi_ssid)
        wlan.active(True)
        wlan.connect(wifi_ssid, wifi_pass)
        while not wlan.isconnected():
            pass
        print('Wi-Fi connected:', wlan.ifconfig())

# Received messages from MQTT topic subscriptions will be delivered to this callback
def sub_cb(topic, msg):
    print('Received MQTT message on topic: %s message: %s' % (topic, msg))

def aws_iot_publish():
    pub_topic = client_id + "/"
    print("Publishing to topic", pub_topic)
    mqtt_client.publish(pub_topic, json.dumps({"message": "Hello World", "device_type": "Core2 for AWS IoT Kit"}), qos=0)

def aws_iot_subscribe():
    sub_topic = client_id + "/#"
    print("Subscribing to topic", sub_topic)
    mqtt_client.subscribe(sub_topic)

    while True:
        mqtt_client.check_msg()
        aws_iot_publish()
        time.sleep(1)

    mqtt_client.disconnect()

def aws_mqtt_pub_sub():
    mqtt_client.set_callback(sub_cb)
    
    print("Attempting to connect to AWS IoT over MQTT using the ATECC608, MQTT client Id:", client_id.decode("utf-8"))
    mqtt_client.connect()

    print("Successfully connected to AWS IoT")

def main():
    wifi_connect()
    aws_mqtt_pub_sub()

if __name__ == "__main__":
    main()