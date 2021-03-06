# DUALSHOCK3 Clone
This is a controller inspired by the Sony's DualShock 3.
It has Twin Thumb Sticks, an accelerometer, buttons, and is wireless.
This project uses a bluetooth module for wireless connectivity.

See the [Google Doc](https://drive.google.com/open?id=0B5K5OUHJGcLmLTlsRlRZWVctNUk) for more details.

[Video Demonstration](https://www.youtube.com/watch?v=FelWu7re3BA)

## Hardware

The accelerometer used is the AdaFruit LIS3DH.
I used HC-05 bluetooth modules for connectivity.
The Thumbsticks are Paralax 2-Axis joysticks.

See the [Google Doc](https://drive.google.com/open?id=0B5K5OUHJGcLmLTlsRlRZWVctNUk) for wiring.

## Controller protocol

The controller has a simple request-get USART protocol for retreiving the controller state.
The protocol begins by sending the controller a null byte.
The controller will then respond with 8 Bytes of data.
The Bytes are sent from Most Significant Byte to Least Significant Byte

The first 3 bytes are the X, Y, and Z accellerometer values in that order.
Each value is 1 byte and in 2's compliment.

The next 3 bytes represent the Thumb sticks state.
From MSB to LSB:

- Left Vertical Axis
- Left Horizontal Axis
- Right Vertical Axis
- Right Horizontal Axis

Each Axis is a 6 bits and unsigned.
This means that an axis is represented on a scale of 0-63
The value for an axis when the thumb stick is centered is about 31.

The final 2 bytes are the button states.
Each bit represents a button.
The button schema is based off the SNES controller.
This means that the 4 primary buttons on the right side of the controller are ABXY.
[Google the SNES controller to see its button schema](https://www.google.com/search?q=snes+controller).

From MSB to LSB:

B Y Sl St UP DN <- -> A X L1 R1 L2 R2 - -

Notice that the last 2 bits are "-". these 2 bits mean nothing.

Additionally, if the protocol begins with a value > 0 instead of a null byte, the controller vibrates.
The controller will continue to vibrate until a null byte is sent.

The USART baud rate is set to 38400.

## File Descriptions

### A2D.h:
This is a driver for the ADC.

### bit.h:
This has binary manipulation helper functions.

### accelerometer.h:
Driver written by me for the Adafruit LIS3DH.
It uses Software SPI to communicate.
There are write functions in there that have not been tested.
Use them at your own peril.

### usart_ATmega1284.h:
This has USART functions for the ATMEGA 1284.
