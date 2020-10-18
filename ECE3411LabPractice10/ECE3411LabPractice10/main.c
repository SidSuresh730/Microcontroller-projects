/*
 * ECE3411LabPractice10.c
 *
 * Created: 11-Mar-20 14:33:13
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include "uart.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define LED_ON PINB |= (1<<PINB5)
#define LED_OFF PINB &= ~(1<<PINB5)

const char* msg = "\nDo you want to change the LED mode? (Y/N)\n";
volatile int counter = 0;
FILE UART_Stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

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
	//WGM = Fast PWM w/ TOP at OCR1A = 1111
	//COM1B = 10, Clear OC1B on compare match, set on BOTTOM
	//CS1 = 11, prescaler = 256
	
	//2 kHz clock
	TCCR1A |= (1<<WGM10) | (1<<WGM11) | (1<<COM1B1);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	OCR1A = (int)(62500/2) - 1;
	OCR1B = (int)((OCR1A + 1)/2) - 1;
	TIMSK1 |= (1<<OCIE1B) | (1<<OCIE1A); //Enable output compare B match interrupt
	TCCR1B |= (1<<CS12); // set prescaler & start timer
}

char* readMSG(void)
{
	static char buf[RX_BUFSIZE];
	char a;
	int index = 0;
	do 
	{
		a = uart_getchar(&UART_Stream);
		buf[index] = a;
		index++;
	} while (a != '\n' && a != '\0');
	buf[index] = '\0';
	return &buf;
}

void writeMSG(void)
{
	for(int i=0; i<strlen(msg); i++)
			{
				uart_putchar(msg[i], &UART_Stream);
			}
}

void changeMode(void)
{
	if(OCR1A == ((int)(62500/2) - 1))
		{
			OCR1A = (int)(62500/8) - 1;
		}
		else if(OCR1A = ((int)(62500/8) - 1))
		{
			OCR1A = (int)(62500/2) - 1;
		}
		OCR1B = (int)((OCR1A+1)/2) - 1;	
}

ISR(TIMER1_COMPB_vect)
{
	LED_OFF;
}

ISR(TIMER1_COMPA_vect)
{
	LED_ON;
}

ISR(TIMER0_COMPA_vect)
{
	counter++;
}

void UART_Init(unsigned int ubrr) 
{
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

int USART_Transmit(char c) // write a char
{
while ( !(UCSR0A & (1<<UDRE0)) ) ;
UDR0 = c;
return 0;
}

unsigned char USART_Receive(void) // read a char
{
/* wait for data to be received */
while ( !(UCSR0A & (1<<RXC0)) ) ;
/* get and return received data from buffer */
return UDR0;
}

int main(void)
{
    /* Replace with your application code */
	UART_Init(MYUBRR);
	//DDRB &= ~(1<<DDB7); //PB7, on-board button
	DDRB |= (1<<DDB5); //PB5, on-board LED
	InitTimer1();
	InitTimer0();
	sei();
			//USART_Transmit(msg);
			//a = uart_getchar(&UART_Stream);
			//uart_putchar(a, &UART_Stream);
    while (1) 
    {
			if(counter >= 10000) // 10 s
			{
				//Do Stuff
				counter = 0;
				writeMSG();
				const char* ans = readMSG();
				if(strcmp((const char*)ans, "YES\n") == 0)
				{
					changeMode();
				}
				else if(strcmp(ans, "NO\n") == 0)
				{
					//DO NOTHING
				}
				else
				{
					//ERROR
				}
			}

		/*
		if(!(PINB & (1<<PINB7)))
		{
			OCR1A = (int)(62500/8) - 1;
		}
		else
		{
			OCR1A = (int)(62500/2) - 1;
		}
		OCR1B = (int)((OCR1A+1)/2) - 1;	
		*/
    }
}

