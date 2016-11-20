#ifndef ACCELEROMETER_H_
#define ACCELEROMETER_H_

#include "bit.h"

#define ACC_PORT PORTC
#define ACC_PIN  PINC
#define ACC_DDR  DDRC

#define ACC_CS  7 // SS
#define ACC_SDO 6 // MISO
#define ACC_SDI 5 // MOSI
#define ACC_SPC 4 // SCK

/*=========================================================================
    I2C ADDRESS/BITS
    -----------------------------------------------------------------------*/
    #define LIS3DH_DEFAULT_ADDRESS  (0x18)    // if SDO/SA0 is 3V, its 0x19
/*=========================================================================*/

#define LIS3DH_REG_STATUS1       0x07
#define LIS3DH_REG_OUTADC1_L     0x08
#define LIS3DH_REG_OUTADC1_H     0x09
#define LIS3DH_REG_OUTADC2_L     0x0A
#define LIS3DH_REG_OUTADC2_H     0x0B
#define LIS3DH_REG_OUTADC3_L     0x0C
#define LIS3DH_REG_OUTADC3_H     0x0D
#define LIS3DH_REG_INTCOUNT      0x0E
#define LIS3DH_REG_WHOAMI        0x0F
#define LIS3DH_REG_TEMPCFG       0x1F
#define LIS3DH_REG_CTRL1         0x20
#define LIS3DH_REG_CTRL2         0x21
#define LIS3DH_REG_CTRL3         0x22
#define LIS3DH_REG_CTRL4         0x23
#define LIS3DH_REG_CTRL5         0x24
#define LIS3DH_REG_CTRL6         0x25
#define LIS3DH_REG_REFERENCE     0x26
#define LIS3DH_REG_STATUS2       0x27
#define LIS3DH_REG_OUT_X_L       0x28
#define LIS3DH_REG_OUT_X_H       0x29
#define LIS3DH_REG_OUT_Y_L       0x2A
#define LIS3DH_REG_OUT_Y_H       0x2B
#define LIS3DH_REG_OUT_Z_L       0x2C
#define LIS3DH_REG_OUT_Z_H       0x2D
#define LIS3DH_REG_FIFOCTRL      0x2E
#define LIS3DH_REG_FIFOSRC       0x2F
#define LIS3DH_REG_INT1CFG       0x30
#define LIS3DH_REG_INT1SRC       0x31
#define LIS3DH_REG_INT1THS       0x32
#define LIS3DH_REG_INT1DUR       0x33
#define LIS3DH_REG_CLICKCFG      0x38
#define LIS3DH_REG_CLICKSRC      0x39
#define LIS3DH_REG_CLICKTHS      0x3A
#define LIS3DH_REG_TIMELIMIT     0x3B
#define LIS3DH_REG_TIMELATENCY   0x3C
#define LIS3DH_REG_TIMEWINDOW    0x3D

/* Used with register 0x2A (LIS3DH_REG_CTRL_REG1) to set bandwidth */
typedef enum
{
    LIS3DH_DATARATE_400_HZ     = 0b0111, //  400Hz
    LIS3DH_DATARATE_200_HZ     = 0b0110, //  200Hz
    LIS3DH_DATARATE_100_HZ     = 0b0101, //  100Hz
    LIS3DH_DATARATE_50_HZ      = 0b0100, //   50Hz
    LIS3DH_DATARATE_25_HZ      = 0b0011, //   25Hz
    LIS3DH_DATARATE_10_HZ      = 0b0010, // 10 Hz
    LIS3DH_DATARATE_1_HZ       = 0b0001, // 1 Hz
    LIS3DH_DATARATE_POWERDOWN  = 0,
    LIS3DH_DATARATE_LOWPOWER_1K6HZ  = 0b1000,
    LIS3DH_DATARATE_LOWPOWER_5KHZ  =  0b1001,

} lis3dh_dataRate_t;

void accStartTransaction(){ACC_PORT = SetBit(ACC_PORT, ACC_CS, 0);}
void accEndTransaction(){ACC_PORT = SetBit(ACC_PORT, ACC_CS, 1);}

