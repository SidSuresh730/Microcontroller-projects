/*
 * ECE3411LabTest1.c
 *
 * Created: 04-Feb-20 21:40:44
 * Author : Siddarth Suresh
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    /* Replace with your application code */
	DDRD = 0b11111111; // set direction of DDRD to output
	DDRB &= ~(1<<DDB7); // set PORTB 7 to input (SW1) // Already debounced
	DDRB &= ~(1<<DDB1); // set PORTB 1 to input (SW2) // need to debounce
	int counter = 0;
	int increment = 1;
	int SW1_pressed = 0;
	int SW2_pressed = 0;
	//int SW1_once = 0;
	//int SW1_twice = 0;
	//int SW2_once = 0;
	//int SW2_twice = 0;
	int state = 0;
    while (1) 
    {
		_delay_ms(10); // debounce
		if(!(PINB & (1<<PINB7)) && !(PINB & (1<<PINB1))) // both pressed
		{
			SW2_pressed = 1;
			SW1_pressed = 1;
		}
		else if(!(PINB & (1<<PINB7))) // SW1 Pressed
		{
			SW1_pressed = 1;
		}
		else if (!(PINB & (1<<PINB1))) // SW2 Pressed
		{
			SW2_pressed = 1;
		}
		else
		{
			if(SW1_pressed && SW2_pressed)
			{
				if(increment + 5 < 25)
				{
					if (increment == 1)
					{
						increment += 4;
					}
					else
					{
						increment += 5;
					}
				}
				else
				{
					increment = 1;
				}
				state = 0;
				SW1_pressed = 0;
				SW2_pressed = 0;
			}
				else if (SW2_pressed)
				{
					if(state == 0) // nothing yet
					{
						state = 0;//still nothing
					}
					else if(state == 1)// SW1 once
					{
						state = 0; // break the sequence
					}
					else if(state == 2) // SW1 twice
					{
						state = 3; // move to SW2 once
					}
					else if (state == 3)
					{
						state = 4; // move to SW2 twice
					}
					else if (state == 4)
					{
						state = 0;
					}
					counter-=increment;
					SW2_pressed = 0;
				}
				else if (SW1_pressed)
				{
					
					if(state == 0)
					{
						state = 1;
					}
					else if(state == 1)
					{
						state = 2;
					}
					else if(state == 2)
					{
						state = 2;
					}
					else if(state == 3)
					{
						state = 0;
					}
					else if(state == 4)
					{
						state = 1;
					}
					counter+=increment;
					SW1_pressed = 0;
				}
				if(state == 4)
				{
					counter = 0;
				}
		PORTD = counter;
		_delay_ms(125);
		PORTD = 0b00000000;
		_delay_ms(125);
		}
		
    }
}

