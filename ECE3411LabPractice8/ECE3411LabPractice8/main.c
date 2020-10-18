/*
 * ECE3411LabPractice8.c
 *
 * Created: 01-Mar-20 23:44:48
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_ON PORTD |= (1<<PORTD0)
#define LED_OFF PORTD &= ~(1<<PORTD0)

volatile int dutyCycle = 0;
volatile int counter = 0;
void setupADC(void)
{
	ADMUX |= (1<<MUX2) | (1<<MUX1); // ADC6 
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | // ADC enable, interrupt enable
				(1<<ADPS1); // prescaler = 64
				
	startConversion();
}

void startConversion(void)
{
	ADCSRA |= (1<<ADSC);
}


void InitTimer1(void)
{
	//WGM = Fast PWM w/ TOP at OCR1A = 1111
	//COM1B = 10, Clear OC1B on compare match, set on BOTTOM
		
	//50 Hz clock
	TCCR1A |= (1<<WGM10) | (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	OCR1A = 1249;
	TIMSK1 |= (1<<OCIE1B) | (1<<OCIE1A); //Enable output compare B match interrupt
	TCCR1B |= (1<<CS12); // prescaler = 256
}

ISR(ADC_vect)
{
	dutyCycle = ADC;
	//startConversion();
}

ISR(TIMER1_COMPB_vect)
{
	LED_OFF;
	if(dutyCycle>0)
		OCR1B = (int)((dutyCycle/1024.0)*(OCR1A+1)) - 1; 
	else 
		OCR1B = 0;
}

ISR(TIMER1_COMPA_vect)
{
	LED_ON;
	if(counter == 4)
	{
		startConversion();
		counter = 0;
	}
	else
		counter++;
}
int main(void)
{
    /* Replace with your application code */
    DDRD |= (1<<DDD0); // PORTD0 Output
	
	//Enable global interrupts
	InitTimer1();
	setupADC();
	sei();
	while (1) 
    {
    }
}


