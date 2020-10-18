/*
 * ECE3411LabTest3.c
 *
 * Created: 03-Mar-20 18:21:35
 * Author : Siddarth Suresh
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_ON PORTB |= (1<<PORTB5)
#define LED_OFF PORTB &= ~(1<<PORTB5)

volatile double dutyCycle = 0;
volatile int counter10ms = 0;
volatile int counter100ms = 0;
volatile unsigned char mode2 = 0;
volatile uint16_t led_selection = 0;
volatile int sum_adc = 0;
volatile int count_adc_flag = 0;

//Initialize ADC
void setupADC(void)
{
	ADMUX |= (1<<MUX2) | (1<<MUX1); // ADC6
	ADCSRA |= (1<<ADEN) | (1<<ADIE) | (1<<ADPS2) | // ADC enable, interrupt enable
	(1<<ADPS1); // prescaler = 64
}

//function for starting a conversion
void startConversion(void)
{
	ADCSRA |= (1<<ADSC);
}


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
	
	//1.5 kHz clock
	TCCR1A |= (1<<WGM10) | (1<<WGM11);
	TCCR1B |= (1<<WGM12) | (1<<WGM13);
	OCR1A = 1332; // ~1.5kHz // PICK SUCH THAT OCR1A > 1024
	OCR1B = 0;
	TIMSK1 |= (1<<OCIE1B) | (1<<OCIE1A); //Enable output compare B, A, match interrupt
	TCCR1B |= (1<<CS11); // prescaler = 8
}

void InitSW1(void)
{
	PCMSK0 |= (1<<PCINT7); //PCINT7 is SW1 (PB7)
	PCICR |= (1<<PCIE0); //PCINT7 is in PCIE0
}

//calculate how to display voltage level on port d LEDs
void bar_display(void)
{
	int i;
	led_selection = 1;
	for(i = 0; i<(int)(9*dutyCycle); i++) //multiply by 9 to get 8 from truncation
		led_selection*=2;
	led_selection--;
}

ISR(ADC_vect)
{
	sum_adc += ADC;
}

ISR(TIMER1_COMPB_vect)
{
	LED_OFF;
	if(mode2)
		PORTD = 0b00000000;
	else
		PORTD = led_selection;
	if(dutyCycle>0)
		OCR1B = (int)((dutyCycle)*(OCR1A+1));
	else
		OCR1B = 0;
}

ISR(TIMER1_COMPA_vect)
{
	LED_ON;
	PORTD = led_selection;
}

ISR(PCINT0_vect) //ON BOARD SWITCH (SW1)
{
	if(!(PINB & (1<<PINB7))) //Falling Edge
	{
		mode2 = !mode2;
	}
}

ISR(TIMER0_COMPA_vect)
{
	counter10ms++;	
	if(counter100ms == 100)
	{
		dutyCycle = (sum_adc/10.0)/1024.0;
		bar_display();
		sum_adc = 0;
		counter100ms = 0;
	}
	if(counter10ms == 10)
	{
		counter10ms = 0;
		counter100ms+=10;
		startConversion();
	}



}



int main(void)
{
    /* Replace with your application code */
	DDRB |= (1<<DDB5); //ON BOARD LED
	DDRB &= ~(1<<DDB7); // ON BOARD LED
	DDRD = 0b11111111; // Set port D to output
	InitTimer1();
	InitTimer0();
	InitSW1();
	setupADC();
	sei();
    while (1) 
    {
    }
}

