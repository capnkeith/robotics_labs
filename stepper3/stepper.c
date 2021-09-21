
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
    ticks++;                            // add 1 to ticks each time called.
}


int main(void)                  
{                   
    DDRB = 0b00001111;                          // B0, B1, B2, B3 are outputs, B6 is our input.
    init_timer();                               // this starts the ISR running periodically.
    uint32_t last_motor_tick=ticks;             // last_tick records elapsed ticks when the motor started
    uint32_t last_button_tick=ticks;            // last_tick records elapsed ticks when the button was pressed
    uint8_t step = 0;                           // step is the LED in steps[] to light
    uint8_t event_triggered = 0;                // track if a button press has been triggered.
    uint8_t last_button = 0;                    // track the last input value read.
    uint8_t direction = 0;

#define MOTOR_DELAY 50
#define DEBOUNCE_DELAY 1 
    while(1)
    {
        if ( (ticks-last_motor_tick) >= MOTOR_DELAY)
        {
            last_motor_tick = ticks;
            if (direction)
                step = (step==sizeof(steps))?0:step+1;
            else
                step = (step==0)?sizeof(steps)-1:step-1;
            PORTB = steps[step];                 // drive the pins with the current step.
        } 
           
        if ( (ticks-last_button_tick) >= DEBOUNCE_DELAY)
        { 
            last_button_tick = ticks;
            uint8_t button_in = PINB & (1<<6);      // read PIN B6 into button_in
            if (button_in != last_button)           // if button_in changed last tic
            {
                last_button = button_in;            // record the new value in last_button
                event_triggered = 0;                // reset the even trigger
            }
            else                                    // otherwise button same for 2 ticks
            {
                if (!event_triggered)               // if the event was not triggered yet
                {
                    event_triggered = 1;            // trigger the event. 
                    if (button_in)                  // if the button is pressed
                    {
                        direction = !direction;     // toggle direction
                    }
                }
            }
        }
    }
}

