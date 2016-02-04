/* ledblink.c, an LED blinking program */

#ifndef F_CPU
#define F_CPU 8000000UL	// 16MHz clock speed
#endif

#include <avr/io.h>
#include <util/delay.h>

main()
{
	DDRC = 0xFF;
	while (1)
	{
		PORTC = 0xFF;
		_delay_ms(1000);

		PORTC = 0x00;
		_delay_ms(1000);
	}
}

