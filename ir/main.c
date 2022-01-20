//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// IR lab 1.
//******************************************************************************
// 
// In this lab we will learn how interface with an IR detedtor in the most 
// primitive way, to learn the fundamentals of IR sensors and learn more about
// analog to digital conversion.
//
//******************************************************************************
// 
// Wiring:
//
// Start with a standard programable AVR setup with a UART connection. Attach
// the output of the IR detector to ADC pin 0 (pin #23). See the 'basic circuts'
// seciton for the 'photo transistor' curcuit to use. 
//
//                                         ATMEGA328P
//                                  +--------------------+
//     programmer reset ---------(1)| RESET              |
//     TTL serial TX-------------(2)| RX                 |
//     TTL serial RX-------------(3)| TX                 | 
//                                  |                    |
//                                  |                    |
//     programmer +5V------------(7)| VCC                |
//     programmer gnd------------(8)| GND                |
//                                  |                PC0 |(23)------ detector output
//                                  |                    |
//                                  |                SCK |(19)------ programmer SCK
//                                  |               MISO |(18)------ programmer MISO 
//                                  |               MOSI |(17)------ programmer MOSI
//     IR emitter power ------------| PB0                |
//                                  +--------------------+
//                 
//******************************************************************************
// 
// How does it work:
//
// The IR detector circuit will output close to +5V when there is no IR light
// detected. By turning on PB0 we light the IR emitter. As more and more IR 
// light from the emitter enters the detector the voltage at the detector output 
// will drop towards zero. We can measure this voltage with a techneque called 
// analog to digital conversion or A2D for short.
//
//******************************************************************************




#include <avr/io.h> 
#include <util/delay.h>
#include <stdio.h>
#include "ttl_serial.h"

uint16_t adc_read(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);
 
  // wait for conversion to complete
  // ADSC becomes ’0′ again
  // till then, run loop continuously
  while(ADCSRA & (1<<ADSC));
 
  return (ADC);
}

int main(void)
{
    uint16_t v;
    char buf[20];
    uart_init(9600);                             // init serial 9600 n,8,2
    ADMUX = (1<<REFS0);                          // set the reference voltage to be +5 internal
                                                 // set sample prescaler to 128 (8Mhz/128=64012 samples per sec)
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0); 
    DDRB |= 1;                                   // make B0 an output
    PORTB |= 1;                                  // turn on the emitter LED
    while(1)
    {
        v = adc_read(0);                         // read from ADC chanbnel 0 (ADC0)
        sprintf(buf,"v=%d\n\r",v);               // format output buffer
        uart_send_string(buf);                   // write it to the serial
    }
}
