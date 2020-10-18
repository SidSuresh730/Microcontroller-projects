/*
 * ECE3411LabTest6.c
 *
 * Created: 21-Apr-20 21:40:00
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <util/twi.h>
#include "uart.h"
#include "i2c_master.h"
#define SPI_DDR DDRB
#define SPI_SS 2
#define SPI_MOSI 3
#define SPI_SCK 5
#define SPI_MISO 4
#define SPI_LDAC 0
#define DAC_COMMAND 0b01110000
//#define F_CPU 16000000UL
//#define BAUD 9600
//#define MYUBRR F_CPU/16/BAUD-1
#define slave_addr 0x90

volatile char r_buffer[RX_BUFSIZE];
volatile unsigned char changeUnitFlag = 0;
volatile unsigned char writeMSGFlag = 0;
volatile unsigned char isSecondHalf = 0;
volatile int tempVar;
int r_index;
volatile char r_ready;
volatile int counter = 0;
volatile int i2c_temp = 0;
volatile double i2c_volt = 0;
volatile double dac_volt = 0;
volatile double adc_temp = 0;
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

void getstr(void)
{
	r_ready = 0; //clear ready flag
	r_index = 0; //clear buffer
	UCSR0B |= (1<<RXCIE0); // enable receive interrupt
}

void setupADC(void)
{
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | (1<<ADPS1); // ADC enable, interrupt enable
	// prescaler = 64
	ADMUX = 7;
}

void SPI_MasterInit(void)
{
	/* Set SS, MOSI and SCK output, all others input */
	SPI_DDR = (1<<SPI_SS) | (1<<SPI_MOSI) |  (1<<SPI_SCK) | (1<<SPI_LDAC);
	/* Enable SPI, Master, set clock rate fck/128, enable interrupt*/
	SPCR0 = (1<<SPE) | (1<<MSTR) | (1<<SPR1) | (1<<SPR0) | (1<<SPIE);
}

void startConversion(void)
{
	ADCSRA |= (1<<ADSC);
}

void SPI_sendFirstHalf(int cData) 
{
	PORTB &= ~(1<<SPI_SS); // Pull Slave_Select low
	PORTB |= (1<<SPI_LDAC); // Pull LDAC high if not already
	SPDR0 = DAC_COMMAND | (cData >> 8); // transmit 4bit command + 4 MSB of 12 bit data

}

void SPI_sendSecondHalf(int cData)
{
	SPDR0 = cData;
}

double celc_to_far(uint8_t celcius)
{
	return celcius*9.0/5.0 + 32;
}

void writeMSG()
{
	int TC74_C = i2c_temp;										// celcius TC74 value
	int MCP_int_C = (int)(adc_temp); 							// celcius MCP value truncated 
	int MCP_dec_C = ((int)(adc_temp*100))%100;					// celcius two decimal places of MCP value
	int CV_int = (int)(i2c_volt);								// converted voltage truncated
	int CV_dec = ((int)(i2c_volt*100))%100;						// two decimal places of converted voltage
	int DAC_int = (int)(dac_volt);								// dac voltage truncated
	int DAC_dec = ((int)(dac_volt*100))%100;					// two decimal places of dac voltage
	int TC74_int_F = (int)celc_to_far(i2c_temp);				// faren TC74 truncated
	int TC74_dec_F = ((int)(celc_to_far(i2c_temp)*100))%100;	// faren TC74 decimal places
	int MCP_int_F = (int)(celc_to_far(adc_temp)); 				// faren MCP truncated			
	int MCP_dec_F = ((int)(celc_to_far(adc_temp)*100))%100;		// faren MCP decimal places
	if(!changeUnitFlag)
	{
		printf("[Temp(TC74): %dC][Temp(MCP): %d.%.2dC][Converted Voltage: %d.%.2dV][DAC: %d.%.2dV]\n", 
		TC74_C, MCP_int_C, MCP_dec_C, CV_int, CV_dec, DAC_int, DAC_dec);
		//printf("[Temp(TC74): %d C][Temp(MCP): %.2f C][Converted Voltage: %.2f V][DAC: %.2f V]\n", 
		//i2c_temp, adc_temp, i2c_volt, dac_volt);
		return;
	}
		printf("[Temp(TC74): %d.%.2dF][Temp(MCP): %d.%.2dF][Converted Voltage: %d.%.2dV][DAC: %d.%.2dV]\n", 
		TC74_int_F, TC74_dec_F, MCP_int_F, MCP_dec_F, CV_int, CV_dec, DAC_int, DAC_dec);
		//printf("[Temp(TC74): %.2f F][Temp(MCP): %.2f F][Converted Voltage: %.2f V][DAC: %.2f V]\n", 
		//celc_to_far(i2c_temp), celc_to_far(adc_temp), i2c_volt, dac_volt);
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
		/*
		*	Use if want to implement listen by pressing Ctrl + Backspace
		*/
		/* 
		else if(r_char == 127) // sent by pressing Ctrl + Backspace
		{
			r_buffer[r_index] = 0; //terminate string with null character
			r_ready = 1;
			listen_state = 1;		// raise listen flag
			delete_flag = 1;		// raise delete flag
			UCSR0B ^= (1<<RXCIE0); // disable receive interrupt
		}
		*/
		else
		{
			if(r_char == 'p')
				changeUnitFlag ^= 1; // Toggle flag
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

ISR(TWI1_vect)
{
	if(TWSR1 & 0x50) //Data byte received, ack returned
	{
		i2c_temp = receive;
		i2c_volt = 2 + 0.0195*i2c_temp;
		tempVar = (int)(i2c_volt*4096.0/5.0);
		SPI_sendFirstHalf(tempVar);
		TWCR1 |= (1<<TWINT1);
	}
}

ISR(SPI0_STC_vect)
{
	if(!isSecondHalf)
	{
		isSecondHalf = 1;
		SPI_sendSecondHalf(tempVar);
	}
	else
	{
		isSecondHalf = 0;
		PORTB |= (1<<SPI_SS); // Pull SS high again
		PORTB &= ~(1<<SPI_LDAC); // Pull LDAC low
		ADMUX = 7;
		startConversion();
	}
}

ISR(ADC_vect)
{
	
	switch (ADMUX)
	{
		case 6:
			adc_temp = (ADC*5.0/1024.0 - 0.4)/(.0195);
			//writeMSG();
			writeMSGFlag = 1;
			break;
		case 7:
			dac_volt = ADC*(5.0/1024);
			PORTB |= (1<<SPI_LDAC); //reassert LDAC
			ADMUX = 6;
			startConversion();
			break;
		default:
			break;
	}
}


int main(void)
{
    /* Replace with your application code */
	i2c_init();
	uart_init();
	InitTimer0();
	setupADC();
	getstr();
	SPI_MasterInit();
	sei();

    while (1) 
    {
		if(counter >= 999)
		{
			counter = 0;
			i2c_transmit(slave_addr, 0x00, 8);
			i2c_receive(slave_addr, &receive, 8);
			TWCR1 |= (1<<TWIE1) | (1<<TWEN1) | (1<<TWSTA1);
			getstr();
		}
		if(writeMSGFlag)
		{
			writeMSGFlag = 0;
			writeMSG();	
		}
    }
}