//******************************************************************************
// Copyright 2021 Seth Keith, all rights reserved.
//******************************************************************************
// Hello Serial
//******************************************************************************
// Wiring: hook serial TX to atmega RX and serial RX to atmega TX. 
// Hookup ground of the seerial to your ground. 
//******************************************************************************
// Using this:
// After wiring, flash this program to your chip. Then open a serial terminal 
// program on the serial port for the TTL to USB converter or whaterver serial 
// port you are using. If you flash it again now, or power cycle the chip you 
// will see:
//    'Hello Serial' printed on your computer.
// When you press a key in the terminal window it will be echo what you pressed.
// 
// This is not a really good way to interface because you cannot do anyting else
// while you are waiting for the registers to be ready. We can get around this
// by polling.
//******************************************************************************
#include <avr/io.h> 
#include <util/delay.h>

unsigned char uart_receive( void );
void uart_send_string(char *StringPtr);
void uart_init( unsigned int ubrr);     
void uart_send( unsigned char data );


/* uart_init() configures the UART of your atmega328p to the baud rate indicated. */
void uart_init(uint16_t baud)
{
   uint16_t ubrr = F_CPU/16/baud-1;     /* compute ubrr from baud and your cpu frequence */
   UBRR0H = (uint8_t)ubrr>>8;           /* baud rate high order byte in UBRR0H */
   UBRR0L = (uint8_t)ubrr;              /* baud rate low order byte in UBRR0L */
   UCSR0B = (1<<RXEN0)|(1<<TXEN0);      /* enable transmit and receive */
   UCSR0C = (1<<USBS0)|(3<<UCSZ00);     /* 8 data bits 2 stop bits */
}

/* send a single byte from the atmega328 to the serial port */
void uart_send( unsigned char data )
{
	while(!(UCSR0A & (1<<UDRE0)));      /* wait until the register is ready */
	UDR0 = data;                        /* write data to the UDR0 registery to be transmitted */
}

/* receive a single byte from the serial port into the atmega328p */
unsigned char uart_receive(void)    
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
    uart_send_string("Hello Serial\n\r");   /* write Hello Serial. on a new line */
    uart_send_string("What you want?\n\r"); /* write What you want? on a new line */
	while(1)
	{
        char c = uart_receive();            /* get the next pressed key from the serial */
        if (c)                              /* if something was pressed */
        {
            uart_send_string("atmega says you Pressed: "); /* write atmega says you Pressed: then no newline */
            uart_send(c);	                /* finish by sending the key pressed */
            uart_send_string("\n\r");       /* and a new line and CR. */
        }
	}

}
