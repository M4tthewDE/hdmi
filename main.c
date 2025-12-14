#include <avr/io.h>
#include <util/delay.h>

#define F_CPU 16000000UL

int main(void) {
    DDRB |= (1 << PB5);

    while (1) {
        PORTB ^= (1 << PB5);  // Toggle LED
        _delay_ms(500);
    }

    return 0;
}
