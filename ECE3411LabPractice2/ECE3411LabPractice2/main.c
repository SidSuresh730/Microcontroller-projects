/*
 * GccApplication1.c
 *
 * Created: 29-Jan-20 14:25:33
 * Author : Siddarth Suresh
 */ 

#define F_CPU 16000000UL //Set the clock frequency
#include <avr/io.h> //call IO library
#include <util/delay.h>

#define LED_ON PORTD |= (1<<PORTD5)
#define LED_OFF PORTD &= ~(1<<PORTD5)
#define LED_TOGGLE PINB |= (1<<PINB5)
#define REG_SIZE 8


void delay_by(int freq)
{
	for(int i = 0; i < freq; i++)
	{
		_delay_ms(1);
	}
}
int main(void)
{
	unsigned char current_led = DDD2;
	int freq = 3;
	int increasing = 1;
	int sw1_pressed = 0;
	int sw2_pressed = 0;
	DDRD = 0b11111111; //Set PORTD to output
	DDRB &= ~(1<<DDB7); //Set PORTB 7 to input (SW1) (HW Debounced)
	DDRB &= ~(1<<DDB1); //Set PORTB 1 to input (SW2) (NEED TO DEBOUNCE)
	
    /* Replace with your application code */
    while (1) 
    {
		_delay_ms(10);
		if(!(PINB & (1<<PINB7)))//SW1 pressed low
		{
			if(!(PINB & (1<<PINB1))) // SW2
			{
				sw2_pressed = 1;
			}
			sw1_pressed = 1;
		}
		else if(!(PINB & (1<<PINB1))) //SW2 pressed low
		{
			if(!PINB & (1<<PINB7))
			{
				sw1_pressed = 1;
			}
			sw2_pressed = 1;
		} 
		else
		{
			if (sw2_pressed && sw1_pressed)
			{
					if(current_led + 1 == REG_SIZE)
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
					sw1_pressed = 0;
					sw2_pressed = 0;
			} 
			else if (sw2_pressed)
			{
					if (freq > 3)
					{
						freq--;
					}
					sw2_pressed = 0;
			}
			else if (sw1_pressed)
			{
				if(freq < 10)
				{
					freq++;
				}
				sw1_pressed=0;
			}
			PORTD |= (1<<current_led); //set current_led high
			delay_by((int)(500/freq));
			//_delay_ms(500); //Delay half a period
			PORTD &= ~(1<<current_led); //set current_led low
			delay_by((int)(500/freq));
			//_delay_ms(500); //Delay half a period
			}
	}
}







