
//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Little Blinky
//******************************************************************************
// This is our second lab demonstrating how to selectively control bits, and
// how to delay. 
//******************************************************************************

#include <avr/io.h>            // for accessing IO
#include <util/delay.h>        // for _delay_us() 

int main (void)
{
    DDRB = 1;                  // set pin B0 to output
    while(1)                   // do this forever. Never exit from main() in an AVR!
    {
        PORTB ^= 1;            // toggle bit B0 using XOR. 
        _delay_us(500000);     // wait 1/2 second (500,000 micro seconds)
    }
}
