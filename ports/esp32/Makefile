# Makefile for MicroPython on ESP32.
#
# This is a simple, convenience wrapper around idf.py (which uses cmake).

# Select the board to build for, defaulting to GENERIC.
BOARD ?= EduKit

# If the build directory is not given, make it reflect the board name.
BUILD ?= build-$(BOARD)

# Device serial settings.
PORT ?= /dev/ttyUSB0
BAUD ?= 1500000

PYTHON ?= python3

GIT_SUBMODULES = lib/berkeley-db-1.xx

.PHONY: all lfs2 clean deploy erase submodules FORCE

IDFPY_FLAGS += -D MICROPY_BOARD=$(BOARD) -B $(BUILD)

all:
	idf.py $(IDFPY_FLAGS) build
	@$(PYTHON) makeimg.py \
		$(BUILD)/bootloader/bootloader.bin \
		$(BUILD)/partition_table/partition-table.bin \
    		$(BUILD)/micropython.bin \
    		$(BUILD)/firmware.bin

$(BUILD)/bootloader/bootloader.bin $(BUILD)/partition_table/partition  -table.bin $(BUILD)/micropython.bin: FORCE

lfs2:
	@$(PYTHON) makelfs2.py \
		boards/$(BOARD)/image_file \
		$(BUILD)/lfs2_image.bin \
		0x190000
	esptool.py -p $(PORT) -b $(BAUD) write_flash 0x210000 $(BUILD)/lfs2_image.bin

clean:
	idf.py $(IDFPY_FLAGS) fullclean

deploy:
	idf.py $(IDFPY_FLAGS) -p $(PORT) -b $(BAUD) flash

erase:
	idf.py $(IDFPY_FLAGS) erase_flash

submodules:
	git submodule update --init $(addprefix ../../,$(GIT_SUBMODULES))
