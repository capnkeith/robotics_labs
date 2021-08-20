//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// LED lab 3, controlled steps.
// in this lab we are setting up 4 different steps and using them to light LEDs.
// our LEDs are on PORTB pins 0, 1, 2, and 3. Use a current limiting resistor 
// (somewhere beteen about 1K and 50k) in seriese with each LED. Hook the GND
// side of the LED to ground and the + side to the micro controller pins. The 
// circuit should look like this:
//
//   +------------+
//   |atmega328p  |
//   |            +- B0 ---|>---/\/\/\-GND
//   |            +- B1 ---|>---/\/\/\-GND
//   |            +- B2 ---|>---/\/\/\-GND
//   |            +- B3 ---|>---/\/\/\-GND
//   |            |
//   +------------+
//
// A different LED "-|>-" above, will light with each step. looking like movement.
// later on we will use this same techneque to actuall move motors and drive stuff
// fun stuff....
//******************************************************************************
#include <avr/io.h>         // include to access port IO
#include <util/delay.h>     // for _delay_us() 

uint8_t steps[] = { 0b0001, 0b0010, 0b0100, 0b1000 }; // each step lights a different bit, steps[0] turns on B0 ... step[3] B3.

int main (void)             // decare the main function. All projects have this it is called first.
{                   
    DDRB = 0b1111;          // B0, B1, B2, B3 are outputs.
    uint8_t i=0;            // start i at 0 step 0, or B0 on.
    while(1)
    {
        PORTB = steps[i];   // set the B port for step i.
        i++;                // increment i by 1.
        if (i==4) i=0;      // if i is 4 (past the end of steps), set it back to 0.
        _delay_us(500000);  // wait for 500,000 micro seconds, or one half second.   
    }
}

