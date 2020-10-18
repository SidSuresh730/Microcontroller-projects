/*
 * LabPractice6.c
 *
 * Created: 17-Feb-20 19:44:45
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define LED_ON PORTD |= (1<<PORTD0)
#define LED_OFF PORTD &= ~(1<<PORTD0)
#define TOGGLE_LED PIND |= (1<<PIND0)
volatile int dutyCycle = 0;
volatile unsigned char increase_duty = 0;
volatile unsigned char decrease_duty = 0;
volatile char count = 0;
void Timer1Init(void)
{
	//WGM = Fast PWM w/ TOP at OCR1A = 1111
	//COM1B = 10, Clear OC1B on compare match, set on BOTTOM
	//CS1 = 11, prescaler = 64
	
	TCCR1A |= (1<<WGM10) | (1<<WGM11) | (1<<COM1B1);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	OCR1A = 249;
	TIMSK1 |= (1<<OCIE1B) | (1<<OCIE1A); //Enable output compare B, A match interrupt
	TCCR1B |= (1<<CS11) | (1<<CS10);
}

int main(void)
{
	DDRB &= ~(1<<DDB7); // set PORTB 7 to input (SW1) 
	DDRB &= ~(1<<DDB1); // set PORTB 1 to input (SW2)
	DDRB |= (1<<DDB2); // set PORTB 2 to output (OC1B)
	DDRD |= (1<<DDD0); // set PORTD 0 to output.
	Timer1Init();
	sei();
    /* Replace with your application code */
    while (1) 
    {
		//dutyCycle = (dutyCycle+10)%100;
		if(!(PINB & (1<<PINB7)))
		{
			increase_duty = 1;
		}
		else if(!(PINB & (1<<PINB1)))
		{
			decrease_duty = 1;
		}
    }
}

ISR(TIMER1_COMPB_vect)
{
	LED_OFF;
	if(dutyCycle > 0)
	{
		if(dutyCycle>100)
			dutyCycle = 100;
		OCR1B = (dutyCycle/100.0)*(OCR1A+1) - 1;
	}
	else
	{
		OCR1B = 0;
		dutyCycle = 0;
	}
		
}

ISR(TIMER1_COMPA_vect)
{
	LED_ON;
	if(count==100)
	{
		if(increase_duty)
		{
			if(dutyCycle<100)
				dutyCycle++;
			increase_duty = 0;
		}
		else if(decrease_duty)
		{
			if(dutyCycle>0)
				dutyCycle--;
			decrease_duty = 0;
		}
		count = -1;
	}
	count++;
}
