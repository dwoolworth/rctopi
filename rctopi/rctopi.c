// rctopi.c

#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define CAMERA_OFF          0b00000000 // sw1 down
#define PICTURE_MODE        0b00000100 // sw1 mid, sw2 mid
#define VIDEO_MODE          0b00001000 // sw1 high, sw2 mid

#define PIC_BURST_ACTION    0b00001100 // sw1 mid, sw2 down (picture mode, sw2 down)
#define PIC_TOGGLE_ACTION   0b00010000 // sw1 mid,  sw2 high
#define VID_TOGGLE_ACTION   0b00010100 // sw1 high, sw2 high 
#define SHUTTER_ACTION      0b00011000 // sw1 high, sw2 low (video mode snapshot)

#define AUX1_ACTION         0b00011100

#define LOW_PWM  1200
#define MID_PWM  1800
#define HIGH_PWM 2200

#define _SLEEP set_sleep_mode(SLEEP_MODE_IDLE); sleep_mode

volatile uint8_t previouspins = 0xFF;

volatile int16_t pwm_p0  = 0;
volatile int16_t tpwm_p0 = 0;
volatile int16_t pwm_p1  = 0;
volatile int16_t tpwm_p1 = 0;


int main(void)
{
    // enables input on pin0 and pin1, output on pin2, pin3, and pin4
    DDRB = 0b00011100;

    // set input pins with pull-up enabled
    PORTB |= ((1 << PORTB0) | (1 << PORTB1));

    // set PCIE to enable PCMSK scan
    GIMSK |= (1 << PCIE);

    // this enables the pin change interrupt on state change on PCINT0
    // of which there is only one interrupt on the attiny85
    PCMSK |= (1 << PCINT0);

    // setup timer TCCR1 at 1Mhz
    TCCR1 |= (1 << CS12);

    // setup overflow interrupt for timer
    TIMSK = (1 << TOIE1);

    sei();

    // loop forever, flash pin1 every second to indicate we're
    while (1)
    {
        _SLEEP();

        if (pwm_p0 <= LOW_PWM) {
            // everything is off if p0 is low
            PORTB = CAMERA_OFF;
        }
        else {
            if (pwm_p0 <= MID_PWM) {
                if (pwm_p1 <= LOW_PWM) {
                    PORTB = PIC_BURST_ACTION;
                }
                else if (pwm_p1 <= MID_PWM) {
                    PORTB = PICTURE_MODE;
                }
                else if (pwm_p1 <= HIGH_PWM) {
                    PORTB = PIC_TOGGLE_ACTION;
                }
                else {
                    // for now, this is an error condition
                    PORTB = AUX1_ACTION;
                }
            }
            else if (pwm_p0 <= HIGH_PWM) {
                if (pwm_p1 <= LOW_PWM) {
                    PORTB = SHUTTER_ACTION;
                }
                else if (pwm_p1 <= MID_PWM) {
                    PORTB = VIDEO_MODE;
                }
                else if (pwm_p1 <= HIGH_PWM) {
                    PORTB = VID_TOGGLE_ACTION;
                }
                else {
                    // for now, this is an error condition
                    PORTB = AUX1_ACTION;
                }
            }
        }
    }

    return 0;
}

ISR(TIM1_OVF_vect)
{
    tpwm_p0 += 256;
    tpwm_p1 += 256;
}

ISR(PCINT0_vect)
{
    volatile static int16_t pwm_start_p0 = 0;
    volatile static int16_t pwm_start_p1 = 0;

    uint8_t changedbits;
    uint8_t current_time = TCNT1;

    changedbits = PINB ^ previouspins;
    previouspins = PINB;

    if (changedbits & (1 << PB0)) {
        if ((PINB & (1 << PB0)) == 1) { // rising
            // clear pulse timer
            tpwm_p0 = 0;
            pwm_start_p0 = current_time;
        }
        else { // falling
            pwm_p0 = tpwm_p0 + (current_time - pwm_start_p0);
        }
    }

    if (changedbits & (1 << PB1)) {
        if ((PINB & (1 << PB1)) == 1) { // rising
            // clear pulse timer
            tpwm_p1 = 0;
            pwm_start_p1 = current_time;
        }
        else { // falling
            pwm_p1 = tpwm_p1 + (current_time - pwm_start_p1);
        }
    }
}


