/*
 * ECE3411LabPractice12.c
 *
 * Created: 03-Apr-20 12:19:43
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
const char* msg_1 = "Current Temp = ";
const char* msg_2 = " degrees F\n";
double temperature = 0;
uint8_t receive;

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

void writeMSG(void)
{
	for(int i=0; i<strlen(msg_1); i++)
	{
		uart_putchar(msg_1[i], &UART_Stream);
	}
	writeDouble(celc_to_far(temperature));
	for(int i=0; i<strlen(msg_2); i++)
	{
		uart_putchar(msg_2[i], &UART_Stream);
	}
	
}

ISR(TIMER0_COMPA_vect)
{
	counter++;
}

int main(void)
{
    /* Replace with your application code */
	i2c_init();
	UART_Init(MYUBRR);
	InitTimer0();
	sei();
    while (1) 
    {
		if(counter>=4999) // 5 seconds
		{
		counter = 0;
		i2c_transmit(slave_addr, 0x00, 8);
		i2c_receive(slave_addr, &receive, 8);
		temperature = receive;
		writeMSG();
		}
    }
}

