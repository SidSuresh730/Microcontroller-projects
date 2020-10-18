/*
 * ECE3411LabTest5.c
 *
 * Created: 07-Apr-20 17:48:12
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <util/twi.h>
#include "uart.h"
#include "i2c_master.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define slave_addr 0x90

volatile int counter = 0;

const char* preamble_msg = "Current temperature = ";
const char* faren_msg = " degrees F\n";
const char* celc_msg = " degrees C\n";
const char* error_msg = "invalid cmd/param\n";
const char* correct_msg = "uart_update ";

double temperature = 0;
uint8_t receive;
volatile char r_buffer[RX_BUFSIZE];
int r_index;
volatile char r_ready;
volatile int freq = 2;
unsigned char listen_state = 1;
unsigned char invalid_msg_flag = 0;
unsigned char delete_flag = 0;
FILE UART_Stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void InitTimer0(void)
{
	//1ms clock
	TCCR0A |= (1<<WGM01); // Clear on Compare A
	OCR0A = 249; // Set number of ticks for Compare A
	TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
	TCCR0B = 3; // Set Prescaler 64 & Timer 0 starts
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

double celc_to_far(uint8_t celcius)
{
	return celcius*9.0/5.0 + 32;
}

void writeDouble(double number) // writes a double (XX.XX) to the buffer
{
	uart_putchar((int)number / 10 + 48, &UART_Stream);
	uart_putchar((int)number % 10 + 48, &UART_Stream); // + 48 converts the integer to the corresponding character in ASCII
	uart_putchar('.', &UART_Stream);
	uart_putchar((int)(number * 10) % 10 + 48, &UART_Stream);
	uart_putchar((int)(number * 100) % 10 + 48, &UART_Stream);
	//uart_putchar('\n', &UART_Stream);
}

void writeMSG(void) // prints the temperature
{
	for(int i=0; i<strlen(preamble_msg); i++)		// beginning part of temperature message
	{
		uart_putchar(preamble_msg[i], &UART_Stream);
	}
	if(!(PINB & (1<<PINB7)))						// IF on-board switch pressed
	{
		writeDouble(temperature);					// write default temperature
		for(int i=0; i<strlen(celc_msg); i++)
		{
			uart_putchar(celc_msg[i], &UART_Stream); // which is in degrees celcius
		}
		return;
	}
	writeDouble(celc_to_far(temperature));			//else  convert to farenheit
	for(int i=0; i<strlen(faren_msg); i++)
	{
		uart_putchar(faren_msg[i], &UART_Stream); 
	}
	
}

void writeErrMSG(void)								// write error message
{
		for(int i=0; i<strlen(error_msg); i++)
		{
			uart_putchar(error_msg[i], &UART_Stream);
		}
}

int getMSG(void)
{
	int i;
	int freq = 0;

	for(i=0; i<strlen(correct_msg); i++)			// check to make sure format is correct
	{
		if(r_buffer[i]!=correct_msg[i])
		{
			invalid_msg_flag = 1;
			return -1;								// invalid cmd
		}
	}
	while(i<RX_BUFSIZE)								// get the frequency 
	{
		if(r_buffer[i]>47 && r_buffer[i]<58)
		{
			freq*=10;
			freq+=(r_buffer[i]-48);
		}
		else if(r_buffer[i]==0)
			return freq;
		else
			return -1;								// invalid param
		i++;
	}
	return freq;
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
	
	if(r_char != '\r' && r_char != 0) // Enter NOT pressed
	{
		if(r_char == 8) // Backspace pressed 
		{
			if(r_index != 0)
				--r_index; //move index back
			uart_putchar(' ', &UART_Stream); //erase character on screen
			uart_putchar('\b', &UART_Stream); // backspace
			
		}
		else if(r_char == 127) // sent by pressing Ctrl + Backspace
		{
			r_buffer[r_index] = 0; //terminate string with null character
			r_ready = 1;
			listen_state = 1;		// raise listen flag
			delete_flag = 1;		// raise delete flag
			UCSR0B ^= (1<<RXCIE0); // disable receive interrupt
		}
		else
		{
			r_buffer[r_index] = r_char;
			if(r_index < RX_BUFSIZE-1) 
			{
				r_index++;
			}
			else {r_index = 0;}
		}
	}
	else
	{
		uart_putchar('\n', &UART_Stream); // newline
		r_buffer[r_index] = 0; //terminate string with null character
		r_ready = 1;
		UCSR0B ^= (1<<RXCIE0); // disable receive interrupt
	}
}


int main(void)
{
	DDRB &= ~(1<<DDB7); //PB7, on-board button
	i2c_init();
	UART_Init(MYUBRR);
	InitTimer0();
	sei();
	getstr();
	/* Replace with your application code */
	while (1)
	{
		if(listen_state)
		{
			if(r_ready)
			{
				freq = getMSG();
				if(freq == -1)		//INVALID COMMAND
				{
					if(!delete_flag) // delete character wasn't received
					{
						writeErrMSG(); // it's an actual error
					}
					getstr();			
					delete_flag = 0;	// deassert delete_flag 
					continue;
				}
				else
					listen_state = 0; // if you get an actual frequency, then exit the listening state
			}
		}
		if(counter>=1000*freq && !listen_state) //if not in listening state, every freq seconds
		{
			counter = 0;
			i2c_transmit(slave_addr, 0x00, 8);			// get temperature i2c
			i2c_receive(slave_addr, &receive, 8);
			temperature = receive;
			writeMSG();
			getstr();									// keep this in case delete is 
		}
	}
}
