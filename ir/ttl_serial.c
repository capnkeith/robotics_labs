
//******************************************************************************
// Copyright 2022 Seth Keith, all rights reserved.
//******************************************************************************

#include <avr/io.h> 
#include <util/delay.h>
#include <stdio.h>
#include "ttl_serial.h"


// init the UART. 
void uart_init(uint16_t baud)
{
   uint16_t ubrr = F_CPU/16/baud-1;     // compute ubrr from baud and your cpu frequence 
   UBRR0H = (uint8_t)ubrr>>8;           // baud rate high order byte in UBRR0H 
   UBRR0L = (uint8_t)ubrr;              // baud rate low order byte in UBRR0L 
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);      // enable transmit and receive 
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);     // 8 data bits 2 stop bits 
}

void uart_send(char data)
{
    while(!(UCSR0A & (1<<UDRE0)));      /* wait until the register is ready */
    UDR0 = data;                        /* write data to the UDR0 registery to be transmitted */
}

uint8_t uart_receive(void)    
{
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

void uart_send_string(char *string)     
{
    while(*string != 0)
    {  
        uart_send(*string);    
        string++;
    }        
}
