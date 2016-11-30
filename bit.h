
// Permission to copy is granted provided that this header remains intact. 
// This software is provided with no warranties.

////////////////////////////////////////////////////////////////////////////////

#ifndef BIT_H
#define BIT_H

#define low  0
#define high 1

#define INPUT_PIN 0
#define OUTPUT_PIN 1

////////////////////////////////////////////////////////////////////////////////
//Functionality - Sets bit on a PORTx
//Parameter: Takes in a uChar for a PORTx, the pin number and the binary value 
//Returns: The new value of the PORTx
unsigned char SetBit(unsigned char pin, unsigned char number, unsigned char bin_value) 
{
	return (bin_value ? pin | (0x01 << number) : pin & ~(0x01 << number));
}

////////////////////////////////////////////////////////////////////////////////
//Functionality - Gets bit from a PINx
//Parameter: Takes in a uChar for a PINx and the pin number
//Returns: The value of the PINx
unsigned char GetBit(unsigned char port, unsigned char number) 
{
	return ( port & (0x01 << number) ) ? 1 : 0;
}

//pulses the given pin low then high.
//notice that port is a pointer
//make sure to pass in &PORTx, not PORTx
void pulse01(volatile uint8_t *port, unsigned char number){
    *port = SetBit(*port, number, 0);
    *port = SetBit(*port, number, 1);
}

//pulses the given pin high then low.
//notice that port is a pointer
//make sure to pass in &PORTx, not PORTx
void pulse10(volatile uint8_t *port, unsigned char number){
    *port = SetBit(*port, number, 1);
    *port = SetBit(*port, number, 0);
}

#endif //BIT_H