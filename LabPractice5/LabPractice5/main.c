/*
 * LabPractice5.c
 *
 * Created: 16-Feb-20 20:38:21
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#define NoPush 0
#define Maybe 1
#define Pushed 2
#define DEBOUNCE_TIME 10

unsigned char pushState = NoPush; //state variable
volatile int timerCount = DEBOUNCE_TIME;
unsigned char current_led = PORTD2;
unsigned char freq = 3;
volatile int led_on = 0;
unsigned char increasing = 1;
unsigned char push_debounce = 0;
int SW1_pressed = 0;
int SW2_pressed = 0;

void buttonSM(void)
{
	switch (pushState)
	{
		case NoPush:
		if (!(PINB & (1<<PINB1)))
		pushState=Maybe;
		else
		pushState=NoPush;
		break;
		case Maybe:
		if (!(PINB & (1<<PINB1)))
		{
			pushState=Pushed;
			push_debounce=1;
		}
		else
		{
			pushState=NoPush;
			push_debounce=0;
		}
		break;
		case Pushed:
		if (!(PINB & (1<<PINB1)))
		pushState=Pushed;
		else
		pushState=Maybe;
		break;
	}
}

void toggle_LED(void)
{
	if(led_on) //set current_led high
	{
		PORTD = 0; // Guarantees that when current_led changes, an LED won't be left on
		//PORTD &= ~(1<<current_led); //set current_led low
		led_on = 0;
	}
	else
	{
		PORTD |= (1<<current_led); //set current_led high
		led_on = 1;
	}
}

ISR(TIMER0_COMPA_vect)
{
	if (timerCount >0)
	{
		timerCount--;
		} else {
		buttonSM();
		timerCount=DEBOUNCE_TIME;
	}
}

ISR(TIMER1_COMPA_vect)
{
	toggle_LED();
}

void InitTimer1(void)
{
	TCCR1B |= (1<<WGM12); //CLear on Compare A
	OCR1A = (int)(62500/3) - 1;
	TIMSK1 = (1<<OCIE1A); //Enable Timer 1 Compare A ISR
	TCCR1B |= (1<<CS12); //Set prescaler & Timer 1 starts
}

// 1 ms ISR for Timer 0 assuming F_CPU = 16MHz
void InitTimer0(void)
{
	TCCR0A |= (1<<WGM01); // Clear on Compare A
	OCR0A = 249; // Set number of ticks for Compare A
	TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
	TCCR0B = 3; // Set Prescaler & Timer 0 starts
}

int main(void)
{
	DDRD = 0b11111111; // set direction of DDRD to output
	DDRB &= ~(1<<DDB7); // set PORTB 7 to input (SW1) // Already debounced
	DDRB &= ~(1<<DDB1); // set PORTB 1 to input (SW2) // need to debounce
	//int SW1_pressed = 0;
	//int SW2_pressed = 0;
	InitTimer0();
	InitTimer1();
	sei();
	while (1)
	{
		if(!(PINB & (1<<PINB7)) && pushState == Pushed) // both pressed
		{
			SW2_pressed = 1;
			SW1_pressed = 1;
		}
		else if(!(PINB & (1<<PINB7))) // SW1 Pressed
		{
			SW1_pressed = 1;
		}
		else if (pushState == Pushed) // SW2 Pressed
		{
			SW2_pressed = 1;
		}
		else
		{
			if(SW1_pressed && SW2_pressed)
			{
				if(current_led == DDD3)
				{
					increasing = 0;
					current_led--;
				}
				else if (current_led == DDD0)
				{
					increasing = 1;
					current_led++;
				}
				else
				{
					if (increasing)
					{
						current_led++;
					}
					else
					{
						current_led--;
					}
				}
				
				SW1_pressed = 0;
				SW2_pressed = 0;
			}
			else if (SW2_pressed)
			{
				if(freq > 3)
				freq--;
				SW2_pressed = 0;
			}
			else if (SW1_pressed)
			{
				if(freq < 10)
				freq++;
				SW1_pressed = 0;
			}
		}
		OCR1A = (int)(62500/(2*freq)) - 1;
	}
}

