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

//bit functions 
#include "bit.h"

//drivers
#include "A2D.h"
#include "accelerometer.h"
#include "usart_ATmega1284.h"
 
//FreeRTOS include files 
#include "FreeRTOS.h" 
#include "task.h" 
#include "croutine.h"


#define TICKRATE 20

enum ControllerState {controller_INIT, controller_WAIT} controllerState;
signed char xTilt, yTilt, zTilt;
unsigned char rightH, rightV, leftH, leftV;
unsigned long long controllerVector;

void controllerInit(){
    controllerState = controller_INIT;
}

void controllerTick(){
    //Actions
    switch(controllerState){
        case controller_INIT:
        controllerVector = 0;
        xTilt = 0;
        yTilt = 0;
        zTilt = 0;
        break;
        
        case controller_WAIT:
        //get accellerometer values
        xTilt = accReadAddress(LIS3DH_REG_OUT_X_H);
        yTilt = accReadAddress(LIS3DH_REG_OUT_Y_H);
        zTilt = accReadAddress(LIS3DH_REG_OUT_Z_H);
        
        //get thumb stick values
        Set_A2D_Pin(0); rightH = ADC >> 2;
        Set_A2D_Pin(1); rightV = ADC >> 2;
        Set_A2D_Pin(2); leftH  = ADC >> 2;
        Set_A2D_Pin(3); leftV  = ADC >> 2;
        
        //wait for USART to be ready
        while(!USART_IsSendReady(0));
        //send data
        USART_Send(rightV, 0);
        //wait for transmission to complete
        while(!USART_HasTransmitted(0)){}
        //flush USART
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