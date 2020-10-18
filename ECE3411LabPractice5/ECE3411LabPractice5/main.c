/*
 * ECE3411LabPractice5.c
 *
 * Created: 12-Feb-20 16:00:49
 * Author : Siddarth Suresh
 */ 
#include <avr/io.h>
#include <avr/interrupt.h>

#define NoPush 0
#define Maybe 1
#define Pushed 2
#define DEBOUNCE_TIME 10
#define LED1 PORTD0
#define LED2 PORTD7

//unsigned char pushState = NoPush; //state variable
//volatile int timerCount = DEBOUNCE_TIME;
//unsigned char current_led = PORTD2;
unsigned char freq = 1;
volatile int led1_on = 0;
volatile int led2_on = 0;
//unsigned char increasing = 1;
//unsigned char push_debounce = 0;
int SW1_pressed = 0;
int SW2_pressed = 0;
int pushCount = 0;
int releaseCount = 0;
int counter = 0;
unsigned char pause = 0;
int pause_count = 0;
int fifty_ms_count = 0;
/*
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
*/
void toggle_LED1(void)
{
	if(led1_on) //set current_led high	{		//PORTD = 0; // Guarantees that when current_led changes, an LED won't be left on		PORTD &= ~(1<<LED1); //set LED1 low		led1_on = 0;	}	else	{		PORTD |= (1<<LED1); //set LED1 high		led1_on = 1;	}
}
void toggle_LED2(void)
{
	if(led2_on) //set current_led high	{		//PORTD = 0; // Guarantees that when LED changes, an LED won't be left on		PORTD &= ~(1<<LED2); //set LED2 low		led2_on = 0;	}	else	{		PORTD |= (1<<LED2); //set LED2 high		led2_on = 1;	}
}

ISR(TIMER0_COMPA_vect) // controls LED1
{
	if(pause_count==0)
	{
		if(fifty_ms_count == 0)
		{
			toggle_LED1();
			fifty_ms_count = 25;
		}
		else
			fifty_ms_count--;
	}
	else
		pause_count--;
}ISR(TIMER1_COMPA_vect) // controls LED2{	toggle_LED2();}
ISR(PCINT0_vect) // turns off LED1
{
	//Turn off LED1 for 5 seconds;
	if(!(PINB & (1<<PINB3)))
	{
		/*releaseCount = 0;
		pushCount++;
		if (pushCount > 500)
		{
			counter++;
			pushCount = 0;
			PORTD &= ~(1<<LED1);
			pause_count = 100;
		}
		else
		{
			pushCount = 0;
			releaseCount++;
			if(releaseCount > 500)
			{
				releaseCount = 0;
			}
		}*/
		PORTD &= ~(1<<LED1);
		pause_count = 5000;
	}
}

void InitPCINT3(void)
{
	PCMSK0 |= (1<<PCINT3); //Enable PB[3] as PCInterrupt PCINT3
	PCICR |= (1<<PCIE0); //Enables Pin Change Interrupt 0, PCI0 Interrupt Vector

}

void InitTimer1(void)
{
	TCCR1B |= (1<<WGM12); //CLear on Compare A
	OCR1A = (int)(62500/(2*freq)) - 1;
	TIMSK1 = (1<<OCIE1A); //Enable Timer 1 Compare A ISR
	TCCR1B |= (1<<CS12); //Set prescaler 256 & Timer 1 starts
}

// 20 Hz ISR for Timer 0
void InitTimer0(void)
{
	//1ms clock
	TCCR0A |= (1<<WGM01); // Clear on Compare A
	OCR0A = 249; // Set number of ticks for Compare A
	TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
	TCCR0B = 3; // Set Prescaler 64 & Timer 0 starts
}

int main(void)
{
	DDRD = 0b10000001; // set direction of DDRD0 and 1k to output
	DDRB &= ~(1<<DDB7); // set PORTB 7 to input (SW1) // Already debounced
	DDRB &= ~(1<<DDB3); // set PORTB 3 to input (SW2) // need to debounce
	//int SW1_pressed = 0;
	//int SW2_pressed = 0;
	InitPCINT3();
	InitTimer0();
	InitTimer1();
	sei();
	while (1)
	{
		
		if(!(PINB & (1<<PINB7))) // SW1 Pressed
		{
			SW1_pressed = 1;
		}
		else
		{
			if (SW1_pressed)
			{
				if(freq == 1)
					freq = 2;
				else if (freq == 2)
					freq = 1;
				SW1_pressed = 0;
				OCR1A = (int)(62500/(2*freq)) - 1;
			}
		}
		
	}
}

