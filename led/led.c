//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Hello LED
// 
// In this first lab we are doing port output. We are using port B0 in this 
// example. This is a good starting port for any project
//******************************************************************************
#include <avr/io.h> // include to access port IO

int main (void)     // decare the main function. All projects have this it is called first.
{                   
    DDRB = 1;       // DDRB means 'Data Direction for port B'. Here we make B0 an output.
    PORTB = 1;      // PORTB sets the output for B pins. This line turns port B0 on.
    while(1);       // This loops forever. MAKE SURE TO NEVER return from main.
}
