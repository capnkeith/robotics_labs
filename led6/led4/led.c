//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Stepper LED
// 
// In this example we are using LEDs to learn how to drive stepper motors and
// debounce switches.
//
// In this example we want to make the LEDs step exactly 1 position for each
// press of the switch. This does not work like this, do you know why?
//
// HINT1: First try adding a pull down resistor to the input pin to keep it 'low'
// and tie the switch to VCC.
// 
// it works differently but still does not work, do you know why?
// HINT: you need to debunce the switch. Any switch is imperfect so instead of 
// one press you get lots. Do something to fix this yourself, else see debounce 
// lab. HINT: there are hardware and software solutions. Any one is fine.
//
//******************************************************************************
#include <avr/io.h> // include to access port IO

uint8_t steps[] = { 0b1, 0b10, 0b100, 0b1000 };  // this is an array of 4 steps, numbered 0,1,2,and 3. Their values are 1,2,4,and 8. 
                                // this will make 1 led light each step. You can do any pattern, what do you think
                                // this one looks like? stpes[]={14,13,11,7}

int main (void)        // decare the main function. All projects have this it is called first.
{                   
    DDRB = 0b1111;     // B0, B1, B2, B3 are outputs. 15 = 0xf hex = 0b1111 binary.
    uint8_t i=0;       // current step starts at 0, 1, 2, 3, back to 0. 
    uint8_t button;    // using B6 as our 1 button input. 
    uint8_t down;      // fire only when button is first pressed.
    while(1)
    {
        uint8_t button = PINB & (1<<6); // read PIN B6. get the button_in pressed value from the pin.
        if (button)
        {
            if (!down)
            {
                i++;
                if (i==4) i=0;                 // after step 3 (0-3) back to step 0.
                down = 1;
            }
        }
        else
        {
            down=0;
        }
        PORTB = steps[i];
    }    
}

