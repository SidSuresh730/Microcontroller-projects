#define F_CPU 16000000UL //Set the clock frequency
#include <avr/io.h> //call IO library
#include <avr/interrupt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include "uart.h"
#include "i2c_master.h"

#define r_buffer_size 50
#define SPI_DDR DDRB
#define SPI_SS 2
#define SPI_MOSI 3
#define SPI_MISO 4
#define SPI_SCK 5

char r_buffer[r_buffer_size];
int r_index = 0;
volatile char r_ready;

volatile int temp_mode = 0;
volatile int counter = 1000;
volatile int read_temp = 0;
volatile uint8_t* a;
volatile int a_1 = 0;

volatile int f_i = 0;
volatile int f_d = 0;

volatile double i2c_voltage = 0;
volatile int Y_i = 0;
volatile int Y_d = 0;

volatile int dac_i = 0;
volatile int dac_d = 0;

//int print_mode = 0;

FILE UART_Stream = FDEV_SETUP_STREAM(uart_putchar, uart_getchar, _FDEV_SETUP_RW); //macro for allocating a valid UART stream buffer.


void clear_buffer(void)
{
	for(int i = 0; i < r_buffer_size; i++)
	{
		r_buffer[i] = 0;
	}
}

void getstr(void)
{
	r_ready = 0;
	r_index = 0;
	UCSR0B |= (1<<RXCIE0);
}

ISR(TWI1_vect)
{
	i2c_voltage = (a_1)*0.0195+2;
	Y_i = i2c_voltage;
	Y_d = (i2c_voltage - Y_i)*100;
	int j = i2c_voltage/5.0*1024;
	int adc_12 = (j<<2);
	int low_half = adc_12 & 0xFF;
	int upper_half = (0b01110000 | (adc_12 >> 8));
	PORTB &= ~(1 << SPI_SS);
	PORTB |= (1<<PINB0);
	SPI_MasterTransmit(upper_half);
	SPI_MasterTransmit(low_half);
	PORTB |= (1 << SPI_SS);
	PORTB &= ~(1 << PINB0);
	ADMUX = 7;
	ADCSRA |= (1 << ADSC);
	dac_i = ADC*5.0/1024.0;
	dac_d = ((ADC*5.0/1024.0) - dac_i)*100;
	ADMUX = 6;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1<<ADSC));
	f_i = (((ADC/1024.0)*5.0)-0.4)/0.0195;
	f_d = (((((ADC/1024.0)*5.0)-0.4)/0.0195)-f_i)*100;
	TWCR1 = (1<<TWINT1);
}

ISR(TIMER0_COMPA_vect)
{
	if(counter <= 0)
	{
		read_temp = 1;
	}
	else
	{
		counter--;
	}
}

ISR(USART0_RX_vect)
{
	char r_char = UDR0;
	UDR0 = r_char;
	if(r_char != '\r')
	{
		if(r_char == '\b')
		{
			putchar(32);
			putchar('\b');
			--r_index;
		}
		else
		{
			r_buffer[r_index] = r_char;
			if (r_index < r_buffer_size-1)
			{
				r_index++;
			}
			else
			{
				r_index = 0;
			}
		}
	}
	else
	{
		putchar('\n');
		r_buffer[r_index] = 0;
		r_ready = 1;
		UCSR0B ^= (1<<RXCIE0);
	}
}


void ms_timer(void)
{
	//Timer 0, set to 1ms
	TCCR0A |= (1<<WGM01);
	OCR0A = 249;
	TIMSK0 = (1<<OCIE0A);
	TCCR0B = 3;
}

void adc_init(void)
{
	ADCSRA |= (1<<ADEN) | (1<<ADPS0) | (1<<ADPS1) | (1<<ADPS2);
}

void SPI_MasterInit(void)
{
	SPI_DDR = (1<<SPI_MOSI) | (1<<SPI_SCK) | (1<<SPI_SS) | (1<<PINB0);
	SPCR0 = (1<<SPE) | (1<<MSTR) | (1<<SPR0);
}

void SPI_MasterTransmit(char cData)
{
	SPDR0 = cData;
	while(!(SPSR0 & (1<<SPIF)));
}

int main(void)
{
	/* Replace with your application code */
	ms_timer();
	uart_init();
	i2c_init();
	adc_init();
	SPI_MasterInit();
	
	sei();
	clear_buffer();
	getstr();
	while (1)
	{
		if(r_ready == 1)
		{
			char a[2];
			a[0] = r_buffer[0];
			a[1] = '\0';
			if(strcmp(a, "p") == 0)
			{
				temp_mode = !temp_mode;
			}
			getstr();
		}
		
		if(read_temp == 1)
		{
			i2c_transmit(0x90, 0x00, 1);
			i2c_receive(0x90, a, 1);
			a_1 = *a;
			TWCR1 = (1<<TWIE1) | (1<<TWEN1) | (1<<TWSTA1);
			if(temp_mode == 0)
			{
				printf("[Temp(TC74): %dC][Temp(MCP): %d.%dF][Converted Voltage: %d.%dV][DAC: %d.%dV]\n", a_1, f_i, f_d, Y_i, Y_d, dac_i, dac_d);
			}
			else if(temp_mode == 1)
			{
				int b = a_1*9.0/5.0 + 32.0;
				int c = ((a_1*9.0/5.0 + 32.0)-b)*100;

				printf("[Temp(TC74): %d.%dF][Temp(MCP): %d.%dF][Converted Voltage: %d.%dV][DAC: %d.%dV]\n", b, c, f_i, f_d, Y_i, Y_d, dac_i, dac_d);
			}
			read_temp = 0;
			counter = 1000;
		}

	}
	
	return 0;
}

