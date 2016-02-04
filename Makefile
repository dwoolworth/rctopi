
test1:
	avr-gcc -I. -I. -g -mmcu=attiny85 -DF_CPU=8000000UL -Os -fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char -Wall -Wstrict-prototypes -Wa,-ahlms=test1.lst -c test1.c -o test1.o
	avr-gcc -Wl,-Map,myproject.out.map -mmcu=attiny85 -lm  -o myproject.out test1.o 
	avr-objcopy -j .text -j .data -O ihex myproject.out myproject.hex
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex myproject.out myproject.ee.hex

simple:
	avr-gcc -I. -I. -g -mmcu=attiny85 -DF_CPU=8000000UL -Os -fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char -Wall -Wstrict-prototypes -Wa,-ahlms=simple.lst -c simple.c -o simple.o
	avr-gcc -Wl,-Map,myproject.out.map -mmcu=attiny85 -lm  -o myproject.out simple.o 
	avr-objcopy -j .text -j .data -O ihex myproject.out myproject.hex
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex myproject.out myproject.ee.hex

default:
	avr-gcc -I. -I. -g -mmcu=attiny85 -DF_CPU=8000000UL -Os -fpack-struct -fshort-enums -funsigned-bitfields -funsigned-char -Wall -Wstrict-prototypes -Wa,-ahlms=blink.lst -c blink.c -o blink.o
	avr-gcc -Wl,-Map,myproject.out.map -mmcu=attiny85 -lm  -o myproject.out blink.o 
	avr-objcopy -j .text -j .data -O ihex myproject.out myproject.hex
	avr-objcopy -j .eeprom --change-section-lma .eeprom=0 -O ihex myproject.out myproject.ee.hex

install:
	sudo avrdude -c usbtiny -p attiny85 -U flash:w:myproject.hex

install-clean:
	sudo avrdude -c usbtiny -p attiny85 -D -U flash:w:myproject.hex

clean:
	rm -f *.hex *.o *.lst *.map *.out

