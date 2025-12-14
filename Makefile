.PHONE: run

run:
	avr-gcc -mmcu=atmega328p -DF_CPU=16000000UL -Os -o main.elf main.c
	avr-objcopy -O ihex main.elf main.hex
	avrdude -c arduino -p atmega328p -P /dev/ttyACM0 -b 115200 -U flash:w:main.hex
