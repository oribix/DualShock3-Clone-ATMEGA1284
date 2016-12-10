This is a controller inspired by the Sony's DualShock 3.
It has Twin Thumb Sticks, an accelerometer, buttons, and is wireless.
This project uses a bluetooth module for wireless conectivity.

This project uses FreeRTOS to power it.
As a result, most of the files here are not my own code but rather FreeRTOS.
"main.c" and "accelerometer.h" are all my own original code.

"main.c" contains the code for the controller itself.
"accelerometer.h" is a driver that I created for the Adafruit LIS3DH.
It assumes 4-wire SPI connection.

The accelerometer used is the AdaFruit LIS3DH.
I used HC-05 bluetooth modules for connectivity.
The Thumbsticks are Paralax 2-Axis joysticks.

The controller has a simple request-get USART protocol for retreiving the controller state.
The protocol begins by sending the controller a null byte.
The controller will then respond with 8 Bytes of data.
The Bytes are sent from Most Significant Byte to Least Significant Byte

The first 3 bytes are the X, Y, and Z accellerometer values in that order.
Each value is 1 byte and in 2's compliment.

The next 3 bytes represent the Thumb sticks state.
From MSB to LSB:

Left Vertical Axis, Left Horizontal Axis, Right Vertical Axis, Right Horizontal Axis

Each Axis is a 6 bits and unsigned.
This means that an axis is represented on a scale of 0-63
The value for an axis when the thumb stick is centered is about 31.

The final 2 bytes are the button states.
Each bit represents a button.
The button schema is based off the SNES controller.
This means that the 4 primary buttons on the right side of the controller are ABXY.
Google the SNES controller to see its button schema.

From MSB to LSB:

B Y Sl St UP DN <- -> A X L1 R1 L2 R2 - -

Notice that the last 2 bits are "-". these 2 bits mean nothing.

Additionally, if the protocol begins with a value > 0 instead of a null byte, the controller vibrates.
The controller will continue to vibrate until a null byte is sent.

The USART baud rate is set to 38400.


I will update this README when I have a video demonstration.
