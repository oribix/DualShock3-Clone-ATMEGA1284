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

I will update this README when I have a video demonstration.
