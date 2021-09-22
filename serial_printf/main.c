//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Serial Printf 
//******************************************************************************
// Using this:
//  just call uart_init() with the proper baud rate, then using printf should
//  result in output on the serial terminal
//
// Serial Terminal from Mac or Linux:
//  screen /dev/usb.tty.xxxxx 9600
//
// Serial Terminal from Windows, try putty.
// 
//******************************************************************************

#include <stdio.h>
#include <avr/io.h> 
#include <util/delay.h>

uint8_t uart_receive( void );
void uart_send_string(char *string);
void uart_init( uint16_t ubrr);     
void uart_send( char data );
static FILE uart_stdout = FDEV_SETUP_STREAM(uart_send, NULL, _FDEV_SETUP_WRITE);


/* uart_init() configures the UART of your atmega328p to the baud rate indicated. */
void uart_init(uint16_t baud)
{
   uint16_t ubrr = F_CPU/16/baud-1;     /* compute ubrr from baud and your cpu frequence */
   UBRR0H = (uint8_t)ubrr>>8;           /* baud rate high order byte in UBRR0H */
   UBRR0L = (uint8_t)ubrr;              /* baud rate low order byte in UBRR0L */
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);      /* enable transmit and receive */
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);     /* 8 data bits 2 stop bits */
   stdout = &uart_stdout;
}

/* send a single byte from the atmega328 to the serial port */
void uart_send(char data)
{
    while(!(UCSR0A & (1<<UDRE0)));      /* wait until the register is ready */
    UDR0 = data;                        /* write data to the UDR0 registery to be transmitted */
    if (data == '\n')
    {
        uart_send('\r');
    }
}

/* receive a single byte from the serial port into the atmega328p */
uint8_t uart_receive(void)    
{
    while(!(UCSR0A & (1 << RXC0)));
    return UDR0;
}

/* send an entire string out to the serial */
void uart_send_string(char *string)     
{
    while(*string != 0)
    {  
        uart_send(*string);    
        string++;
    }        
}

int main(void)
{
    uart_init(9600);                        /* init serial to 9600 N,8,2 */
    printf("Hello Serial\n");   		/* write Hello Serial. on a new line */
    printf("What you want?\n"); /* write What you want? on a new line */
    while(1)
    {
        char c = uart_receive();            /* get the next pressed key from the serial */
        if (c)                              /* if something was pressed */
        {
            printf("atmega says you Pressed: %c\n", c); /* write atmega says you Pressed: then no newline */
        }
    }
}
