# Avr Servo Controller

This is a simple 12 servo controller based on AtMega8 microcontroller.
It uses uart as an input port, the pinout is prepared to use with cheap Bluetooth transceiver HC-05 family, but can be used with any uart compatible devices like Raspberry Pi, Arduino USB to Serial converter, etc.

## AtMega Setup

### Program
I strongly advice to use socket for microcontroller or use bootloader to be able to update uc's software.

### Fuses
* High: 0xDC
* Low: 0xA4

### Uart settings
* 9600 baud
* One stop bit
* No parity

## Protocol

This device uses very simple protocol, you just have to send three bytes frames to it.
* First - start byte is always 0xAA
* Second - servo name - A to L (Ascii value - 0x41 to 0x4C)
* Third - position value from 0 is (max left position - 0,5 ms) to 0xFF (max right position - 2,5ms) the step is about 8us
* Frames are send one after one. If no data is present, the controller remembers last position.
* Bad frames are skipped

## Scheme

![Scheme](http://www.mlodedrwale.pl/Github/Avr-Servo-Controller/scheme.png)

## PCB

Printed board is designed in Eagle

![Layout](http://www.mlodedrwale.pl/Github/Avr-Servo-Controller/pcb.png)

### Download

[Click to download Eagle PCB and scheme project](http://www.mlodedrwale.pl/Github/Avr-Servo-Controller/AVR-Servo-Controller-Board.zip)

## Demo app for testing

![Screenshot](http://www.mlodedrwale.pl/Github/Avr-Servo-Controller-Demo/screen.png)

[Link to demo App on GitHub](https://github.com/mlodedrwale/Avr-Servo-Controller-Demo)