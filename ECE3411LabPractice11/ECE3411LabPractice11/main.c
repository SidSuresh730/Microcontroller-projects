/*
 * ECE3411LabPractice11.c
 *
 * Created: 26-Mar-20 22:32:38
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
#define LED_ON PORTB |= (1<<PORTB5)
#define LED_OFF PORTB &= ~(1<<PORTB5)

volatile int counter = 0;
char r_buffer[RX_BUFSIZE];
int r_index;
volatile char r_ready;
volatile int freq = 2;
const char* msg = "\nSwitch to frequency: ";
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
	OCR1A = (int)(62500/4) - 1;
	OCR1B = (int)((OCR1A + 1)/2) - 1;
	TIMSK1 |= (1<<OCIE1B) | (1<<OCIE1A); //Enable output compare B match interrupt
	TCCR1B |= (1<<CS12); // set prescaler & start timer
}

void UART_Init(unsigned int ubrr) 
{
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void getstr(void)
{
	r_ready = 0; //clear ready flag
	r_index = 0; //clear buffer
	UCSR0B |= (1<<RXCIE0); // enable receive interrupt
}

void writeMSG(void)
{
	for(int i=0; i<strlen(msg); i++)
			{
				uart_putchar(msg[i], &UART_Stream);
			}
}

ISR(TIMER1_COMPB_vect)
{
		LED_OFF;
}

ISR(TIMER1_COMPA_vect)
{
	if(freq != 0 || !(PINB & (1<<PINB7)))// freq is not 0
		LED_ON;
	else
		LED_OFF;
}

ISR(TIMER0_COMPA_vect)
{
	counter++;
}

ISR(USART0_RX_vect)
{
	char r_char = UDR0; 
	//Echo character back so that human  user can see
	UDR0 = r_char;
	
	if(r_char != '\r') // Enter NOT pressed
	{
		if(r_char == 127) // Backspace pressed ( using \b instead of 127 does NOT WORK)
		{
			--r_index; //move index back
			//uart_putchar(' ', &UART_Stream); //erase charater on screen
			//uart_putchar('\b', &UART_Stream); // backspace
		}
		else
		{
			r_buffer[r_index] = r_char;
			if(r_index < RX_BUFSIZE-1) {r_index++;}
			else {r_index = 0;}
		}
	}
	else
	{
		//uart_putchar('\n', &UART_Stream); // newline
		r_buffer[r_index] = 0; //terminate string with null character
		r_ready = 1;
		UCSR0B ^= (1<<RXCIE0); // disable receive interrupt 
	}
}

int getFreq(void)
{
	int freq = 0;
	int i;
	int mult10 = 1;
	if (r_index == 0)
		i = RX_BUFSIZE-1;
	else
		i = r_index - 1;
	while(r_buffer[i] != 0)
	{
			freq += (int)(r_buffer[i] - 48)*mult10;
			mult10 *= 10;
			if (i == 0)
				i = RX_BUFSIZE-1;
			else
				i--;
	}
	return freq;
}

int main(void)
{
	DDRB &= ~(1<<DDB7); //PB7, on-board button
	DDRB |= (1<<DDB5); //PB5, on-board LED
	UART_Init(MYUBRR);
	InitTimer1();
	InitTimer0();
	sei();
	getstr();
    /* Replace with your application code */
    while (1) 
    {
		if(counter>=5000) //5 seconds
		{
			counter = 0;
			writeMSG();
			getstr();
		}
	
		if(!(PINB & (1<<PINB7)))
		{
			OCR1A = (int)(62500/8) - 1;
			OCR1B = (int)((OCR1A+1)/2) - 1;
		}
		else
		{	
			if(r_ready) // string ready to be read
			{
				freq = getFreq();
				if(freq == 0)
				{
					LED_OFF;
				}
				getstr();
			}
			if(freq!=0)
				OCR1A = (int)(62500.0/(freq)) - 1;
			OCR1B = (int)((OCR1A+1)/2) - 1;
		}
    }
}