//sends a single bit to the accelerometer
//b: value being sent
void accSendBit(unsigned char b){
    //set the bit to be sent
    ACC_PORT = SetBit(ACC_PORT, ACC_SDI, b);
    
    //pulse clock
    ACC_PORT = SetBit(ACC_PORT, ACC_SPC, 0);
    ACC_PORT = SetBit(ACC_PORT, ACC_SPC, 1);
}

//transmits an address. Also specifies Read(1) or Write(0)
void accTransmitAddress(unsigned char addr, unsigned char rw){
    unsigned char data = 0;
    data = SetBit(data, 7, rw); //set read or write mode
    data = SetBit(data, 6, 1);  //sequential reads increment address
    data |= addr;
    
    int i = 8;
    while(i-->0){
        unsigned char bit = GetBit(data, i);
        accSendBit(bit);
    }
}

//NOT TESTED
//addr: address you want to read from
//data: array of data to write
//n:    number of addresses to write to. Addresses are written sequentially
//returns the data at address addr
void accWrite(unsigned char addr, unsigned char data[], unsigned char n){
    accStartTransaction();
    
    accTransmitAddress(addr, 0); //also enables write
    
    //write the data
    int i;
    for(i = 0; i < n; i++){
        unsigned char d = data[i];
        int j = 8;
        while(j-->0){
            unsigned char bit = GetBit(d, i);
            accSendBit(bit);
        }
    }
    
    accEndTransaction();
}

//NOT TESTED
void accWriteAddress(unsigned char addr, unsigned char data){
    unsigned char d[1];
    d[0] = data;
    accWrite(addr, d, 1);
}

//addr: address you want to read from
//data: array where the data will be stored
//n:    number of addresses to read. Each address is a byte. Addresses are read sequentially
//returns the data at address addr
signed char* accRead(unsigned char addr, signed char data[], unsigned char n){
    accStartTransaction();
    accTransmitAddress(addr, 1);
    
    //read the addresses
    int i;
    for(i = 0; i < n; i++){
        signed char d = 0;
        int j = 8;
        while(j-->0){
            accSendBit(0); //pulses the clock
            unsigned char bit = GetBit(ACC_PIN, ACC_SDO);
            d = bit ? d | (1 << j) : d & ~(1 << j);
        }
        data[i] = d;
    }
    
    accEndTransaction();
    return data;
}

signed char accReadAddress(unsigned char addr){
    signed char data[1] = {0};
    signed char* d = data;
    d = accRead(addr, d, 1);
    return data[0];
}

void accBegin(){
    
    // Set DDRB to have MOSI, SCK, and SS as output and MISO as input
    ACC_DDR = SetBit(ACC_DDR, ACC_CS, 1);
    ACC_DDR = SetBit(ACC_DDR, ACC_SDI, 1);
    ACC_DDR = SetBit(ACC_DDR, ACC_SPC, 1);
    ACC_DDR = SetBit(ACC_DDR, ACC_SDO, 0);
    
    //ACC_PORT = (ACC_PORT & 0x0F) | (1 << ACC_CS) | (1 << ACC_SDO) | (1 << ACC_SPC);
    
    ACC_PORT = SetBit(ACC_PORT, ACC_CS, 1);
    ACC_PORT = SetBit(ACC_PORT, ACC_SDO, 1);
    ACC_PORT = SetBit(ACC_PORT, ACC_SDI, 0);
    ACC_PORT = SetBit(ACC_PORT, ACC_SPC, 1);
    
    //enable all X, Y, and Z axis. Datarate is 400HZ
    unsigned char REGCTRL1 = 0x07 | (LIS3DH_DATARATE_400_HZ << 4);
    accWriteAddress(LIS3DH_REG_CTRL1, REGCTRL1);
    
    ////enable high resolution mode
    //unsigned char REGCTRL4 = 0x08;
    //accWriteAddress(LIS3DH_REG_CTRL4, REGCTRL4);
}

#endif /* ACCELEROMETER_H_ */