/*
 * ECE3411LabPractice7.c
 *
 * Created: 26-Feb-20 14:41:51
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <math.h>
#include <avr/interrupt.h>
volatile double dutyCycle = 0;
volatile int counter = 0;
volatile double sin_arg = 0;
void InitTimer0(void)
{
	//50 KHz clock
	TCCR0A |= (1<<WGM01); // CTC OCR0A
	OCR0A = 39;
	TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
	TCCR0B |= (1<< CS01); //prescaler = 8 start timer
}

void InitTimer1(void)
{
	//50 KHz clock
	TCCR1A |= (1<<COM1B1) | (1<<WGM10) | (1<<WGM11); // OC1B clear non-inverting
	TCCR1B |= (1<<WGM12) | (1<<WGM13); // Mode 15, Fast PWM OCR1A =Top
	OCR1A = 39;
	OCR1B = 0;
	TIMSK1 = (1<<OCIE1B); //Enable Timer 1 Compare B ISR
	TCCR1B |= (1<<CS11); //prescaler = 8 and timer starts
}

ISR(TIMER0_COMPA_vect)
{
	sin_arg = 2*M_PI*(counter)/1000.0;
	counter = (counter+1)%1000;
}

ISR(TIMER1_COMPB_vect)
{
	dutyCycle = abs(sin(sin_arg));
	if(dutyCycle>0)
		OCR1B = (int)(dutyCycle*(OCR1A+1)) - 1;
	else
		OCR1B = 0;		
}
int main(void)
{
	DDRB |= (1<<DDB2); //B2 output
	InitTimer1();
	InitTimer0();
	sei();
    while (1) 
    {
    }
}


