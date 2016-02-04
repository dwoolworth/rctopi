#ifndef F_CPU
#define F_CPU 8000000UL	// 16MHz clock speed
#endif

#define FLIP_SWITCH 1020

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

volatile uint16_t temprcpulse = 0;
volatile uint16_t rcpulse = 0;

void wait_sec(int secs);

void reset(void)
{

    DDRB = 0b00000111;

    TCCR1 |= (1 << CS12); //timer1 8MHz/8
    TIMSK |= (1 << TOIE1);  //enable overflow interrupt for timer 1

    GIMSK |= (1 << PCIE); //enable pin change interrupt

    PCMSK |= (1 << PCINT0); // enable PCINT0 pin change interrupt

    sei();
}

void wait_sec(int secs)
{
    int i;
    for (i = 0; i < secs; i++)
        _delay_ms(1000);
} 

int main (void)
{
    reset();

    for(;;)  // Loop Forever
    {
        if (rcpulse > FLIP_SWITCH) {
            PORTB |= (1 << PCINT2);
        }
        else {
            PORTB &= ~(1 << PCINT2);
        }

        PORTB |= (1 << PCINT1);
        wait_sec(1);

        PORTB &= ~(1 << PCINT1);
        wait_sec(1);
    }
} 


ISR(TIM1_OVF_vect)  //overflow interrupt timer 1
{
    temprcpulse += 256;
}


ISR(PCINT0_vect)    //Pin change interrupt
{
    uint8_t     temprcpulse1 = TCNT1;   //save timer 1
    // uint16_t    check = temprcpulse;    //save to check value before any calculations in interrupt

    if (TIFR & (1 << TOV1)) //check for a timer overflow
    {
        TIFR &= ~(1 << TOV1); //clear overflow flag
    }

    if(PINB & (1 << PINB3)) /// Rising edge
    {   
        temprcpulse = 0; ///clear
        TCNT1 = 0;          //restart timer 1
        TIMSK |= (1 << TOIE1); //enable overflow on timer 1
    }
    else    //falling edge
    {   
        
        temprcpulse = temprcpulse + temprcpulse1;   //add timer 1 to overflow value
        rcpulse = temprcpulse;
                
        TIMSK &= ~(1 << TOIE1); //clear overflow flag
        TIFR &= ~(1 << TOV1);   //disable overflow interrupt

    }

}

