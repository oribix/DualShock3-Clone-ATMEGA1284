#ifndef A2D_H_
#define A2D_H_

//initializes the ADC Pins for use
void A2D_init() {
    // ADEN: Enables analog-to-digital conversion
    // ADSC: Starts analog-to-digital conversion
    // ADATE: Enables auto-triggering, allowing for constant
    //	    analog to digital conversions.
    ADCSRA |= (1 << ADEN) | (1 << ADSC) | (1 << ADATE);
}

//Selects input on PORTA for A2D conversion
//default is A0
void Set_A2D_Pin(unsigned char pinNum) {
    ADMUX = (pinNum <= 0x07) ? pinNum : ADMUX;
    // Allow channel to stabilize
    static unsigned char i = 0;
    for ( i=0; i<15; i++ ) { asm("nop"); }
}

#endif /* A2D_H_ */