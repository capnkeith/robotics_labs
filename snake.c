#define __PROG_TYPES_COMPAT__
#include <avr/pgmspace.h>
#include <avr/io.h>
#include "uTFT_ST7735.h"

void myDelay(int16_t ms);

#define red Color565(0xff,0,0)
#define green Color565(0x0,0xff,0)
#define blue Color565(0x0,0,0xff)


uint16_t head[5][5] = { 
	{ 0,     0,     red,  red,   0 },	
	{ 0,     red,   red,  green, green },	
	{ red,   red,   red,  green, green },	
	{ 0,     red,   red,  red,   red },	
	{ 0,     0,     red,  red,   0 }	
};

uint16_t head2[5][5] = { 
	{ 0,       0,     red,  red,   0 },	
	{ red,     red,   red,  green, green },	
	{ 0,       0,     0,    green, green},	
	{ red,     red,   red,  red,   red },	
	{ 0,       0,     red,  red,   0 }	
};

uint16_t head3[5][5] = { 
	{ red,     0,     0,    red,   0 },	
	{ 0,       red,   red,  green, green },	
	{ 0,       0,     0,    green, green},	
	{ 0,       red,   red,  red,   red },	
	{ red,     0,     red,  red,   0 }	
};

uint16_t *cycles[] = {head, head2, head3, NULL};


enum events {
	NO_EVENT = 0,
	LT_PRESS,
	RT_PRESS,
};

enum states {
	START = 0,
	RUN = 0
};

volatile uint32_t ticks=0;

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


void run_state()
{
	switch(state) {
		case START:
			draw_splashscreen();
			next_state = RUN;
			next_time = ticks + 10000;
			break;
		case RUN:
			update_position();
			process_events();
			draw();
			break;
	}
} 

int main(void){
	// init the 1.8 lcd display

	init();
	init_timer();
	fillScreen(0);
	DDRD=0;
	PORTD=3;
	uint8_t event=NO_EVENT;

	while(1){
		if (!(PIND & 1)) {
			event = LT_PRESS;
		} else {
			setCursor(55,20);
			print("d0 not pressed\n");
		}
		if (!(PIND & 2)) {
			event = RT_PRESS;
		} else {
			print("d1 not pressed\n");
		}
		run_state();

		int y;
		int x;
		for (y=0; y<5; y++) {
		    for (x=0; x<5; x++) {
                        drawPixel(x,y,head[x][y]);		
		    }
                }		
	}
}
