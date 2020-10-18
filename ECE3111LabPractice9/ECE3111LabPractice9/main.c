/*
 * ECE3111LabPractice9.c
 *
 * Created: 09-Mar-20 00:05:44
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

volatile double tempRead = 0;
volatile double brightness = 0;
volatile unsigned char tempRead_flag = 0;
volatile unsigned char brightness_flag = 0;
volatile unsigned char counter = 0;
volatile int tempDuty = 50;
volatile int brightDuty = 50;
volatile unsigned char secondRead = 0;
void setupADC(void)
{
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1); // ADC enable, interrupt enable
	 // prescaler = 64
	 ADMUX = 6;
}

void startConversion(void)
{
	ADCSRA |= (1<<ADSC);
}

void tempDutyConversion(void)
{
	//0.4; // V0degrees
	//0195; //Temperature coefficient
	tempDuty = (int)((tempRead*(5.0/1024.0) - 0.4)/(.0195)*100)%100;
}
void brightDutyConversion(void)
{
	
	brightDuty = ((int)(brightness*(5.0/1024.0)/0.5))*10;
	
}

void InitTimer0(void)
{
	//2kHz clock
	//WGM0 = 111 (fast PWM w/ top at OCR0A)
	//COM0B = 10;
	TCCR0A |= (1<<WGM00) | (1<<WGM01) | (1<<COM0B1); // Clear on Compare A
	TCCR0B |= (1<<WGM02);
	OCR0A = 124; // 2kHz
	OCR0B = 62; 
	TIMSK0 = (1<<OCIE0B) | (1<<OCIE0A);  // Enable Timer 0 Compare B, A ISR
	TCCR0B |= (1<<CS01) | (1<<CS00); // Set Prescaler 64 & Timer 0 starts
}

void InitTimer1(void)
{
	//WGM = Fast PWM w/ TOP at OCR1A = 1111
	//COM1B = 10, Clear OC1B on compare match, set on BOTTOM
	//CS1 = 1, prescaler = 1
	
	TCCR1A |= (1<<WGM10) | (1<<WGM11) | (1<<COM1B1);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	OCR1A = 3199; // 5kHz
	OCR1B = 1599;
	TIMSK1 |= (1<<OCIE1B); //Enable output compare B match interrupt
	TCCR1B |= (1<<CS10); //(1<<CS11) | (1<<CS10); // Start timer
}

ISR(ADC_vect)
{
	
	switch (ADMUX)
	{
		case 6:
			tempRead = ADC;
			tempDutyConversion();
			ADMUX = 7;
			break;
		case 7:
			brightness = ADC;
			brightDutyConversion();
			ADMUX = 6;
			break;
		default:
			break;
	}
	if(!secondRead)
	{
		secondRead = 1;
		startConversion();
	}
	else
		secondRead = 0;
}

ISR(TIMER0_COMPB_vect)
{
	//tempDutyConversion();
	if(tempDuty>0)
		OCR0B = (int)((tempDuty/100.0)*(OCR0A+1)) - 1;
	else
		OCR0B = 0;
}

ISR(TIMER1_COMPB_vect)
{
	//brightDutyConversion();
	if(brightDuty>0)
		OCR1B = (int)((brightDuty/100.0)*(OCR1A+1)) - 1;
	else
		OCR1B = 0;
}

ISR(TIMER0_COMPA_vect)
{
	if(counter == 79)
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
	DDRB |= (1<<DDB2); //OC1B // light
	DDRD |= (1<<DDD5); //OC0B // temperature
	InitTimer0();
	InitTimer1();
	setupADC();
	sei();
    while (1) 
    {
    }
}

