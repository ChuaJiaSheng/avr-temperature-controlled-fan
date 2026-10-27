#include <avr/io.h>
#include <avr/interrupt.h>
#define TurnOnTemp 260
#define TurnOffemp 250 
unsigned int dpNum,temperature,tempInADC; 
unsigned char whichDigit;

//I/O visualisation
void ioInit(){
	DDRB = 0x07; //SW3,2,1 & Mux3,2,1
	DDRC = 0x0F; //ADC4(LDR) & ADC5(TEMP) and 3-Digit display
	DDRD = 0xFF; //LED0-7
	PORTB = 0x38; //SW3,2,1
	PORTC = 0;
	PORTD = 0;
}

//3-Digit Display
void displayNow(unsigned int dpNum){
	PORTB = 0x38; // Turn off all mux transistors.
	PORTC = 0;// Clear the 4 bit BCD output ports to 0.
	switch(whichDigit){
		case 1: PORTC = (PORTC & 0xF0) | (dpNum/100);// Display 100th.
			PORTB = (PORTB & 0xF8) | 0x01; // Turn on mux 1 transistor.
		break;
		case 2: PORTC = (PORTC & 0xF0) | (dpNum%100)/10;// Display 10th.
			PORTB = (PORTB & 0xF8) | 0x02; // Turn on mux 2 transistor.
		break;
		case 3: PORTC = (PORTC & 0xF0) | dpNum%10;// Display 1th.
			PORTB = (PORTB & 0xF8) | 0x04; // Turn on mux 3 transistor.
		break;
	}
	whichDigit++;
	if (whichDigit>3) whichDigit=1; // Set back to 1st digit.
}

void interruptInit(){
	TCNT0 = 0xB2; // Load timer for 5 ms overflow.
	TCCR0B = (TCCR0B & 0xF8) | 0x05; // Run timer 0 with prescaler 1024.
	TIMSK0 = TIMSK0 | 0x01; // Enable timer 0 interrupt.
	TIFR0 |= 0x01; // OV Flag for Timer0
}

ISR(TIMER0_OVF_vect){
	TCNT0 = 0xB2; // Load timer for 5 ms overflow.
	TIFR0 |= 0x01;// Reload timer.
	displayNow(dpNum);
}

unsigned int readADC(){
	ADMUX = (ADMUX & 0xF0) | 0x05; //Select PC5 as input.
	ADCSRA |= 0x40; //ADSC [Start ADC]
	while ((ADCSRA & 0x10)==0); //ADIF [Wait until conversion finish]
	ADCSRA |= 0x10; //ADIF [Reset interrupt flag]
	return ADCW;
}

void initADC(){
	DIDR0 = 0x20; //Disable ADC5(TEMP)
	DDRC &= ~(0x20);// Make ADC5(TEMP) an input
	ADCSRA |= 0x97; // Enable ADC, clear ADC conversion
	ADMUX = 0xC0; // Vref=1.1V
}

//Convert ADC Reading to Temperature
unsigned int convert(unsigned int k){
	unsigned long c;
	c = (long)k * 1075; //VREF = 1.1V
	temperature = c/1000;
	return(temperature);
}

//Test Multiplexing Display
int main(){//2.4 Test Your Multiplexing Display
	ioInit();
	interruptInit();
	sei();// Global Interrupt enable
	initADC();
	DDRD |= 0x80;
	while(1){
		tempInADC = readADC();
		dpNum = convert(tempInADC);
		if (dpNum >= TurnOnTemp) PORTD |= 0x80; //On Fan
		if (dpNum <= TurnOffemp) PORTD &= ~0x80; //Off Fan
	}
}