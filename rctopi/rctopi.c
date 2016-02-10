// rctopi.c

#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CAMERA_OFF          0b00000000 // sw1 down
#define PICTURE_MODE        0b00000001 // sw1 mid,  sw2 mid
#define VIDEO_MODE          0b00000010 // sw1 high, sw2 mid

#define PIC_BURST_ACTION    0b00000011 // sw1 mid,  sw2 down (picture mode, sw2 down)
#define PIC_TOGGLE_ACTION   0b00000100 // sw1 mid,  sw2 high
#define VID_TOGGLE_ACTION   0b00000101 // sw1 high, sw2 high 
#define SHUTTER_ACTION      0b00000110 // sw1 high, sw2 low (video mode snapshot)

#define AUX1_ACTION         0b00000111

#define LOW_PWM             75
#define MID_PWM             100
#define HIGH_PWM            150

#define PWM_AVG_CNT         10

#define _SLEEP \
    set_sleep_mode(SLEEP_MODE_IDLE); \
    sleep_mode

volatile uint8_t previouspins = 0xFF;
volatile uint8_t changedpins = 0;

volatile int16_t pwm_p0  = 0;
volatile int16_t tpwm_p0 = 0;
volatile int16_t pwm_p1  = 0;
volatile int16_t tpwm_p1 = 0;

void setup(void);
void set_pin_values(void);

////
// Entry
int main(void)
{
    // ...keep Bryan from being fussy
    setup();

    // loop forever, flash pin1 every second to indicate we're
    while (1)
    {
        // Zzzzz...
        _SLEEP();

        sei();

        // Main program logic for notifying Raspy about switch positions
        set_pin_values();
    }

    return 0;
}


////
// Initialize ports, interrupts, timers, etc.
//
void setup(void)
{
    // disable interrupt
    cli();

    // enables input on pin3 and pin4, output on pin0, pin1, and pin2
    DDRB = 0b00000111;

    // set input pins with pull-up enabled
    PORTB |= ((1 << PORTB3) | (1 << PORTB4));

    // set PCIE to enable PCMSK scan
    GIMSK |= (1 << PCIE);

    // this enables the pin change interrupt on state change on PCINT0
    // of which there is only one interrupt on the attiny85
    PCMSK |= ((1 << PCINT3) | (1 << PCINT4));

    TCCR1 = 0;
    TCNT1 = 0;
    OCR1A = 1;
    OCR1C = 1;
    GTCCR = _BV(PSR1);

    // setup compare/match interrupt for timer
    TIMSK = (1 << OCIE1A);

    // setup timer TCCR1 at 1Mhz
    TCCR1 = (_BV(CTC1) | _BV(CS12) | _BV(CS11) | _BV(CS10));

    // enable interrupt
    sei();
}


////
// Set pin values based on PWM values
//
void set_pin_values()
{
    // Camera off
    if (pwm_p0 <= LOW_PWM) {
        // everything is off if p0 is low
        PORTB = CAMERA_OFF;
    }
    // Picture Mode
    else if (pwm_p0 <= MID_PWM) {
        if (pwm_p1 <= LOW_PWM) {
            PORTB = PIC_BURST_ACTION;
        }
        else if (pwm_p1 <= MID_PWM) {
            PORTB = PICTURE_MODE;
        }
        else if (pwm_p1 > MID_PWM) {
            PORTB = PIC_TOGGLE_ACTION;
        }
    }
    // Video Mode
    else if (pwm_p0 > MID_PWM) {
        if (pwm_p1 <= LOW_PWM) {
            PORTB = SHUTTER_ACTION;
        }
        else if (pwm_p1 <= MID_PWM) {
            PORTB = VIDEO_MODE;
        }
        else if (pwm_p1 > MID_PWM) {
            PORTB = VID_TOGGLE_ACTION;
        }
    }
}


////
// Timer compare/match interrupt
//
ISR(TIM1_COMPA_vect)
{
    tpwm_p0++;
    tpwm_p1++;
}


////
// Pin change interrupt for PB0 and PB1
//
ISR(PCINT0_vect)
{
    changedpins = PINB ^ previouspins;
    previouspins = PINB;

    if (changedpins & (1 << PB3)) {
        // rising edge
        if (PINB & (1 << PB3)) {
            // clear pulse timer
            tpwm_p0 = 0;
        }
        // falling edge
        else {
            pwm_p0 = tpwm_p0;
        }
    }

    if (changedpins & (1 << PB4)) {
        // rising edge
        if (PINB & (1 << PB4)) {
            // clear pulse timer
            tpwm_p1 = 0;
        }
        // falling edge
        else {
            pwm_p1 = tpwm_p1;
        }
    }
}

// EOF
