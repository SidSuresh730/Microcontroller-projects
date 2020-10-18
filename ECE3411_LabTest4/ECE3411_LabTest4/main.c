/*
 * ECE3411_LabTest4.c
 *
 * Created: 24-Mar-20 22:50:15
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"

#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
#define LED_ON PINB |= (1<<PINB5)
#define LED_OFF PINB &= ~(1<<PINB5)

volatile double adc0_volt = 0; //Voltage readings
volatile double adc1_volt = 0; // to be displayed
volatile double adc2_volt = 0; // by the UART
volatile double adc3_volt = 0; //
volatile double adc4_volt = 0; //
volatile int led_vnum = 0; // normalized voltage number
volatile int counter2sec = 0; // 2 second counter for a 1msec clock
volatile int counter40msec = 0; // 40 msec counter to run the adc conversion
volatile unsigned char conversions_done = 0; // flag that signals when conversions for all ADC readings are done
FILE UART_Stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW); // buffer for UART communication
const char* boundary =  "/////////////////////\n"; // boundary string
const char* adc0_sub = "ADC0 = "; // Strings 
const char* adc1_sub = "ADC1 = "; // to be 
const char* adc2_sub = "ADC2 = "; // displayed
const char* adc3_sub = "ADC3 = "; // by the UART
const char* adc4_sub = "ADC4 = "; //
const char* led_vnum_sub = "LED Vnum = "; // 
const char* volts = " Volts\n"; //






void setupADC(void)
{
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1); // ADC enable, interrupt enable
	// prescaler = 64
	ADMUX = 0;	
}

void startConversion(void) // run a single conversion
{
	ADCSRA |= (1<<ADSC);
}

void UART_Init(unsigned int ubrr) 
{
	UBRR0H = (unsigned char) (ubrr>>8);
	UBRR0L = (unsigned char) ubrr;
	UCSR0B = (1<<RXEN0) | (1<<TXEN0);
	UCSR0C = (1<<USBS0)|(3<<UCSZ00);
}

void InitTimer0(void)
{
	//1ms clock
	TCCR0A |= (1<<WGM01); // Clear on Compare A
	OCR0A = 249; // Set number of ticks for Compare A
	TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
	TCCR0B = 3; // Set Prescaler 64 & Timer 0 starts
}

void writeDouble(double number) // writes a double (X.XX) to the buffer 
{
	uart_putchar((int)number + 48, &UART_Stream); // + 48 converts the integer to the corresponding character in ASCII
	uart_putchar('.', &UART_Stream);
	uart_putchar((int)(number * 10) % 10 + 48, &UART_Stream);
	uart_putchar((int)(number * 100) % 10 + 48, &UART_Stream);
	//uart_putchar('\n', &UART_Stream);
}

void writeInt(int number) // writes an integer (Y) to the buffer
{
	if(number >= 10)
		uart_putchar(number/10 + 48, &UART_Stream);
	uart_putchar(number % 10 + 48, &UART_Stream);
	uart_putchar('\n', &UART_Stream);
}

void writeMSG(void) // generates the entire message sent by UART formatted properly
{
	for(int i=0; i<strlen(boundary); i++) 
	{
		uart_putchar(boundary[i], &UART_Stream);
	}
	for(int i=0; i<strlen(adc0_sub); i++)
	{
		uart_putchar(adc0_sub[i], &UART_Stream);
	}
	writeDouble(adc0_volt);
	for(int i=0; i<strlen(volts); i++)
	{
		uart_putchar(volts[i], &UART_Stream);
	}
	for(int i=0; i<strlen(adc1_sub); i++)
	{
		uart_putchar(adc1_sub[i], &UART_Stream);
	}
	writeDouble(adc1_volt);
		for(int i=0; i<strlen(volts); i++)
	{
		uart_putchar(volts[i], &UART_Stream);
	}
	for(int i=0; i<strlen(adc2_sub); i++)
	{
		uart_putchar(adc2_sub[i], &UART_Stream);
	}
	writeDouble(adc2_volt);
	for(int i=0; i<strlen(volts); i++)
	{
		uart_putchar(volts[i], &UART_Stream);
	}
	for(int i=0; i<strlen(adc3_sub); i++)
	{
		uart_putchar(adc3_sub[i], &UART_Stream);
	}
	writeDouble(adc3_volt);
	for(int i=0; i<strlen(volts); i++)
	{
		uart_putchar(volts[i], &UART_Stream);
	}
	for(int i=0; i<strlen(adc4_sub); i++)
	{
		uart_putchar(adc4_sub[i], &UART_Stream);
	}
	writeDouble(adc4_volt);
	for(int i=0; i<strlen(volts); i++)
	{
		uart_putchar(volts[i], &UART_Stream);
	}
	for(int i=0; i<strlen(led_vnum_sub); i++)
	{
		uart_putchar(led_vnum_sub[i], &UART_Stream);
	}	
	writeInt(led_vnum);
	for(int i=0; i<strlen(boundary); i++)
	{
		uart_putchar(boundary[i], &UART_Stream);
	}
	
}

ISR(ADC_vect)		// this ISR is triggered when a conversion is done and 
{					// triggers another conversion until adc4 conversion is done
	switch (ADMUX)
	{
		case 0:
			adc0_volt = (ADC*5/1024.0);
			ADMUX = 1;
			break;
		case 1:
			adc1_volt = (ADC*5/1024.0);
			ADMUX = 2;
			break;
		case 2:
			adc2_volt = (ADC*5/1024.0);
			ADMUX = 3;
			break;
		case 3:
			adc3_volt = (ADC*5/1024.0);
			ADMUX = 4;
			break;
		case 4:
			adc4_volt = (ADC*5/1024.0);  
			led_vnum = (int)(adc4_volt/0.5); // calculates normalized vnum
			PORTD = (led_vnum << 2); // displays vnum on Ports D2 - D5
			ADMUX = 0;
			conversions_done = 1; // signals end of all conversions
			break;
		default:
			break;
	}
	if(!conversions_done) // if adc4 conversion hasn't been completed, run the next conversion
	{
		startConversion();
	}
	else
		conversions_done = 0;
}

ISR(TIMER0_COMPA_vect)
{
	counter2sec++;
	counter40msec++;
	if(counter2sec == 1999) // 2 seconds have passed, display the UART message
	{
		counter2sec = 0;
		writeMSG();
	}
	if(counter40msec == 39) // 40msec have passed, run ADC conversions again
	{
		counter40msec = 0;
		startConversion();
	}
}
int main(void)
{
	DDRD |= (1<<DDD2) | (1<<DDD3) | (1<<DDD4) | (1<<DDD5);
	InitTimer0();
	UART_Init(MYUBRR);
	setupADC();
	sei();
    /* Replace with your application code */
    while (1) 
    {
    }
}

