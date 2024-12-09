#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>

extern "C" {
#include "light_ws2812.h"

#define MAX_LEDS 120 

volatile uint32_t ticks = 1;
volatile uint32_t last_ticks = 1;

}

#if 0
// init_timer()
//  start a timer interrupt periodically. 
void init_timer(void)
{
    cli();                      // disable interrupts during timer init.
    OCR2A = 62;                 // compare against 
    TCCR2A |= (1 << WGM21);     // CTC mode
    TIMSK2 |= (1 << OCIE2A);    // enable timer compare interrupt
//    TCCR2B |= (1 << CS21);      // prescaler to 128
    TCCR2B |= (1 << CS20);      // prescaler to 128
    ticks = 1;                  // init ticks to 0
    sei();                      // enable interrupts. Timer is now running.
}
#else
void init_timer(void){
   TCCR1A = 0; //CTC mode 
   TCCR1B = 0B00000001; //prescaler=256,CTC mode
   TIMSK1 = 0B00000010; //Interrupt enable for OCR1A compare match
   OCR1A = 10;
//   OCR1A = 1;
}
#endif


// This is an interrupt service routine. It will be called every few
// micro seconds interrupting your main() function in the middle of whatever 
// it was doing. This can be very hard to debug. For this reason keep code in
// the ISR to a minium. Any shared variables between main() and ISR should be
// volatile.
ISR (TIMER1_COMPA_vect)
{
    ticks++;                            // add 1 to ticks each time called.
}

class led_t
{
public:

    led_t(uint8_t pin, uint8_t count, uint8_t red, uint8_t green, uint8_t blue, uint8_t gamma)
    {
        total = count * 3;
        led_pin = pin;
        red_level = red;
        green_level = green;
        blue_level = blue;
        gamma_level = gamma;

        uint16_t gamma_correct = ((uint16_t)green_level * gamma)/0xff;
        green_level = (uint8_t)gamma_correct;
        gamma_correct = ((uint16_t)red_level * gamma)/0xff;
        red_level = (uint8_t)gamma_correct;
        gamma_correct = ((uint16_t)blue_level * gamma)/0xff;
        blue_level = (uint8_t)gamma_correct;
    }

    void update_gamma(void) 
    {
        uint16_t gamma_correct = ((uint16_t)green_level * gamma_level)/0xff;
        green_level = (uint8_t)gamma_correct;
        gamma_correct = ((uint16_t)red_level * gamma_level)/0xff;
        red_level = (uint8_t)gamma_correct;
        gamma_correct = ((uint16_t)blue_level * gamma_level)/0xff;
        blue_level = (uint8_t)gamma_correct;
    }

    struct      cRGB led_workspace[MAX_LEDS];
    uint8_t     total;      // total number of leds
    uint8_t     led_pin;    // control pin for led chain;
    uint8_t     red_level;
    uint8_t     green_level;
    uint8_t     blue_level;
    uint8_t     gamma_level;
    uint32_t    delay;      // transition time delay
};

class ping_t : public led_t 
{
    public:
        ping_t(uint8_t pin, uint8_t count=MAX_LEDS, uint8_t set_width=1, uint8_t red=0xff, uint8_t green=0xff, uint8_t blue=0xff) : led_t(pin, count, red, green, blue, 0xff)
        {
            led_pin = pin;
            width = set_width;
            width_dir = 1;
            position = 0;
            last_pos = 0;
            state = PING_START;    
            next_state = PING_UP;
            delay = 1;
            next_time = ticks + delay;
        }
       

        uint8_t     position;   // led offset start
        uint8_t     last_pos;
        uint8_t     width;      // number of leds on
        uint8_t     width_dir; 
        uint8_t     state;      // current state;
        uint8_t     next_state; // next state;
        uint32_t    next_time;  // next transition time
        void run();

        enum {
            PING_START=0,
            PING_UP,
            PING_DOWN
        };
};      

class color_t : public led_t
{
    public:
        color_t(uint8_t pin, uint8_t count, uint8_t r, uint8_t g, uint8_t b, uint8_t gamma) : 
            led_t(pin, count, r, g, b, gamma) {

            state = COLOR_START;    
            delay = 1;
        }
        void run(void);
        void set_fade(uint32_t time, uint8_t gamma, uint8_t next);
        void correct_gamma(uint8_t gamma_adjust);

        uint8_t     state;      // current state;
        uint8_t     next_state;      // current state;
        uint8_t     after_fade;      // current state;
        uint32_t    next_time;  // next transition time
        uint32_t    fade_time;
        uint8_t    fade_amount;
        
        enum {
            COLOR_START=0,
            COLOR_FADE
        };
};


void color_t::set_fade(uint32_t time, uint8_t gamma, uint8_t next)
{
    fade_time = time;
    next_state = COLOR_FADE;
    fade_amount = gamma;
    after_fade = next;
    next_time = ticks + fade_time;
}

void color_t::run(void)
{
    uint8_t i;
    switch(state) {
        case COLOR_START:
            for (i=0; i< total; i++) {
               led_workspace[i].r = red_level;
               led_workspace[i].g = green_level;
               led_workspace[i].b = blue_level;
            }
            ws2812_sendarray_mask((uint8_t *)led_workspace, total, led_pin);
            state = next_state;
            break;
        case COLOR_FADE:
            if (ticks > next_time) {
                if (gamma_level > 0) {
                    gamma_level--;
                    update_gamma();
                    next_time = ticks + fade_time;
                } else {
                    next_state = after_fade;
                }
                state = COLOR_START;
            }
            break;
    }

}

void ping_t::run(void)
{
    uint8_t i;
    switch(state) {
        case PING_START:
            for (i=0; i< total; i++) {

                if (i< position) {
                    led_workspace[i].r = led_workspace[i].g  = led_workspace[i].b = 0;
                } else if (i < position + width) {
                   led_workspace[i].r = red_level;
                   led_workspace[i].g = green_level;
                   led_workspace[i].b = blue_level;
                } else {
                    led_workspace[i].r = led_workspace[i].g  = led_workspace[i].b = 0;
                }
            }
            ws2812_sendarray_mask((uint8_t *)led_workspace, total, led_pin);
            state = next_state;
            break;
        case PING_UP:
            if (ticks > next_time) {
                if (position < total-1) {
                    last_pos = position;
                    position++;
                } else {
                    next_state = PING_DOWN;
                    width += width_dir;
                    if (width >= total) {
                        width_dir = -1;
                    } else if (width <= 1) {
                        width_dir = 1;
                    }     
                    
                }
                next_time = ticks + delay;
            }
            state = PING_START;
            break;
        case PING_DOWN:
            if (ticks > next_time) {
                if (position > 0) {
                    last_pos = position;
                    position--;
                } else {
                    next_state = PING_UP;
                }
                next_time = ticks + delay;
            }
            state = PING_START;
            break;
    }
}

int main(void)
{
	DDRD = 3;
    init_timer();

/*
    ping_t ping(1,120,1,0,0xff,0);
    ping_t ping2(2,120,5,0xff,0,0);
    ping.blue_level = 0xff;
    ping2.red_level = 0xff;
    ping2.width = 5;
*/
    color_t color1(1,10,0,0xff,0,0xff);
    color_t color2(2,10,0xff,0,0,0x7f);
    
    color1.set_fade(10,1,color_t::COLOR_FADE);
    color2.set_fade(10,1,color_t::COLOR_FADE);
    while(1) {
//        ping.run();
//        ping2.run();
        color1.run();
        color2.run();
    }

}
