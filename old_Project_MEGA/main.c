
#define F_CPU 16000000UL
#define BAUD 9600


#include <avr/io.h>
#include "keypad.h"
#include "lcd.h" // lcd header file made by Peter Fleury
#include <util/delay.h>
#include <util/setbaud.h>
#include <stdio.h>
#include <stdlib.h>

// SPI WITH ISR START

static void USART_init(uint16_t ubrr) //USART initiation p.206
{
	/* Set baud rate in the USART Baud Rate Registers (UBRR) *///datasheet p.222
	UBRR0H = (unsigned char) (ubrr >> 8); //datasheet p.206, ubbr value is 103
	UBRR0L = (unsigned char) ubrr; //datasheet p.206, , ubbr value is 103
	
	/* Enable receiver and transmitter on RX0 and TX0 */
	UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //RX complete interrupt enable//Transmitter enable // datasheet p.206, p.220
	
	/* Set frame format: 8 bit data, 2 stop bit */
	UCSR0C |= (1 << USBS0) | (3 << UCSZ00);//Stop bit selection at 2-bit// UCSZ bit setting at 8 bit//datasheet p.221 and p.222
	
}

static void USART_Transmit(unsigned char data, FILE *stream) //datasheet p.207
{
	/* Wait until the transmit buffer is empty*/
	while(!(UCSR0A & (1 << UDRE0))) //datasheet p.207, p. 219
	{
		;
	}
	
	/* Puts the data into a buffer, then sends/transmits the data */
	UDR0 = data;
}


static char USART_Receive(FILE *stream) //datasheet p.210, 219
{
	/* Wait until the transmit buffer is empty*/
	while(!(UCSR0A & (1 << RXC0)))
	{
		;
	}
	
	/* Get the received data from the buffer */
	return UDR0;
}

// Setup the stream functions for UART, read  https://appelsiini.net/2011/simple-usart-with-avr-libc/
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

// SPI WITH ISR END


// Initialize LCD and Keypad
void setup() {
	// Initialize LCD
	lcd_init(LCD_DISP_ON);
	lcd_clrscr();
	lcd_puts("Ready");
	_delay_ms(1000); // Delay to display initial message

	// Initialize Keypad
	KEYPAD_Init();
}

int
main(void) {
	setup();
	// SPI CONFIG START
	// initialize the UART with 9600 BAUD
	USART_init(MYUBRR);
	    
	// redirect the stdin and stdout to UART functions, read https://appelsiini.net/2011/simple-usart-with-avr-libc/
	stdout = &uart_output;
	stdin = &uart_input;
	
    /* Set SS, MOSI and SCK as output at pins 53 (PB0), 51 (PB2) and 52 (PB1) */
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2); //See datasheet p.192
	
    /* Enable SPI, master, set clock rate fosc/16 (1 MHz) */
	SPCR |= (1 << SPE) | (1 << MSTR) | (1 << SPR0); //See example in datasheet p.193 
    
	/* Create variable data array that will be sent and received */
    unsigned char spi_send_data[20] = "master to slave\n\r"; //to slave
    unsigned char spi_receive_data[20]; //from slave
	// SPI CONFIG END


	// LOOP START
	while (1) {

		// KEYPAD SCREEN CONFIG START
		// Read raw signal from keypad
		uint8_t key_signal = KEYPAD_GetKey();

		_delay_ms(300);

		// Check for valid key press
		if (key_signal != 0xFF) { // Assuming 0xFF means no key pressed
			char key_str[4];

			// Check if it's a numeric key (1 to 9) or special key (A, B, C, D, *, #)
			if (key_signal >= '1' && key_signal <= '9') {
				// Convert key to numeric value
				uint8_t key_value = key_signal - '0'; // Convert ASCII value to numeric value
				itoa(key_value, key_str, 10); // Convert numeric value to string

				} else {
				// For special keys, just display the character
				key_str[0] = key_signal;
				key_str[1] = '\0'; // Null terminate the string
			}

			// Display the key on the LCD
			lcd_clrscr();
			lcd_puts("Key Pressed:");
			lcd_gotoxy(0, 1);
			lcd_puts(key_str);
		}
		// KEYPAD SCREEN CONFIG END

		// SPI SIGNAL LOOP START
		        /* send byte to slave and receive a byte from slave by setting the SS to low */
						PORTB &= ~(1 << PB0); // SS low, enable the slave device
            
						for(int8_t spi_data_index = 0; spi_data_index < sizeof(spi_send_data); spi_data_index++) //to read data from slave
						{
								SPDR = spi_send_data[spi_data_index]; // Use SPI data register (SPDR) to send byte of data
					
					_delay_us(5);
					/* wait until the transmission is complete */
								while(!(SPSR & (1 << SPIF))) //see example datasheet p.193, and datasheet at p.198.
								{
										;
								}
								_delay_us(5);
								spi_receive_data[spi_data_index-1] = SPDR; // receive byte from the SPI data register
						}
								
						PORTB |= (1 << PB0); // SS HIGH to disable the slave device
						
				/* Print the received data*/
						printf(spi_receive_data);
						_delay_ms(2000);
		// SPI SIGNAL LOOP END

	}


	return 0;
}
// LOOP END
