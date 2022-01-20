//******************************************************************************
// Copyright 2022 Seth Keith, all rights reserved.
//******************************************************************************
//
//******************************************************************************
#include <avr/io.h>             // include to access port IO
#include <avr/interrupt.h>      // include for using timers.
#include <stdio.h>

uint8_t uart_receive( void );
void uart_send_string(char *string);
void uart_init( uint16_t ubrr);     
void uart_send( char data );

volatile uint32_t ticks=0;
uint32_t last_tick=0;
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
void uart_send(char data)
{
    while(!(UCSR0A & (1<<UDRE0)));      /* wait until the register is ready */
    UDR0 = data;                        /* write data to the UDR0 registery to be transmitted */
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

void adc_start(uint8_t ch)
{
  // select the corresponding channel 0~7
  // ANDing with ’7′ will always keep the value
  // of ‘ch’ between 0 and 7
  ch &= 0b00000111;  // AND operation with 7
  ADMUX = (ADMUX & 0xF8)|ch; // clears the bottom 3 bits before ORing
 
  // start single convertion
  // write ’1′ to ADSC
  ADCSRA |= (1<<ADSC);
}


enum a2d_states 
{
    START_A2D=0,
    POLL_A2D,
    SHOW_A2D,
    IR_ON, 
    IR_OFF, 
    WAIT_UNTIL,
}; 

uint8_t state = START_A2D;
uint8_t next_state;
uint16_t result = 0;
char buf[20];
uint8_t adc_channel = 0;
uint32_t until_time;

#define MIN_ON_TICKS 2 
#define MIN_OFF_TICKS 2

void process(uint16_t delta)
{
    switch(state)
    {
        case IR_ON:
            PORTB |= 1;
            next_state = START_A2D;
            state = WAIT_UNTIL;
            until_time = ticks + MIN_ON_TICKS;
            break;
        case IR_OFF:
            PORTB &= ~1;
            next_state = IR_ON;
            state = WAIT_UNTIL;
            until_time = ticks + MIN_OFF_TICKS;
            break;
        case WAIT_UNTIL:
            if (until_time >= ticks)
            {
                state = next_state;
                next_state = -1;
            }
            break; 
        case START_A2D:
            adc_start(adc_channel);
            state = POLL_A2D;
            break;
        case POLL_A2D:
            if (!(ADCSRA & (1<<ADSC)))
            {
                result = ADC;
                state = SHOW_A2D;
            }
            break;
        case SHOW_A2D:
            sprintf(buf,"v=%d\n\r",result);
            uart_send_string(buf);
            state = IR_OFF;
            break;
        default:
            uart_send_string("Insane case.\n\r");
            break;
    }
}


int main(void)                  
{    
                   
    init_timer();                               // this starts the ISR running periodically.
    uart_init(9600);                        /* init serial to 9600 N,8,2 */
    ADMUX = (1<<REFS0);
    ADCSRA = (1<<ADEN)|(1<<ADPS2)|(1<<ADPS1)|(1<<ADPS0);

    DDRB = 1;
    while(1)
    {
        if ( last_tick != ticks )
        {
            process(ticks-last_tick);
            last_tick = ticks;
        }
    }
}
