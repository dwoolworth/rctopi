// blink.c
#ifndef F_CPU
#define F_CPU 8000000UL	// 16MHz clock speed
#endif

#include <stdint.h>
#include <stdlib.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "dbg_putchar.h"

#define BLUE_LED    0b00000010
#define YELLOW_LED  0b00001000
#define RED_LED     0b00010000

#define PWM_LOW 0
#define PWM_MID 100
#define PWM_HGH 150
// #define PWM_LOW 78 
// #define PWM_MID 100
// #define PWM_HGH 150

#define _IDLE_SLEEP \
    set_sleep_mode(SLEEP_MODE_IDLE); \
    sleep_mode

// while it is potentially ok to place changedbits and
// previouspins as static variables inside of the ISR()
// routine, I would rather not chance it.  Interrupts
// are supposed to be disabled when the ISR() routine
// runs, and re-enabled when it returns (hence the reti()
// function which re-enables interrupts within the ISR()
// routine before it returns).  Strange behavior has me
// concerned at the pc interrupts.
volatile uint8_t  changedbits   = 0;
volatile uint8_t  previouspins  = 0xFF;

// rcpulse_0 and tmprcpulse_0 pertain to signals received on pin0
volatile uint16_t rcpulse_0     = 0;
volatile uint16_t tmprcpulse_0  = 0;

// rcpulse_1 and tmprcpulse_1 pertain to signals received on pin1
volatile uint16_t rcpulse_1     = 0;
volatile uint16_t tmprcpulse_1  = 0;

void wait_sec(int seconds);
void enable_led(uint8_t led);
void display_pulse(uint16_t pwm_to_display);
void system_test(void);
void dbg_putstr(char *str);

////
// entry point...
//
int main(void)
{
    char buffer[100];

    cli();

    // enables input on pin0 and pin1, output on pin2, pin3, and pin4
    DDRB = 0b00011010;

    // set input pins to pull-up
    PORTB |= ((1 << PORTB0) | (1 << PORTB2));

    // set PCIE to enable PCMSK scan
    GIMSK |= (1 << PCIE);

    // set PCINT0 to trigger an interrupt on state change
    PCMSK |= ((1 << PCINT0) | (1 << PCINT2));

    // setup timer TCCR1 at 1Mhz
    TCCR1 = 0;
    TCNT1 = 0;
    GTCCR = _BV(PSR1);
    OCR1A = 2;
    OCR1C = 1;
    TIMSK = (1 << OCIE1A);
    TCCR1 = (_BV(CTC1) | _BV(CS12) | _BV(CS11) | _BV(CS10));
    sei();

    // test the LEDs
    system_test();

    dbg_tx_init();

    // loop forever, flash pin1 every second to indicate we're
    while (1)
    {
        sei();
        _IDLE_SLEEP();

        
        // display_pulse(rcpulse_1);
        dbg_putstr(utoa(rcpulse_1, buffer, 10));
    }

    return 0;
}

////
// Output string to serial pin
//
void dbg_putstr(char *str)
{
    char *a;
    for (a = str; *a != '\0'; a++)
        dbg_putchar(*a);
}


////
// take a copy of the rcpulse_n var and set the
// LEDs to the appropriate value
//
void display_pulse(uint16_t pwm_to_display)
{
    if (pwm_to_display > 20000)
        return;

    if (pwm_to_display <= PWM_LOW) {
        enable_led(BLUE_LED);
    }
    else {
        if (pwm_to_display <= PWM_MID) {
            enable_led(YELLOW_LED);
        }
        else {
            if (pwm_to_display <= PWM_HGH) {
                enable_led(RED_LED);
            }
            else if (pwm_to_display > PWM_HGH) {
                enable_led(BLUE_LED | YELLOW_LED | RED_LED);
            }
        }
    }
}


////
// reset all and turn on the specified LED
//
void enable_led(uint8_t led)
{
    // turn them all off
    PORTB &= 0b11100101;
    PORTB |= led;
}


////
// wait for .5 a second
//
void wait_sec(int seconds)
{
    int i;
    for (i = 0; i < seconds; i++)
        _delay_ms(500);
}


////
// Test all the LEDs
//
void system_test(void)
{
    // system test...
    enable_led(BLUE_LED);   wait_sec(1);
    enable_led(YELLOW_LED); wait_sec(1);
    enable_led(RED_LED);    wait_sec(1);
}


////
// should be fired everytime we have a compare/match for OCR1A = 1
// or somewhere around every 20us
//
ISR(TIM1_COMPA_vect)
{
    tmprcpulse_0++;
    tmprcpulse_1++;
}


////
// this is supposed to fire anytime a pin value has changed
//
ISR(PCINT0_vect)
{
    changedbits = PINB ^ previouspins;
    previouspins = PINB;

    // only should occur if PB0 changes
    if (changedbits & (1 << PB0)) {
        if ((PINB & (1 << PB0)) == 1) {
            tmprcpulse_0 = 0;
        }
        else {
            rcpulse_0 = tmprcpulse_0;
        }
    }

    // only should occur if PB1 changes
    if (changedbits & (1 << PB2)) {
        if ((PINB & (1 << PB2)) == 1) {
            tmprcpulse_1 = 0;
        }
        else {
            rcpulse_1 = tmprcpulse_1;
        }
    }
}

// EOF
