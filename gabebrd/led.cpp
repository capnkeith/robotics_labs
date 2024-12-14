#include <util/delay.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
extern "C" {
#include "light_ws2812.h"

#define MAX_LEDS 120 
#define MAX_PINS 2
volatile uint32_t ticks = 1;
struct      cRGB global_led_workspaces[MAX_PINS][MAX_LEDS];
}

#define TIMER_PERIOD        0x1
#define ONE_SECOND_TICKS    50000

ISR(TCA0_OVF_vect) {
    ticks++;
    TCA0.SINGLE.INTFLAGS = TCA_SINGLE_OVF_bm;
}

void init_timer(void)
{
    TCA0.SINGLE.INTCTRL = TCA_SINGLE_OVF_bm;
    TCA0.SINGLE.PER = TIMER_PERIOD;
    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_1_bm | TCA_SINGLE_CLKSEL_2_bm);
//    TCA0.SINGLE.CTRLA = (TCA_SINGLE_CLKSEL_1_bm);
    TCA0.SINGLE.CTRLA |= TCA_SINGLE_ENABLE_bm;
}

#if 0
// This is an interrupt service routine. It will be called every few
// micro seconds interrupting your main() function in the middle of whatever 
// it was doing. This can be very hard to debug. For this reason keep code in
// the ISR to a minium. Any shared variables between main() and ISR should be
// volatile.
ISR (TIMER1_COMPA_vect)
{
    ticks++;                            // add 1 to ticks each time called.
}
#endif

class led_t
{
public:
    led_t(void) 
    {
    }

