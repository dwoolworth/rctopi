// blink.c
#ifndef F_CPU
#define F_CPU 8000000UL	// 16MHz clock speed
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/sleep.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BLUE_LED    0b00000100
#define YELLOW_LED  0b00001000
#define RED_LED     0b00010000

#define PWM_LOW 32000
#define PWM_MID 100
#define PWM_HGH 150
// #define PWM_LOW 78 
// #define PWM_MID 100
// #define PWM_HGH 150

#define _SLEEP \
    set_sleep_mode(SLEEP_MODE_IDLE); \
    sleep_mode


volatile uint8_t  ready = 0;
volatile uint16_t rcpulse = 0;
volatile uint16_t tmprcpulse = 0;

void wait_sec(int seconds);
void enable_led(uint8_t led);

int main(void)
{
    cli();

    // enables input on pin0 and pin1, output on pin2, pin3, and pin4
    DDRB = 0b00011100;

    // set input pins to pull-up
    PORTB |= ((1 << PORTB0) | (1 << PORTB1));

    // set PCIE to enable PCMSK scan
    GIMSK |= (1 << PCIE);

    // set PCINT0 to trigger an interrupt on state change
    PCMSK |= ((1 << PCINT0) | (1 << PCINT1));

    // setup timer TCCR1 at 1Mhz
    TCCR1 = 0;
    TCNT1 = 0;
    GTCCR = _BV(PSR1);
    OCR1A = 1;
    OCR1C = 1;
    TIMSK = (1 << OCIE1A);
    TCCR1 = (_BV(CTC1) | _BV(CS12) | _BV(CS11) | _BV(CS10));
    sei();

    enable_led(BLUE_LED);
    wait_sec(1);

    enable_led(YELLOW_LED);
    wait_sec(1);

    enable_led(RED_LED);
    wait_sec(1);

    // loop forever, flash pin1 every second to indicate we're
    while (1)
    {
        sei();
        _SLEEP();

        if (ready) {
            if (rcpulse <= PWM_LOW) {
                enable_led(BLUE_LED);
            }
            else {
                if (rcpulse <= PWM_MID) {
                    enable_led(YELLOW_LED);
                }
                else {
                    if (rcpulse <= PWM_HGH) {
                        enable_led(RED_LED);
                    }
                    else if (rcpulse > PWM_HGH) {
                        enable_led(BLUE_LED | YELLOW_LED | RED_LED);
                    }
                }
            }
        }
    }

    return 0;
}

void enable_led(uint8_t led)
{
    // turn them all off
    PORTB &= 0b11100011;
    PORTB |= led;
}

void wait_sec(int seconds)
{
    int i;
    for (i = 0; i < seconds; i++)
        _delay_ms(500);
}

ISR(TIM1_COMPA_vect)
{
    tmprcpulse++;
}

ISR(PCINT0_vect)
{
    volatile uint8_t previouspins = 0xFF;
    uint8_t changedbits;

    changedbits = PINB ^ previouspins;
    previouspins = PINB;

    if (changedbits & (1 << PB1)) {
        if ((PINB & (1 << PB1)) == 1) {
            tmprcpulse = 0;
            ready = 0;
        }
        else {
            rcpulse = tmprcpulse;
            ready = 1;
        }
    }

}


