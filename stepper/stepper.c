//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Stepper Motor Lab1. This project looks much like LED lab 3.
// This lab is designed to use a 28BYJ-48 unipolar stepper motor and a 
// ULN2803 stepper motor driver chip.
// 
// Here is the LED curcuit from LED lab 3:
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
// Add in the driver chip keeping the LEDs also. The driver chip is hooked
// up like this:
// 
//   +------------+         +------------+               Stepper Motor
//   |atmega328p  |         | ULN2803    |                  +--+
//   |            |         |            |                  |[]|--red-VCC
//   |            +- B0 --1-+            +-10-------orange--|[]|
//   |            +- B1 --2-+            +-11-------yellow--|[]|
//   |            +- B2 --3-+            +-12-------pink----|[]|
//   |            +- B3 --4-+            +-13-------blue----|[]|
//   |            |       5-+            +-14               +--+
//   +------------+       6-+            +-15           
//                        7-+            +-16   
//                        8-+            +-17  
//                 GND----9-+            +-18---VCC
//                          -------------
//
// Hook up both pin1 of the ULN2803 and the positive end of the LED directly to 
// pin B0 of the ATMEG328P. Do the same for pins 2,3,and 4 of the ULN2803.
// The input voltage (pin 18) of the ULN2803 does not need to be VCC it can be 
// some other voltage like 12V for a 12V stepper for instance. For our lab just
// hook it up to VCC. 
//
// NOTE: you need a power supply to drive this stepper motor. Do not drive this
// just off the programmer!
//
//******************************************************************************
#include <avr/io.h>         // include to access port IO
#include <util/delay.h>     // for _delay_us() 

uint8_t steps[] = {0b0001, 0b0010, 0b0100, 0b1000}; //full steps. Minimum delay per step is about 2000us.
//uint8_t steps[] = {0b1000,0b1100,0b0100,0b0110,0b0010,0b0011,0b0001,0b1001}; // half steps. You can delay as little as 900us with half-steps.
//uint8_t steps[] = {0b1010, 0b0110, 0b0101, 0b1001}; // bipolar

int main (void)                     // decare the main function. All projects have this it is called first.
{                   
    DDRB = 0b1111;                  // B0, B1, B2, B3 are outputs.
    uint8_t i=0;
    while(1)
    {
        PORTB = steps[i];
        i++;
        if (i==sizeof(steps)) i=0; // If I use sizeof instead of 8 then it works if I change steps[] to full steps...
        _delay_us(50000);           // here you cannot see the LEDs at all because they are moving too fast.
                                   // this is maximum stpeed for the stepper motors we use (28BYJ-48). 
                                   // if you delay much less, the motor will lock up instead of spin. This will
                                   // happen at different points for different loads. Experiment with this.
    }
}

