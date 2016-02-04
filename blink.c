// blink.c
#ifndef F_CPU
#define F_CPU 8000000UL	// 16MHz clock speed
#endif

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint8_t pinsprev = 0x00;

uint16_t rcpulse0 = 0;
uint16_t rcpulse_start0 = 0;
uint16_t rcpulse1 = 0;
uint16_t rcpulse_start1 = 0;

void wait_sec(int seconds);

int main(void)
{
    // enables input on pin0 and pin1, output on pin2, pin3, and pin4
    DDRB = 0b00011100;

    // set input pins to pull-up
    PORTB = 0b00000011; // pull-up pins 0 and 1

    // set PCIE to enable PCMSK scan (unsure if it's this or PCICR)
    GIMSK |= (1 << PCIE);

    // set PCIE to enable PCMSK scan
    // PCICR0 |= (1 << PCIE);

    // set PCINT0 and PCINT1 to trigger an interrupt on state change
    PCMSK |= ((1 << PCINT0) | (1 << PCINT1));

    // setup timer TCCR1 at 1Mhz
    TCCR1 |= (1 << CS12); // 8Mhz/8

    // setup overflow interrupt for timer
    TIMSK = (1 << TOIE1);

    sei();

    // loop forever (something to do while waiting for interrupts)
    while (1)
    {
        PORTB |= 0b00000100;
        wait_sec(2);

        PORTB &= 0b11111011;
        wait_sec(2);
    }

    return 0;
}

void wait_sec(int seconds)
{
    int i;
    for (i = 0; i < seconds; i++)
        _delay_ms(1000);
}

ISR(TIM1_OVF_vect)
{
    rcpulse0 += 256;
    rcpulse1 += 256;
}

ISR(PCINT0_vect)
{
    uint8_t pinschanged;
    uint8_t current_time = TCNT1;

    pinschanged = PINB ^ pinsprev;
    pinsprev = PINB;

    if (pinschanged & (1 << PB0)) {
        // pin 0 has changed

        PORTB |= 0b00001000;

        if ((PINB & (1 << PB0)) == 1) {
			// LOW to HIGH pin change

            // clear pulse timer
            rcpulse0 = 0;
            rcpulse_start0 = current_time;
        }
        else {
            // HIGH to LOW pin change
            
            // calculate pulse width
            rcpulse0 += current_time - rcpulse_start0;
        }
    }

    if (pinschanged & (1 << PB1)) {
        // pin 1 has changed

        PORTB &= 0b11110111;

        if ((PINB & (1 << PB1)) == 1) {
            // LOW to HIHGH pin change

            // clear pulse timer
            rcpulse1 = 0;
            rcpulse_start1 = current_time;
        }
        else {
            // HIGH to LOW pin change

            // calculate pulse width
            rcpulse1 += current_time - rcpulse_start1;
        }
    }

    // turn on LED pin 5 if width is over 1000
    if (rcpulse0 > 1000) {
        PORTB |= 0b00001000;
    }
    else {
        PORTB &= 0b11110111;
    }

    // turn on LED pin 4 if width is over 1000
    if (rcpulse1 > 1000) {
        PORTB |= 0b00010000;
    }
    else {
        PORTB &= 0b11101111;
    }
}


