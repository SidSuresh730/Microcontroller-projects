/*
 * ECE3411LabPractice15.c
 *
 * Created: 20-Apr-20 13:52:18
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <string.h>
#include "uart.h"
#define SPI_DDR DDRB
#define SPI_SS 2
#define SPI_MOSI 3
#define SPI_SCK 5
#define SPI_MISO 4
#define SPI_LDAC 0
#define DAC_COMMAND 0b01110000
#define F_CPU 16000000UL
#define BAUD 9600
#define MYUBRR F_CPU/16/BAUD-1
volatile unsigned int counter = 0;
volatile double potVolt = 0;
volatile double dacVolt = 0;
int temp = 0;
volatile unsigned char isSecondHalf = 0;
const char* pot_preamble = "Pot Voltage: ";
const char* dac_preamble = "DAC Voltage: ";
FILE UART_Stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW);

void InitTimer0(void)
{
	//1ms clock
	TCCR0A |= (1<<WGM01); // Clear on Compare A
	OCR0A = 249; // Set number of ticks for Compare A
	TIMSK0 = (1<<OCIE0A);  // Enable Timer 0 Compare A ISR
	TCCR0B = 3; // Set Prescaler 64 & Timer 0 starts
}

//Initialize ADC
void setupADC(void)
{
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | // ADC enable, interrupt enable
	(1<<ADPS1); // prescaler = 64
	ADMUX = 4; // Potentiometer
}

void startConversion(void)
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

void SPI_MasterInit(void)
{
	/* Set SS, MOSI and SCK output, all others input */
	SPI_DDR = (1<<SPI_SS) | (1<<SPI_MOSI) |  (1<<SPI_SCK) | (1<<SPI_LDAC);
	/* Enable SPI, Master, set clock rate fck/128, enable interrupt*/
	SPCR0 = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0) | (1<<SPIE);
}

uint8_t SPI_Master_Transceiver(uint8_t cData)
{
	PORTB &= ~(1<<SPI_SS); // Pull Slave_Select low
	SPDR0 = cData; // Start transmission
	while( !(SPSR0 & (1<<SPIF)) ); // Wait for transmission complete
	PORTB |= (1<<SPI_SS); // Pull Slave Select High
	return SPDR0; // Return received data
}

void writeDouble(double number) // writes a double (X.XX) to the buffer
{
	//if(number > 99)
	//uart_putchar((int)number / 100 + 48, &UART_Stream);
	//uart_putchar((int)number %100 / 10 + 48, &UART_Stream);
	uart_putchar((int)number % 10 + 48, &UART_Stream); // + 48 converts the integer to the corresponding character in ASCII
	uart_putchar('.', &UART_Stream);
	uart_putchar((int)(number * 10) % 10 + 48, &UART_Stream);
	uart_putchar((int)(number * 100) % 10 + 48, &UART_Stream);
	uart_putchar('\n', &UART_Stream);
}

void writeMSG(void)
{
	
	for(int i=0; i<strlen(pot_preamble); i++)
	{
		uart_putchar(pot_preamble[i], &UART_Stream);
	}
	writeDouble(potVolt);
	for(int i=0; i<strlen(dac_preamble); i++)
	{
		uart_putchar(dac_preamble[i], &UART_Stream);
	}
	writeDouble(dacVolt);
	uart_putchar('\n', &UART_Stream);
	
}
/*
int SPI_Master_Transmit_12(int cData)
{
	PORTB &= ~(1<<SPI_SS); // Pull Slave_Select low
	PORTB |= (1<<SPI_LDAC); // Pull LDAC high if not already
	SPDR0 = DAC_COMMAND | (cData >> 8); // transmit 4bit command + 4 MSB of 12 bit data
	while( !(SPSR0 & (1<<SPIF)) ); // Wait for transmission complete
	SPDR0 = cData; // transmit 8 LSB of 12 bit data
	while( !(SPSR0 & (1<<SPIF)) ); // Wait for transmission complete
	PORTB |= (1<<SPI_SS); // Pull Slave Select High
	PORTB &= ~(1<<SPI_LDAC); // Pull LDAC low
	ADMUX = 5;
	startConversion();
	return SPDR0;
}
*/

/*
void SPI_SlaveInit(void)
{
	// Set MISO output, all others input 
	SPI_DDR = (1<<SPI_MISO);
	// Enable SPI 
	SPCR0 = (1<<SPE);
}
*/
	
/*
uint8_t SPI_SlaveReceive(void)
{
	// Wait for reception complete
	while(!(SPSR0 & (1<<SPIF)));
	// Return Data Register 
	return SPDR0;
}
*/
	
ISR(TIMER0_COMPA_vect)
{
	counter++;
}

ISR(ADC_vect)
{
	
	switch (ADMUX)
	{
		case 4:
			potVolt = ADC*(5.0/1024);
			ADMUX = 5;
			temp = ADC << 2; // Normalize ADC to 12 bits
			PORTB &= ~(1<<SPI_SS); // Pull Slave_Select low
			PORTB |= (1<<SPI_LDAC); // Pull LDAC high if not already
			SPDR0 = DAC_COMMAND | (temp >> 8); // transmit 4bit command + 4 MSB of 12 bit data
			break;
		case 5:
			dacVolt = ADC*(5.0/1024);
			PORTB |= (1<<SPI_LDAC); //reassert LDAC
			ADMUX = 4;
			writeMSG();
			break;
		default:
			break;
	}
}

ISR(SPI0_STC_vect)
{
	if(!isSecondHalf)
	{
		isSecondHalf = 1;
		SPDR0 = temp;
	}
	else
	{
		isSecondHalf = 0;
		PORTB |= (1<<SPI_SS); // Pull SS high again
		PORTB &= ~(1<<SPI_LDAC); // Pull LDAC low
		startConversion();
	}
}

int main(void)
{
	InitTimer0();
	UART_Init(MYUBRR);
	setupADC();
	SPI_MasterInit();
	sei();
	/* Replace with your application code */
	while (1)
	{
		if(counter >= 999)
		{
			counter = 0;
			ADMUX = 4;
			startConversion();
		}
		
	}
}




