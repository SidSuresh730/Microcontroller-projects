/*
 * GccApplication1.c
 *
 * Created: 27-Jan-20 14:25:33
 * Author : Siddarth Suresh
 */ 

#define F_CPU 16000000UL //Set the clock frequency
#include <avr/io.h> //call IO library
#include <util/delay.h>

#define LED_ON PORTD |= (1<<PORTD5)
#define LED_OFF PORTD &= ~(1<<PORTD5)
#define LED_TOGGLE PINB |= (1<<PINB5)
#define REG_SIZE 8;

int main(void)
{
	int current_led = DDD0;
	DDRD |= (1<<current_led); //Set PORTD 0 to output (1 is output, 0 is input)
	DDRB &= ~(1<<DDB7); //Set PORTB 7 to input (BTN 1) 
	DDRB &= ~(1<<DDB1); //Set PORTB 1 to input (BTN 2)
	
    /* Replace with your application code */
    while (1) 
    {
		/*PORTD |= (1<<PORTD5); //Set PB5 to 1 which is high (LED On)
		_delay_ms(1000); //Delay for 1000ms 
		PORTD &= ~(1<<PORTD5); //Set PB5 to 0 which is low (LED off)
		_delay_ms(1000); //Delay for 1000ms 		
		*/
		if(!(PINB & (1<<PINB1))){ //If PINB1 is low (BTN2)
			current_led = (current_led+1)%REG_SIZE;
			DDRD |= (1<<current_led);
			if(!(PINB & (1<<PINB7))){ //If PINB7 is low as well
				PORTD |= (1<<current_led); //set current_led high
				_delay_ms(62.5); //Delay 125 ms
				PORTD &= ~(1<<current_led); //set current_led low
				_delay_ms(62.5); //Delay 125 ms
				//(current_led++)%REG_SIZE;
			}
			else{
				PORTD |= (1<<current_led); //set current_led high
				_delay_ms(250); //Delay 500 ms
				PORTD &= ~(1<<current_led); //set current_led low
				_delay_ms(250); //Delay 500 ms
				//(current_led++)%REG_SIZE;
			}
			
			
		}
		else if(!(PINB & (1<<PINB7))) //If PINB7 is low
		{
			//LED_ON;
			PORTD |= (1<<current_led); //set current_led high 
			_delay_ms(62.5); //delay 125 ms
			PORTD &= ~(1<<current_led); //set current_led low
			_delay_ms(62.5); //Delay 125 ms
		}
		else{
			//LED_OFF;
				PORTD |= (1<<current_led); //set current_led high 
				_delay_ms(250); //Delay 500 ms
				PORTD &= ~(1<<current_led); //set current_led low
				_delay_ms(250); //Delay 500 ms
		}		
	}
}




