/*
 * ECE3411LabPractice13.c
 *
 * Created: 13-Apr-20 13:58:29
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
const char* i2c_msg = "I2C Temp = ";
const char* adc_msg = "ADC Temp = ";
const char* farenheit_msg = " degrees F\n";
volatile double i2c_temp = 0;
volatile double adc_temp = 0;
volatile int tempRead = 0;
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

double celc_to_far(double celcius)
{
	return celcius*9.0/5.0 + 32;
}

void setupADC(void)
{
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1); // ADC enable, interrupt enable
	// prescaler = 64
	ADMUX = 7;
}

void startConversion(void)
{
	ADCSRA |= (1<<ADSC);
}


void tempDutyConversion(void)
{
	//0.4; // V0degrees
	//0195; //Temperature coefficient
	adc_temp = (tempRead*5.0/1024.0 - 0.4)/(.0195);
}

void writeDouble(double number) // writes a double (XX.XX) to the buffer
{
	if(number > 99)
		uart_putchar((int)number / 100 + 48, &UART_Stream);
	uart_putchar((int)number %100 / 10 + 48, &UART_Stream);
	uart_putchar((int)number % 10 + 48, &UART_Stream); // + 48 converts the integer to the corresponding character in ASCII
	uart_putchar('.', &UART_Stream);
	uart_putchar((int)(number * 10) % 10 + 48, &UART_Stream);
	uart_putchar((int)(number * 100) % 10 + 48, &UART_Stream);
	//uart_putchar('\n', &UART_Stream);
}

void writeMSG(void)
{
	for(int i=0; i<strlen(i2c_msg); i++)
	{
		uart_putchar(i2c_msg[i], &UART_Stream);
	}
	writeDouble(celc_to_far(i2c_temp));
	for(int i=0; i<strlen(farenheit_msg); i++)
	{
		uart_putchar(farenheit_msg[i], &UART_Stream);
	}
	for(int i=0; i<strlen(adc_msg); i++)
	{
		uart_putchar(adc_msg[i], &UART_Stream);
	}
	writeDouble(celc_to_far(adc_temp));
	for(int i=0; i<strlen(farenheit_msg); i++)
	{
		uart_putchar(farenheit_msg[i], &UART_Stream);
	}
	uart_putchar('\n', &UART_Stream);
	
}

ISR(TIMER0_COMPA_vect)
{
	counter++;
}

ISR(TWI1_vect)
{
	if(TWSR1 & 0x50) //Data byte received, ack returned
	{
		i2c_temp = receive;
		TWCR1 |= (1<<TWINT1);
		startConversion();
	}
}
ISR(ADC_vect)
{
	tempRead = ADC;
	tempDutyConversion();
	writeMSG();
}
int main(void)
{
	/* Replace with your application code */
	i2c_init();
	UART_Init(MYUBRR);
	InitTimer0();
	setupADC();
	sei();
	while (1)
	{
		if(counter>=1999) // 2 seconds
		{
			counter = 0;
			i2c_transmit(slave_addr, 0x00, 8);
			i2c_receive(slave_addr, &receive, 8);
			TWCR1 |= (1<<TWIE1) | (1<<TWEN1) | (1<<TWSTA1);
		}
	}
}


