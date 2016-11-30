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

//returns an unsigned short containing the status of each button
unsigned short getButtonVector(){
    unsigned char up, down, left, right, A, B, X, Y, L1, L2, R1, R2, sl, st;
    
    //d-pad
    right   = GetBit(PIND, 4);
    up      = GetBit(PIND, 5);
    left    = GetBit(PIND, 6);
    down    = GetBit(PIND, 7);
    
    //shoulder buttons
    L1 = GetBit(PINB, 7);
    L2 = GetBit(PIND, 2);
    R1 = GetBit(PINB, 5);
    R2 = GetBit(PINB, 4);
    
    //select start
    sl = GetBit(PIND, 3);
    st = GetBit(PINB, 3);
    
    //ABXY ... DualShock 3 does not have ABXY but I cant name it ^OX[]
    A = GetBit(PINB, 0);  //O
    B = GetBit(PINA, 4);  //X
    X = GetBit(PINB, 1);  //^
    Y = GetBit(PINB, 2);  //[]
    
    //construct button vector
    unsigned short bv = 0;
    bv = (bv | B)     << 1;
    bv = (bv | Y)     << 1;
    bv = (bv | sl)    << 1;
    bv = (bv | st)    << 1;
    
    bv = (bv | up)    << 1;
    bv = (bv | down)  << 1;
    bv = (bv | left)  << 1;
    bv = (bv | right) << 1;
    
    bv = (bv | A)     << 1;
    bv = (bv | X)     << 1;
    bv = (bv | L1)    << 1;
    bv = (bv | R1)    << 1;
    
    bv = (bv | L2)    << 1;
    bv = (bv | R2)    << 2;
    
    return ~bv;
}

//returns the full 64-bit controller vector
unsigned long long getControllerVector(){
    signed char xTilt, yTilt, zTilt;                //accelerometer values
    unsigned char rightH, rightV, leftH, leftV;     //thumb stick values
    unsigned short buttons;                         //button vector
    unsigned long long controllerVector = 0;        //64bit data vector
    
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
    
    //get buttons
    buttons = getButtonVector();
    
    //construct controller vector
    controllerVector = shiftIn(controllerVector, xTilt, TILT_PRECISION);
    controllerVector = shiftIn(controllerVector, yTilt, TILT_PRECISION);
    controllerVector = shiftIn(controllerVector, zTilt, TILT_PRECISION);
    controllerVector = shiftIn(controllerVector, leftV,  TS_PRECISION);
    controllerVector = shiftIn(controllerVector, leftH,  TS_PRECISION);
    controllerVector = shiftIn(controllerVector, rightV, TS_PRECISION);
    controllerVector = shiftIn(controllerVector, rightH, TS_PRECISION);
    controllerVector <<= 16;
    controllerVector |= buttons;

    return controllerVector;
}

enum ControllerState {controller_INIT, controller_WAIT} controllerState;
static unsigned char duration;

void controllerInit(){
    controllerState = controller_INIT;
}

void controllerTick(){
    //Actions
    switch(controllerState){
        case controller_INIT:
        break;
        
        case controller_WAIT:
        
        if(USART_HasReceived(0)){
            //get rumble duration
            //also helps to synchronize USART
            duration = USART_Receive(0);
            
            //long long is 8 bytes... send 8 bytes
            unsigned long long cv = getControllerVector();
            unsigned char i = 8;
            while(i-->0){
                unsigned char var = (cv >> (i * 8)) & 0xFF;
                USART_Send(var, 0);
            }
            while(!USART_HasTransmitted(0));
        }
        
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
    
    DDRB = 0x00; PORTB = 0xFF;
    DDRC = 0xFF; PORTC = 0x00;
    DDRD = 0x00; PORTD = 0xFF;
    
    initUSART(0);
    accBegin();
    
    A2D_init();
    DDRA = DDRA & 0x0F; PORTA |= 0xF0;
    USART_Flush(0);
    
    //Start Tasks
    startController(1);
    //RunSchedular
    vTaskStartScheduler();
    
    
    
    return 0;
}