/*SPI_ISR_UNO_SLAVE

 * ATmega328p is slave */

#define F_CPU 16000000UL //clock speed
#define FOSC 16000000UL // system clock frequency
#define BAUD 9600 //Baud rate
#define MYUBRR (FOSC/16/BAUD-1) //Define the UBBR value using the equation in datasheet p.203 Table 22-1

#include <avr/io.h>
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>

#include <avr/interrupt.h>
#include <stdbool.h>

static void USART_init(uint16_t ubrr)
{
	/* Set baud rate in the USART Baud Rate Registers (UBRR) */
	UBRR0H = (unsigned char) (ubrr >> 8); //see example datasheet p. 176
	UBRR0L = (unsigned char) ubrr;
	
	/* Enable receiver and transmitter on RX0 and TX0 */
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); // see example datasheet p. 176
	
	/* Set frame format: 8 bit data, 2 stop bit */
	UCSR0C |= (1 << USBS0) | (3 << UCSZ00); //see example datasheet p. 176
	
}

static void USART_Transmit(unsigned char data, FILE *stream)
{
	/* Wait until the transmit buffer is empty*/
	while(!(UCSR0A & (1 << UDRE0))) //see example datasheet p.177
	{
		;
	}
	
	/* Puts the data into a buffer, then sends/transmits the data */
	UDR0 = data;
}

static char
USART_Receive(FILE *stream)
{
	/* Wait until the transmit buffer is empty*/
	while(!(UCSR0A & (1 << RXC0)))  //see example datasheet p. 180
	{
		;
	}
	
	/* Get the received data from the buffer */
	return UDR0;
}

// Setup the stream functions for UART, read https://appelsiini.net/2011/simple-usart-with-avr-libc/
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

// variables for storing data used in SPI communication
unsigned char spi_send_data[20]= "Slave to master\n\r";
unsigned char spi_receive_data[20];
volatile bool transfer_complete = false;
volatile int8_t spi_index = false;
volatile int8_t spi_receive_index = false;


ISR
(SPI_STC_vect)
{
	unsigned char spi_interrupt_byte = SPDR;
	unsigned char transfer_end_check = '\r';
	SPDR = spi_send_data[spi_index]; // from zero
	spi_index++; // to ++
	
	if (spi_receive_index < sizeof(spi_receive_data)){//store interupt byte in receive data
		spi_receive_data[spi_receive_index++] = spi_interrupt_byte;
		
		//check if the received byte is '\r'
		if (spi_interrupt_byte == transfer_end_check){
			transfer_complete = true;
		}
	}
	
}

int
main(void)
{
	// initialize the UART with 9600 BAUD
	USART_init(MYUBRR);
	
	// redirect the stdin and stdout to UART functions, read https://appelsiini.net/2011/simple-usart-with-avr-libc/
	stdout = &uart_output;
	stdin = &uart_input;
	
	/* set MISO as output, pin 12 (PB4)*/
	DDRB  |= (1 << PB4); // see datasheet p.162
	
	
	/* set SPI enable (SPE) and and interrupt enable (SPIE) */
	SPCR  |= (1 << SPE)| (1 << SPIE);
	SPDR = 0;
	
	//enable interupt
	sei();
	
	/* send message to master and receive message from master */
	while (1)
	    {
		if (transfer_complete == true){
			printf(spi_receive_data);
			transfer_complete = false;
			spi_index = false;
			spi_receive_index = false;
		}
		}
		
 return 0;
}
