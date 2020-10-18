#ifndef  F_CPU
#define F_CPU 16000000UL
#endif

#include <avr/io.h>
#include <util/twi.h>

#include "i2c_master.h"

#define F_SCL 100000UL // SCL frequency
#define Prescaler 1
#define TWBR_val ((((F_CPU / F_SCL) / Prescaler) - 16 ) / 2)

void i2c_init(void)
{
	TWBR1 = (uint8_t)TWBR_val;
}

uint8_t i2c_start(uint8_t address)
{
	// reset TWI control register
	TWCR1 = 0;
	// transmit START condition
	TWCR1 = (1<<TWINT1) | (1<<TWSTA1) | (1<<TWEN1); 
	// wait for end of transmission
	while( !(TWCR1 & (1<<TWINT1)) );
	
	// check if the start condition was successfully transmitted
	if((TWSR1 & 0xF8) != TW_START){ return 1; }
	
	// load slave address into data register
	TWDR1 = address;
	// start transmission of address
	TWCR1 = (1<<TWINT1) | (1<<TWEN1);
	// wait for end of transmission
	while( !(TWCR1 & (1<<TWINT1)) );
/*	
	// check if the device has acknowledged the READ / WRITE mode
	uint8_t twst = TW_STATUS & 0xF8;
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
*/
	return 0;
}

uint8_t i2c_write(uint8_t data)
{
	// load data into data register
	TWDR1 = data;
	// start transmission of data
	TWCR1 = (1<<TWINT1) | (1<<TWEN1);
	// wait for end of transmission
	while( !(TWCR1 & (1<<TWINT1)) );
	
	if( (TWSR1 & 0xF8) != TW_MT_DATA_ACK ){ return 1; }
	
	return 0;
}

uint8_t i2c_read_ack(void)
{
	
	// start TWI module and acknowledge data after reception
	TWCR1 = (1<<TWINT1) | (1<<TWEN1) | (1<<TWEA1);
	// wait for end of transmission
	while( !(TWCR1 & (1<<TWINT1)) );
	// return received data from TWDR
	return TWDR1;
}

uint8_t i2c_read_nack(void)
{
	
	// start receiving without acknowledging reception
	TWCR1 = (1<<TWINT1) | (1<<TWEN1);
	// wait for end of transmission
	while( !(TWCR1 & (1<<TWINT1)) );
	// return received data from TWDR
	return TWDR1;
}

uint8_t i2c_transmit(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c_start(address | I2C_WRITE)) return 1;
	
	for (uint16_t i = 0; i < length; i++)
	{
		if (i2c_write(data[i])) return 1;
	}
	
	i2c_stop();
	
	return 0;
}

uint8_t i2c_receive(uint8_t address, uint8_t* data, uint16_t length)
{
	if (i2c_start(address | I2C_READ)) return 1;
	
	for (uint16_t i = 0; i < (length-1); i++)
	{
		data[i] = i2c_read_ack();
	}
	data[(length-1)] = i2c_read_nack();
	
	i2c_stop();
	
	return 0;
}

void i2c_stop(void)
{
	// transmit STOP condition
	TWCR1 = (1<<TWINT1) | (1<<TWEN1) | (1<<TWSTO1);
}