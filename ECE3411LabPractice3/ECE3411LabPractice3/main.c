/*
 * ECE3411LabPractice3.c
 *
 * Created: 03-Feb-20 14:31:14
 * Author : Siddarth Suresh
 */ 

# define F_CPU 16000000UL
#include <avr/io.h>
# include <util/delay.h>
#include <avr/interrupt.h>
# define SW_PRESSED !(PIND & (1<<PIND3))

ISR(INT1_vect)
{
	_delay_ms(10);
	PORTD &= ~(1<<PORTD0);
	_delay_ms(5000);
}

int main(void)
{
    /* Replace with your application code */
	DDRD |= (1<<DDD0);	// Port D0 output
	DDRD &= ~(1<<DDD3); // Port D3 input
	EICRA |= (1<<ISC11); // falling edge of INT1 (PORT D7) generates interrupt
	EIMSK |= (1<<INT1);
	
	sei();
    while (1) 
    {
		PORTD |= (1<<PORTD0);
		_delay_ms(83);
		PORTD &= ~(1<<PORTD0);
		_delay_ms(83);
    }
}

