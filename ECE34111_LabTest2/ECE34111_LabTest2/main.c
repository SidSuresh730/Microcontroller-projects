/*
 * ECE34111_LabTest2.c
 *
 * Created: 24-Feb-20 13:47:51
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
volatile uint32_t ms_count = 0;
volatile int OC1A_duty = 50;
volatile int OC1B_duty = 70;

void my_delay_ms(uint32_t setCount)
{
	ms_count = setCount;
	while(ms_count>0)
	{
	
	}
}

void InitTimer0(void)
{
		//1ms clock
		TCCR0A |= (1<<WGM01); // Clear on Compare A
		OCR0A = 249; // Set number of ticks for Compare A
		TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
		TCCR0B = 3; // Set Prescaler 64 & Timer 0 starts
}

void InitTimer1(void)
{
	//WGM = 1110 //Mode 14: Fast PWM with TOP = ICR1
	//COM1A = 10 //non-inverting PWM OC1A
	//COM1B = 10 //non-inverting PWM OC1B
	TCCR1A |= (1<<WGM11) | (1<<COM1A1) | (1<<COM1B1);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	ICR1 = 99; // 250K / (99+1) = 2.5KHz clock
	OCR1A = 49; //49
	OCR1B = 69; //69
	TIMSK1 |= (1<<OCIE1B) | (1<<OCIE1A); //Enable output compare B, A match interrupt
	TCCR1B |= (1<<CS11) | (1<<CS10); //prescaler = 64 and timer starts
}

void InitSW1(void)
{
	PCMSK0 |= (1<<PCINT7); //PCINT7 is SW1 (PB7)
	PCICR |= (1<<PCIE0); //PCINT7 is in PCIE0
}

void InitSW2(void)
{
	EICRA |= (1<<ISC11); //Set up INT1 Falling Edge Trigger 
	EIMSK |= (1<<INT1); //INT1 is SW2 (PD3)
}

ISR(PCINT0_vect) //ON BOARD SWITCH (SW1)
{
	if(!(PINB & (1<<PINB7))) //Falling Edge
	{
		if(OC1A_duty >= 100)
			OC1A_duty = 0;
		else
			OC1A_duty = (OC1A_duty+5); //increment OC1A_duty by 5 do not exceed 100 when SW1 pressed
	}
}

ISR(INT1_vect) //EXTERNAL SWITCH (SW2)
{
	if(OC1B_duty >= 100)
		OC1B_duty = 0;
	else
		OC1B_duty = (OC1B_duty + 10); //increment OC1B_duty by 10 do not exceed 100 when SW2 pressed
}

ISR(TIMER0_COMPA_vect)
{
	if(ms_count>0)
	{
		ms_count--;
	}
	else
	{
		my_delay_ms(ms_count);
	}
}

ISR(TIMER1_COMPA_vect)
{
	if(OC1A_duty == 0)
		OCR1A = 0;
	else
	{
		OCR1A = (OC1A_duty/100.0)*(ICR1+1) - 1;
	}
}

ISR(TIMER1_COMPB_vect)
{
	if(OC1B_duty == 0)
		OCR1B = 0;
	else
	{
		OCR1B = (OC1B_duty/100.0)*(ICR1+1) - 1;
	}
}

int main(void)
{
	DDRB |= (1<<DDB1) | (1<<DDB2) | (1<<DDB5); //OC1A OC1B onboardLED
	DDRB &= ~(1<<DDB7); //SW1
	DDRD &= ~(1<<DDD3); // SW2
	InitTimer0();
	InitTimer1();
	InitSW1();
	InitSW2();
	sei();
	
    while (1) 
    {
		my_delay_ms(16);
		PORTB |= (1<<PORTB5);
		my_delay_ms(17);
		PORTB &= ~(1<<PORTB5);
    }
}







