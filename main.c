#include <avr/io.h>
#include <stdint.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#define MASK_7BIT 0b01111111

void init();

//uart
volatile char RXBuf[16]=			//tablica odebranych znaków
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
volatile uint8_t rxindex=0;			//indeks znaku do odebrania

enum tone {	C, Cis, D, Dis, E, F, Fis, G, Gis, A, Ais, B};
typedef enum tone tone;

volatile uint16_t nuty;

ISR (USART_RXC_vect) 
{
	RXBuf[rxindex] = UDR;
	rxindex++;
	if(rxindex>=1)
	{
		if(((RXBuf[0] &0b11110000) != 0b10000000 )&& ((RXBuf[0]&0b11110000) != 0b10010000))
		{
			rxindex=0;
			return;
		}
	}
	
	if(rxindex>=3)
	{
		if((RXBuf[0]&0b11110000) == 0b10000000)	//jeśli WYŁĄCZ_NUTĘ
		{
			nuty&=~(1<<(RXBuf[1]&MASK_7BIT));
		}
		if((RXBuf[0]&0b11110000) == 0b10010000)	//jeśli WŁĄCZ_NUTĘ
		{
			nuty|=(1<<(RXBuf[1]&MASK_7BIT));
		}
		rxindex=0;
	}	
}

int main(void)
{
	init();
	
	cli();
	
	while(1)
	{
		//PORTA=0xFF;
		//_delay_ms(1000);
		//PORTA=0x00;
		//_delay_ms(1000);
		PORTA=nuty&0xFF;
		PORTB=(nuty>>8);
	}
	
	return 0;
	
}

void init()
{
	uint8_t baud=31;												//BAUD MIDI dla F_OSC=16MHz
	
	DDRA=0xFF;
	DDRB=0xFF;
	
	PORTA=0x00;
	PORTB=0x00;
	
	UBRRH=(baud>>8);												//ustaw starszy bajt
	UBRRL=baud;														//ustaw młodszy bajt
	
	UCSRB = (1<<RXEN);												//odblokuj odbiór i nadawanie UART
	UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);						//8 bitów, 1 bit stopu, bez kontroli parzystości
	
	UCSRB |= (1<<RXCIE);											//odblokuj przerwanie po odebraniu bajtu
}
