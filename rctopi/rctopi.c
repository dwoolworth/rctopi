// rctopi.c

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define BLUE_LED   0b00000010
#define RED_LED    0b00000100
#define YELLOW_LED 0b00001000

volatile int16_t rcpulse = 0;
volatile int16_t tmprcpulse = 0;
volatile int16_t rcpulse_start = 0;

volatile uint16_t secs_to_flash = 1;

void wait_sec(int seconds);

int main(void)
{
    uint8_t led = BLUE_LED;

    // enables input on pin0,  output on pin1, pin2, pin3, and pin4
    DDRB = 0b00011110;

    // set input pins to pull-up
    PORTB = 0b00000001; // pull-up pins 0 and 1

    // set PCIE to enable PCMSK scan
    GIMSK |= (1 << PCIE);

    // set PCINT0 to trigger an interrupt on state change
    PCMSK |= (1 << PCINT0);

    // setup timer TCCR1 at 1Mhz
    TCCR1 |= (1 << CS12);

    // setup overflow interrupt for timer
    TIMSK = (1 << TOIE1);

    sei();

    // loop forever, flash pin1 every second to indicate we're
    while (1)
    {

        if (rcpulse <= 1200) {
            led = BLUE_LED;
        }
        else if (rcpulse <= 1800) {
            led = RED_LED;
        }
        else if (rcpulse <= 2200) {
            led = YELLOW_LED;
        }
        else if (rcpulse > 2200) {
            led = RED_LED | YELLOW_LED | BLUE_LED;
        }

        PORTB |= (led);
        wait_sec(secs_to_flash);

        PORTB &= ~(led);
        wait_sec(secs_to_flash);
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
    tmprcpulse += 256;
}

ISR(PCINT0_vect)
{
    uint8_t current_time = TCNT1;

    if ((PINB & (1 << PB0)) == 1) {
        // LOW to HIGH pin change

        // clear pulse timer
        tmprcpulse = 0;
        rcpulse_start = current_time;
    }
    else {
        // HIGH to LOW pin change
        rcpulse = tmprcpulse + (current_time - rcpulse_start);
    }
}