    led_t(uint8_t pin, uint8_t count, uint8_t red, uint8_t green, uint8_t blue, uint8_t gamma)
    {
        total = count;
        led_pin = pin;
        red_level = red;
        green_level = green;
        blue_level = blue;
        gamma_level = gamma;
        done = false;
        uint16_t gamma_correct = ((uint16_t)green_level * gamma)/0xff;
        green_level = (uint8_t)gamma_correct;
        gamma_correct = ((uint16_t)red_level * gamma)/0xff;
        red_level = (uint8_t)gamma_correct;
        gamma_correct = ((uint16_t)blue_level * gamma)/0xff;
        blue_level = (uint8_t)gamma_correct;
        led_workspace = &global_led_workspaces[pin][0]; 
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

    void set_done(bool toset) 
    {
        done = toset;
    }

    bool get_done(void) 
    {
        return done;
    }

    bool        done;
    struct      cRGB *led_workspace;
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
        ping_t(void)
        {
        }
        ping_t(uint8_t pin, uint8_t count=MAX_LEDS, uint8_t set_width=1, uint8_t red=0xff, uint8_t green=0xff, uint8_t blue=0xff) : led_t(pin, count, red, green, blue, 0xff)
        {
            led_pin = pin;
            width = set_width;
            width_dir = 1;
            position = 0;
            state = PING_START;    
            next_state = PING_UP;
            delay = 1;
            next_time = ticks + delay;
        }
       
        void set_delay(uint32_t ticks)
        {
            delay=ticks;
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
        color_t(void) 
        {
        } 
        color_t(uint8_t pin, uint8_t count, uint8_t r, uint8_t g, uint8_t b, uint8_t gamma) : 
            led_t(pin, count, r, g, b, gamma) {
            state = COLOR_START;    
            delay = ONE_SECOND_TICKS;
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
            ws2812_sendarray_mask((uint8_t *)led_workspace, total * sizeof(cRGB) , led_pin);
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
            ws2812_sendarray_mask((uint8_t *)led_workspace, total * sizeof(cRGB), led_pin);
            state = next_state;
            break;
        case PING_UP:
            if (ticks > next_time) {
                if (position < total-1) {
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
                state = PING_START;
            }
            break;
        case PING_DOWN:
            if (ticks > next_time) {
                if (position > 0) {
                    last_pos = position;
                    position--;
                } else {
                    next_state = PING_UP;
                    set_done(true);
                }
                next_time = ticks + delay;
                state = PING_START;
            }
            break;
    }
}

#if 0
void init_timer(void)
{
    TCB0.CCMP = 240;
    TCB0.INTCTRL = TCB_CAPTEI_bm;
    TCB0.CTRLA = TCB_ENABLE_bm;
//TCB_CLKSEL_CLKDIV_gv | TCB_ENABLE_bm;
}
#endif



void self_check(void)
{
    {
        color_t color0(1,120,0xff,0,0,0xff);
        color0.run(); 
    }
    uint32_t next_tick = ticks + ONE_SECOND_TICKS/10;
    while(ticks < next_tick){}
    {
        color_t color0(1,120,0,0xff,0,0xff);
        color0.run(); 
    }
    next_tick = ticks + ONE_SECOND_TICKS/10;
    while(ticks < next_tick){}
    {
        color_t color0(1,120,0,0,0xff,0xff);
        color0.run(); 
    }
    next_tick = ticks + ONE_SECOND_TICKS/10;
    while(ticks < next_tick){}
    {
        next_tick = ticks + ONE_SECOND_TICKS/10;
        color_t color0(1,120,0,0,0,0xff);
        color0.run(); 
    }
    {
        color_t color0(2,120,0xff,0,0,0xff);
        color0.run(); 
    }
    next_tick = ticks + ONE_SECOND_TICKS/10;
    while(ticks < next_tick){}
    {
        color_t color0(2,120,0,0xff,0,0xff);
        color0.run(); 
    }
    next_tick = ticks + ONE_SECOND_TICKS/10;
    while(ticks < next_tick){}
    {
        color_t color0(2,120,0,0,0xff,0xff);
        color0.run(); 
    }
    next_tick = ticks + ONE_SECOND_TICKS/10;
    while(ticks < next_tick){}
    {
        next_tick = ticks + ONE_SECOND_TICKS/10;
        color_t color0(2,120,0,0,0,0xff);
        color0.run(); 
    }
}

class display_t
{
    public:
        display_t() 
        {
            ping1 = ping_t(1,10,2,0,0xff,0);
            ping2 = ping_t(2,10,2,0,0,0xff);
            display_state = PING_DISPLAY;
            ping1.set_delay(ONE_SECOND_TICKS);
            ping2.set_delay(ONE_SECOND_TICKS/2);
        }
   
        ping_t ping1;
        ping_t ping2;
        color_t color1;
        color_t color2;
        uint8_t display_state;
        void run() {
            switch(display_state) {
                case PING_DISPLAY:
                    ping1.run();
                    ping2.run();
                    if (ping2.get_done()) {
                        display_state = COLOR_DISPLAY;
                        color1 = color_t(1,120,0xff,0,0,0x10);
                        color2 = color_t(2,120,0,0,0xff,0x10);
                    }
                    break;
                case COLOR_DISPLAY:
                    color1.run();
                    color2.run();
                    break;
            } 
        }          
        
        enum {
            PING_DISPLAY=0,
            COLOR_DISPLAY
        };
};

int main(void)
{
	PORTC.DIR = 7;
    PORTC.OUT = 0;

    _PROTECTED_WRITE (CLKCTRL.OSCHFCTRLA, CLKCTRL_FRQSEL_24M_gc);
    init_timer();
    sei();
    self_check();

    display_t disp;
    while(1) {
        disp.run();
    } 

#if 0
    ping_t ping1(1,120,1,0xff,0,0);
    ping_t ping2(2,120,1,0,0xff,0);
    while(1) {
        ping1.run();
        ping2.run();
    }
#endif

}
//    color_t color0(1,10,10,0,0,0xff);
//    color0.run(); 
//    PORTC.OUT |= 4;
//    init_timer();
//    ping_t ping(1,10,2,0,0xff,0);
//    ping_t ping2(2,120,5,0xff,0,0);
//    ping.blue_level = 0xff;
//    ping2.red_level = 0xff;
//    ping2.width = 5;
//      color_t color1(1,5,0xff,0,0,0x10);
//    color_t color2(2,10,0xff,0,0,0x7f);
    
//    color1.set_fade(10,1,color_t::COLOR_FADE);
//    color2.set_fade(10,1,color_t::COLOR_FADE);
//        ping.run();
//        ping2.run();
//        color1.run();
     
