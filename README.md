# M5Stack Core2 for AWS IoT EduKit MicroPython Examples
_NOTE:_ This repo is limited in functionality, and is a result of customer requests to have an AWS IoT referenceable example utilizing the ATECC608 in MicroPython. For more complete set of MicroPython functionality, we recommend using M5Stack's UIFlow tool. Community contributions are welcome.

## General Setup and Usage
To use this repository on the [M5Stack Core2 for AWS IoT EduKit](https://m5stack.com/products/m5stack-core2-esp32-iot-development-kit-for-aws-iot-edukit), you must have the [ESP-IDF release v4.2](https://github.com/espressif/esp-idf/tree/release/v4.2) installed first. You can find the installation instructions on [Espressif's Documentation](https://docs.espressif.com/projects/esp-idf/en/release-v4.2/esp32/get-started/index.html#installation-step-by-step). With the ESP-IDF tools added to your path, you can follow the steps below to run the various examples provided in this repository.

### Compiling the MicroPython Runtime
To compile the runtime ported for the Core2 for AWS, you'll need to use the following commands to go into the port directory and use the **make** build system to compile the firmware. This will take some time depending on your computer's configuration:

```bash
cd ports/esp32
make
```

### Uploading the MicroPython Runtime
The now compiled firmware can be uploaded to the device by using the make operation "deploy":

_NOTE:_ If you receive an could not open port error, modify the **ports/esp32/Makefile** and replace the `PORT ?= /dev/ttyUSB0` with the correct port that the device is mounted to on your machine.

```bash
make deploy
```

### Uploading the Provided Examples
There are a examples created specifically for the Core2 for AWS located in the **ports/esp32/boards/Core2forAWS/image_file/examples** folder. To upload these files to the device's filesystem, you can use the configured make operation "lfs2":

```bash
make lfs2
```

### Running one of the Uploaded Examples
To run the example manually, you can execute the script from the REPL. To enter REPL prompt, use the make operation "monitor":

```bash
make monitor
```

_NOTE:_ To exit the serial monitor at any time, use the key combination **CTRL + }**

To view the list of examples in the device's filesystem in the "examples" folder, use the **os** library.

```bash
os.listdir('examples')
```

To run one of the examples, replace the <<FILENAME>> with the name of the Python script you want to execute:

```bash
execfile("examples/<<FILENAME>>.py")
```

## Running the AWS IoT Connectivity Example
### Prerequisites
In order to run the example, you will need to first have the device registered to your AWS account with the required policy attached to the thing, and your AWS IoT mqtt host endpoint address. It's recommended you complete the following AWS IoT EduKit tutorials first so you have everything setup:
1) [Getting Started](https://edukit.workshop.aws/en/getting-started.html)
2) [Cloud Connected Blinky](https://edukit.workshop.aws/en/blinky-hello-world.html)

### Wi-Fi Configuration
To setup Wi-Fi you'll need to open the **ports/esp32/boards/Core2forAWS/image_file/examples/AWS_IoT_connect.py** file in your editor. You will need to modify the code below with the correct values in between the quotes:
```Python
wifi_ssid = "AWSWorkshop"       # Your Wi-Fi network SSID
wifi_pass = "IoTP$AK1t"         # Your Wi-Fi network password
```
### AWS IoT Core Endpoint Address Configuration
You will need to tell the client application the address of where to connect. To retrieve the AWS IoT ATS endpoint address, use the following AWS CLI command in a separate (not REPL) terminal window:

```bash
aws iot describe-endpoint --endpoint-type iot:Data-ATS
```

Paste the value with the address retrieved from above in the Python script:

```Python
mqtt_endpoint_address = ""
```

### Running the AWS IoT Example
With the files newly modified, you'll need to reupload them to the device first. To do so, enter the command in your terminal window from the **/ports/esp32/** directory:
```bash
make lfs2
```

Next, you'll execute that example from the REPL prompt by entering the following:
```bash
make monitor
execfile("examples/AWS_IoT_connect.py")
```

## Clean up
To save power, bandwidth, and avoid unexpected AWS charges, it is always good to shut off usage of resources. You can either hold the power button for 6-seconds to shut off the device, or erase the flash memory.

_NOTE:_ Erasing the flash memory will cause the device to make a "ticking" sound as it constantly restarts itself due to not having an application to run. To avoid this, it is recommended you upload the [factory firmware](https://github.com/m5stack/Core2-for-AWS-IoT-EduKit/tree/master/Factory-Firmware).

To erase the flash memory, you can use the configured make operation "erase":

```bash
make erase
```