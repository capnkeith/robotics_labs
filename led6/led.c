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
#include <avr/io.h>             // include to access port IO
#include <avr/interrupt.h>      // include for the timer interrupt

uint8_t steps[] = { 1,2,4,8 };  // this is an array of 4 steps, numbered 0,1,2,and 3. Their values are 1,2,4,and 8. 
                                // this will make 1 led light each step. You can do any pattern, what do you think
                                // this one looks like? stpes[]={14,13,11,7}

volatile uint32_t ticks=0;

void init_timer(void)                 //this function starts the periodic timer below (ISR function). 
{
    /* start timer */
	OCR2A = 62;
	TCCR2A |= (1 << WGM21);
	TIMSK2 |= (1 << OCIE2A);
	TCCR2B |= (1 << CS21);
	sei();
}


/* This is an interrupt service routime. After you call init_timer() function this will be called very frequently
   we don't need to worry about the details now, just know this is called like 50,000 times per second, and all it
   does is increment the 'ticks' count. This variable is declared 'volatile' and it is global. This is the only
   way I want you to interface with the interrupt service routines for now. This is safer.
*/

ISR (TIMER2_COMPA_vect)      
{
    ticks++;
}

int main (void)        // decare the main function. All projects have this it is called first.
{                   
    DDRB = 0b1111;     // B0, B1, B2, B3 are outputs. 15 = 0xf hex = 0b1111 binary.
    uint8_t i=0;       // current step starts at 0, 1, 2, 3, back to 0. 
    uint8_t button_in; // using B6 as our 1 button input. 
    uint8_t last=0;    // last button value to detect button press/release.
    uint32_t down=0;   // indicates a button is held.

    init_timer();      // start a periodic timer for debouncing.
    uint32_t last_ticks = 0;
    while(1)
    {
        if ( ticks != last_ticks )
        {
            last_ticks = ticks;
            uint8_t button_in = PINB & (1<<6); // read PIN B6. get the button_in pressed value from the pin.
            if (( button_in == last )&&button_in)
            {
                if ((!down))
                {
                    down=ticks;
                    if (direction == CCW)
                        i = (i>0)?i-1:3;
                    if (directoin == CW)
                        i = (i<4)?i+1:0;
                }
                else if (ticks-down == 20000)
                {
                    direction = (direction == CCW)?CW:CCW;
                }
            }
            else
            {
                down = 0;
                last = button_in;
            }
            PORTB = steps[i];
        }
    }    
}

