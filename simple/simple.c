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
// #define RED_LED     0b00010000
#define LED_NONE    0b00000000

#define PWM_LOW 78
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
    OCR1A = 1;
    OCR1C = 1;
    TIMSK = (1 << OCIE1A);
    TCCR1 = (_BV(CTC1) | _BV(CS12) | _BV(CS11) | _BV(CS10));

    // Initialize before we enable interrupts
    dbg_tx_init();

    sei();

    // test the LEDs
    // system_test();

    // loop forever, flash pin1 every second to indicate we're
    while (1)
    {
        sei();
        _IDLE_SLEEP();

        // display_pulse(rcpulse_0);

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
                enable_led(LED_NONE);
            }
            else if (pwm_to_display > PWM_HGH) {
                enable_led(BLUE_LED | YELLOW_LED);
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
    if (led > 0)
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
    // enable_led(RED_LED);    wait_sec(1);
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
    static int counter = 0;
    char buffer[100];

    changedbits = PINB ^ previouspins;
    previouspins = PINB;

    // only should occur if PB0 changes
    if (changedbits & (1 << PB0)) {
        if ((PINB & (1 << PB0)) == 1) {
            tmprcpulse_0 = 0;
        }
        else {
            rcpulse_0 = tmprcpulse_0;
            dbg_putstr("pb0: ");
            dbg_putstr(utoa(rcpulse_0, buffer, 10));
            dbg_putstr("| ");
            if (counter++ > 80) {
                dbg_putstr("\r\n");
                counter = 0;
            }
        }
    }

    // only should occur if PB1 changes
    if (changedbits & (1 << PB2)) {
        if ((PINB & (1 << PB2)) == 1) {
            tmprcpulse_1 = 0;
        }
        else {
            rcpulse_1 = tmprcpulse_1;
            dbg_putstr("pb0: ");
            dbg_putstr(utoa(rcpulse_0, buffer, 10));
            dbg_putstr("| ");
            if (counter++ > 80) {
                dbg_putstr("\r\n");
                counter = 0;
            }
        }
    }
}


#if DBG_UART_ENABLE

void dbg_putchar(uint8_t c)
{
#define DBG_UART_TX_NUM_DELAY_CYCLES    ((F_CPU/DBG_UART_BAUDRATE-16)/4+1)
#define DBG_UART_TX_NUM_ADD_NOP     ((F_CPU/DBG_UART_BAUDRATE-16)%4)
    uint8_t sreg;
    uint16_t tmp;
    uint8_t numiter = 10;

    sreg = SREG;
    cli();

    asm volatile (
        /* put the START bit */
        "in %A0, %3"        "\n\t"  /* 1 */
        "cbr %A0, %4"       "\n\t"  /* 1 */
        "out %3, %A0"       "\n\t"  /* 1 */
        /* compensate for the delay induced by the loop for the
         * other bits */
        "nop"           "\n\t"  /* 1 */
        "nop"           "\n\t"  /* 1 */
        "nop"           "\n\t"  /* 1 */
        "nop"           "\n\t"  /* 1 */
        "nop"           "\n\t"  /* 1 */

        /* delay */
       "1:" "ldi %A0, lo8(%5)"  "\n\t"  /* 1 */
        "ldi %B0, hi8(%5)"  "\n\t"  /* 1 */
       "2:" "sbiw %A0, 1"       "\n\t"  /* 2 */
        "brne 2b"       "\n\t"  /* 1 if EQ, 2 if NEQ */
#if DBG_UART_TX_NUM_ADD_NOP > 0
        "nop"           "\n\t"  /* 1 */
  #if DBG_UART_TX_NUM_ADD_NOP > 1
        "nop"           "\n\t"  /* 1 */
    #if DBG_UART_TX_NUM_ADD_NOP > 2
        "nop"           "\n\t"  /* 1 */
    #endif
  #endif
#endif
        /* put data or stop bit */
        "in %A0, %3"        "\n\t"  /* 1 */
        "sbrc %1, 0"        "\n\t"  /* 1 if false,2 otherwise */
        "sbr %A0, %4"       "\n\t"  /* 1 */
        "sbrs %1, 0"        "\n\t"  /* 1 if false,2 otherwise */
        "cbr %A0, %4"       "\n\t"  /* 1 */
        "out %3, %A0"       "\n\t"  /* 1 */

        /* shift data, putting a stop bit at the empty location */
        "sec"           "\n\t"  /* 1 */
        "ror %1"        "\n\t"  /* 1 */

        /* loop 10 times */
        "dec %2"        "\n\t"  /* 1 */
        "brne 1b"       "\n\t"  /* 1 if EQ, 2 if NEQ */
        : "=&w" (tmp),          /* scratch register */
          "=r" (c),         /* we modify the data byte */
          "=r" (numiter)        /* we modify number of iter.*/
        : "I" (_SFR_IO_ADDR(DBG_UART_TX_PORT)),
          "M" (1<<DBG_UART_TX_PIN),
          "i" (DBG_UART_TX_NUM_DELAY_CYCLES),
          "1" (c),          /* data */
          "2" (numiter)
    );
    SREG = sreg;
}
#undef DBG_UART_TX_NUM_DELAY_CYCLES
#undef DBG_UART_TX_NUM_ADD_NOP

#endif

// EOF
