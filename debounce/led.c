//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Stepper LED
// 
// In this example we are using LEDs to learn how to drive steppert motors and
// debounce switches.
//
// In this example we want to make the LEDs step exactly 1 position for each
// press of the switch. This does not work like this, do you know why?
//
// HINT: you need to debunce the switch.
//
//******************************************************************************
#include <avr/io.h> // include to access port IO
#include <avr/interrupt.h>
volatile uint32_t ticks=0;
uint32_t last_tick=0;

void init_timer(void)
{
   /* start timer */
    OCR2A = 62;
    TCCR2A |= (1 << WGM21);
    TIMSK2 |= (1 << OCIE2A);
    TCCR2B |= (1 << CS21);
    ticks = 0;
    sei();
}

ISR (TIMER2_COMPA_vect)
{
    ticks++;
}



uint8_t steps[] = { 1,2,4,8 };

int main(void)     // decare the main function. All projects have this it is called first.
{                   
    DDRB = 0b00001111;      // B0, B1, B2, B3 are outputs.
    PORTB = 0;
    uint8_t state=0;      
    init_timer();
    while(1)
    {
        uint8_t button_in = PINB & (1<<6); // read PIN B6.
        if ((ticks-last_tick) >50000)
        {
            i++;
            if ( i==4 ) i = 0;
            last_tick = ticks;
            PORTB = steps[i];
        } 
        if ( button_in )                   // if the button is pressed 
        {
            i++;                           // advance to the next step
            if (i==4) i=0;                 // after step 3 (0-3) back to step 0.
        }
    }
}

