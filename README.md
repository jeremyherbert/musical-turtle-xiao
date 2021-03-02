# musical-turtle-xiao

This repository contains a firmware implementation of a composite USB device which exposes the following functionality:

- USB to UART conversion (fixed at a baudrate of 9600)
- USB MIDI interface via SPI

It is designed to be used with the [seeeduino xiao](http://wiki.seeedstudio.com/Seeeduino-XIAO/) (ATSAMD21G18A). When flashed with the correct firmware, this will be hereafter referred to as a *turtleboard*.

**Important notes:**

- The seeeduino xiao supports 0V to 3.3V digital signals. Connecting signals outside this range will likely cause permanent damage to the device.
- For simplicity, the USB to UART baudrate is fixed at 9600. The device will ignore requests from the PC to change this.

## Flashing the musical-turtle-xiao firmware

1. Download the firmware file `musical-turtle-xiao.uf2` from the [releases](https://github.com/jeremyherbert/musical-turtle-xiao/releases) section of this repository.
2. Connect your seeeduino xiao to your computer with a USB C cable.
3. Force your device into bootloader mode. To do this, you need to take a wire and connect the RST and GND pads together **twice** in quick succession. These two pads are located next to the USB connector on the board. See the animation at the end of this list for an example.
4. A new USB storage device will appear connected to your computer (likely called "ARDUINO" or something similar)
5. Copy and paste the `musical-turtle-xiao.uf2` file onto the new storage device
6. The device will reflash its own firmware and then disconnect the storage device. You're now ready to go! 
7. (Your device may need a power cycle to start correctly the first time; just unplug and replug the USB cable.)

![bootloader-animation](https://github.com/SeeedDocument/Seeeduino-XIAO/raw/master/img/XIAO-reset.gif)

## Usage

### USB to UART
To use the USB to UART converter, simply connect the device to a free USB port on your computer. Drivers should be automatically installed on all platforms that need them. The device will appear in your operating system as a serial port.

### USB MIDI

This device supports 128 notes, and 125 control channels. Much like the USB to UART interface, drivers should be automatically installed on all platforms that need them.

Three types of message can be sent to the device: Note On, Note Off and Control Change.

The data can be sent using the SPI interface on the turtleboard; it behaves as a receive-only SPI slave (so there is no need to connect MISO as it is unused). The interface uses Motorola SPI mode 0, meaning that the clock polarity is active low, and the phase polarity is first/rising edge. The maximum supported clock speed of the interface is 1MHz.

All SPI transactions are two bytes long. The first byte is the message type, and the second byte contains packet value. A list of the possible message types is below.

| Value of first byte (hex) | Message type                 |
|---------------------------|------------------------------|
| 0x00                      | No operation (ignored)       |
| 0x01                      | Note On                      |
| 0x02                      | Note Off                     |
| 0x03                      | Control Change for channel 1 |
| 0x04                      | Control Change for channel 2 |
| 0x05                      | Control Change for channel 3 |
| ...                       | ...                          |
| 0x03 + N                  | Control change for channel N |

In the Note On and Note Off packets, the second byte is the MIDI note number for the note you wish to turn on or off (between 0-127 inclusive). For the Control Change message, the second byte is the level of the control (between 0-127 inclusive).

Due to the way that USB works, the device cannot immediately send the MIDI data to the PC. The device is configured to send updates every 2ms (500Hz), but this may be much longer depending on the USB host controller and operating system that the device is connected to. If data is sent too fast, messages may be skipped or corrupted.

In order to send messages successfully, you should communicate with the turtleboard in the following manner:

1. Wait until the busy pin is low
2. Set CS low
3. Send two bytes of SPI data (ie the MIDI message)
4. Wait at least 10us
5. Set CS high

Repeat this process each time you wish to send a message to the device.

## LEDs

There are four LEDs next to the USB connector. 

- The green LED indicates that the board is powered on. 
- The yellow LED blinks when SPI data has been received. 
- The two blue LEDs correspond to UART TX and RX activity.

## Connection information

WARNING: Do not use the pinout information provided by seeedstudio. 

The blank pins on the connection layout must be left floating.

![musical-turtle-xiao pinout](https://i.imgur.com/wzKBnAp.png)