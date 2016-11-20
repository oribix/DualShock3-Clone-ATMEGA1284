#include <stdint.h> 
#include <stdlib.h> 
#include <stdio.h> 
#include <stdbool.h> 
#include <string.h> 
#include <math.h> 
#include <avr/io.h> 
#include <avr/interrupt.h> 
#include <avr/eeprom.h> 
#include <avr/portpins.h> 
#include <avr/pgmspace.h> 
 
//FreeRTOS include files 
#include "FreeRTOS.h" 
#include "task.h" 
#include "croutine.h"

//binary manipulation helpers
#include "bit.h"
//drivers
#include "A2D.h"
#include "accelerometer.h"
#include "usart_ATmega1284.h"

#define TICKRATE 20         //ticks per second
#define TS_PRECISION  6     //thumb stick bit precision
#define TILT_PRECISION 8    //accelerometer tilt precision

//thumb stick pin numbers
#define RIGHTH  0
#define RIGHTV  1
#define LEFTH   2
#define LEFTV   3

long long shiftIn(unsigned long long cv, unsigned char val, unsigned char n){
    cv <<= n;
    cv |= val;
    return cv;
}

//returns ADC value of thumb stick at pinNum
unsigned char getThumbStickVal(unsigned char pinNum){
    Set_A2D_Pin(pinNum);
    unsigned char tsTruncate = ADC_PRECISION - TS_PRECISION;
    static unsigned char i;
    for(i = 0; i < 24; i++) asm("nop");
    unsigned char thumbStickVal = ADC >> tsTruncate;
    return thumbStickVal;
}

//inverts thumb stick
unsigned char invertThumbStick(unsigned char ts){
    unsigned char maxValTS = (1 << TS_PRECISION) - 1;
    return maxValTS - ts;
}

enum ControllerState {controller_INIT, controller_WAIT} controllerState;
signed char xTilt, yTilt, zTilt;                //accelerometer values
unsigned char rightH, rightV, leftH, leftV;     //thumb stick values
unsigned short buttons;                         //button vector
unsigned long long controllerVector;            //64bit data vector holding all the measured values

void controllerInit(){
    controllerState = controller_INIT;
}

void controllerTick(){
    //Actions
    switch(controllerState){
        case controller_INIT:
            xTilt = 0;
            yTilt = 0;
            zTilt = 0;
            rightH = 0;
            rightV = 0;
            leftH = 0;
            leftV = 0;
            buttons = 0;
            controllerVector = 0;
        break;
        
        case controller_WAIT:
        //get accelerometer values
        xTilt = accReadAddress(LIS3DH_REG_OUT_X_H);
        yTilt = accReadAddress(LIS3DH_REG_OUT_Y_H);
        zTilt = accReadAddress(LIS3DH_REG_OUT_Z_H);
        
        //get thumb stick values
        rightH = getThumbStickVal(RIGHTH);
        rightV = getThumbStickVal(RIGHTV);
        leftH  = getThumbStickVal(LEFTH);
        leftV  = getThumbStickVal(LEFTV);
        
        //invert horizontal axis
        //I do this because of the way the sticks are wired on the breadboard
        rightH = invertThumbStick(rightH);
        leftH  = invertThumbStick(leftH);
        
        //construct controller vector
        controllerVector = shiftIn(controllerVector, xTilt, TILT_PRECISION);
        controllerVector = shiftIn(controllerVector, yTilt, TILT_PRECISION);
        controllerVector = shiftIn(controllerVector, zTilt, TILT_PRECISION);
        controllerVector = shiftIn(controllerVector, leftV,  TS_PRECISION);
        controllerVector = shiftIn(controllerVector, leftH,  TS_PRECISION);
        controllerVector = shiftIn(controllerVector, rightV, TS_PRECISION);
        controllerVector = shiftIn(controllerVector, rightH, TS_PRECISION);
        
        //send values
        while(!USART_IsSendReady(0));
        USART_Send(leftH, 0);
        while(!USART_HasTransmitted(0));
        USART_Flush(0);

        break;
    }

    //Transitions
    switch(controllerState){
        case controller_INIT:
        controllerState = controller_WAIT;
        break;
        
        case controller_WAIT:
        break;
        
        default:
        controllerState = controller_INIT;
        break;
    }
}

void controllerTask()
{
    controllerInit();
    for(;;)
    {
        controllerTick();
        int tickRate = 1000 / TICKRATE;
        vTaskDelay(tickRate);
    }
}

void startController(unsigned portBASE_TYPE Priority)
{
    xTaskCreate(controllerTask, (signed portCHAR *)"controllerTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

int main(void)
{
    DDRA = 0x00; PORTA = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    
    initUSART(0);
    accBegin();
    
    A2D_init();
    
    USART_Flush(0);
    
    //Start Tasks
    startController(1);
    //RunSchedular
    vTaskStartScheduler();
    
    
    
    return 0;
}