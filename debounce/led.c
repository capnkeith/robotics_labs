//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// De-Bounce 
// 
// In this lab we investigate one way of debounching a switch using a timer. 
//
// CAUTION: we are introducing a timer here which uses an interrupt service 
// routine. Using interrupt service routines can make your program impossible
// to debug. Be careful not to stray from my recipie. I have only a single 
// variable called 'ticks' in my interrupt service routine. Make sure this 
// variable is declared 'volatile'. Do not try and put more code inside the
// ISR function, you will regret it if you do.
//
// The way the ISR (interrupt service routine) 'function' works is once you
// set up the timer, sometime later on, the timer will trigger. This causes
// an interrupt which will 'interrupt' your main() program right in the middle
// of whatever it is doing and jump to the ISR() function. When ISR is done, 
// your main() program will be resumed where it was interrupted. If you try 
// and do much work in ISR() and use it in main() you will find it very 
// difficult to debug. DO NOT DO THIS.
//
// By limiting our exposure between main() and ISR() to a single volatile 
// variable called ticks, we simplify ISR interaction and make it relitively 
// safe to use.
//
// There are lots of problems with this design. For one thing is is really
// complex just for a single button. The other problem is busy waiting.
// When you wait in a tight while loop for the next tick, nothing else in
// the program (except the interrupt routine) can function. We will solve both 
// these problems by introducing state machine programming.
//
//******************************************************************************
#include <avr/io.h> 		// include to access port IO
#include <avr/interrupt.h>	// include for using timers.

volatile uint32_t ticks=0;
uint8_t steps[] = { 1,2,4,8 };

// init_timer()
//  start a timer interrupt periodically. 
void init_timer(void)
{
    cli();			// disable interrupts during timer init.
    OCR2A = 62;			// compare against 
    TCCR2A |= (1 << WGM21);     // CTC mode
    TIMSK2 |= (1 << OCIE2A);    // enable timer compare interrupt
    TCCR2B |= (1 << CS21);	// prescaler to 128
    ticks = 0;			// init ticks to 0
    sei();			// enable interrupts. Timer is now running.
}

// This is an interrupt service routine. It will be called every few
// micro seconds interrupting your main() function in the middle of whatever 
// it was doing. This can be very hard to debug. For this reason keep code in
// the ISR to a minium. Any shared variables between main() and ISR should be
// volatile.
ISR (TIMER2_COMPA_vect)
{
    ticks++;	     	        	// add 1 to ticks each time called.
}


int main(void)     		
{                   
    DDRB = 0b00001111;          		// B0, B1, B2, B3 are outputs, B6 is our input.
    init_timer();               		// this starts the ISR running periodically.
    uint32_t last_tick=ticks;   		// last_tick records elapsed ticks
    uint8_t step = 0;           		// step is the LED in steps[] to light
    uint8_t event_triggered = 0;		// track if a button press has been triggered.
    uint8_t last_button = 0;			// track the last input value read.
    while(1)
    {
        while (last_tick==ticks); 		// wait till the next tick.
        last_tick = ticks;			// update last_tick current ticks

        uint8_t button_in = PINB & (1<<6); 	// read PIN B6 into button_in
        if (button_in != last_button)		// if button_in changed last tic
        {
            last_button = button_in;		// record the new value in last_button
            event_triggered = 0;		// reset the even trigger
        }
        else 					// otherwise button same for 2 ticks
        {
            if (!event_triggered)		// if the event was not triggered yet
            {
                event_triggered = 1;		// trigger the event. 
		if (button_in)			// if the button is pressed
 		{
                    step++;			// then go to the next step
                    if (step == sizeof(steps))  // if past the last step
	                step = 0;               // go back to step 0.
		}
		else				// otherwise this is a button release
		{
	            			        // if you want a button relese action do it here.
		}
            }
        }
        PORTB = steps[step];			// light the LEDs with the current step.
    }
}

