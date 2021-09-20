//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Stepper Motor Lab2. 
//
// In this lab we comine the timer from the debounce lab with the code from 
// stepper lab1, to drive the stepper without using a delay() funtion. Delay
// is bad because we cannot do anything else while we are delaying, and for our
// robot to work we need to do a lot of things at once.
//
//******************************************************************************
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
//******************************************************************************
#include <avr/io.h>         // include to access port IO
#include <util/delay.h>     // for _delay_us() 
#include <avr/interrupt.h>	// include for using timers.
volatile uint32_t ticks=0;

//uint8_t steps[] = {0b1100, 0b0110, 0b0011, 0b1001}; //full steps. Minimum delay per step is about 25 ticks.
uint8_t steps[] = {0b1000,0b1100,0b0100,0b0110,0b0010,0b0011,0b0001,0b1001}; // half steps. You can delay as 12 ticks.
//uint8_t steps[] = {0b1010, 0b0110, 0b0101, 0b1001}; // bipolar

// init_timer()
//  start a timer interrupt periodically. 
void init_timer(void)
{
    cli();                      // disable interrupts during timer init.
    OCR2A = 62;                 // compare against 
    TCCR2A |= (1 << WGM21);     // CTC mode
    TIMSK2 |= (1 << OCIE2A);    // enable timer compare interrupt
    TCCR2B |= (1 << CS21);      // prescaler to 128
    ticks = 0;                  // init ticks to 0
    sei();                      // enable interrupts. Timer is now running.
}

// This is an interrupt service routine. It will be called every few
// micro seconds interrupting your main() function in the middle of whatever 
// it was doing. This can be very hard to debug. For this reason keep code in
// the ISR to a minium. Any shared variables between main() and ISR should be
// volatile.
ISR (TIMER2_COMPA_vect)
{
    ticks++;                                // add 1 to ticks each time the ISR is called.
}
    
int main (void)                             // decare the main function. All projects have this it is called first.
{                   
    DDRB = 0b1111;                          // B0, B1, B2, B3 are outputs.
    uint8_t i=0;
    uint32_t last_ticks=0;
    init_timer();                           // start the timer interrupts. 
    while(1)
    {
#define MOTOR_DELAY 12                      // minimum delay to wait per step in ticks. 12 is fastest for 1/2 step, 25 for full step.
        if ( (ticks - last_ticks) > MOTOR_DELAY )
        {
            last_ticks = ticks;             // update last_ticks to ticks.
            i++;                            // show next state
            if (i==sizeof(steps)) i=0;      // wrap around i if i is larger than steps array. 
            PORTB = steps[i];               // set the B pins by the current step.
        }
    }
}

