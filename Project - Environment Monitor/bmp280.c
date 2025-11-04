/*
 * bmp280.c
 *
 * Created: 03-11-2025 11.41.18 PM
 *  Author: anees
 */ 
#define F_CPU 16000000UL
#include "bmp280.h"
#include<util/delay.h>
#include<avr/io.h>
#include "uart.h"

uint8_t BMP280_readChipID(void)
{
	// ===== READ CHIP ID =====
	uint8_t chip_id;
	PORTB &= ~(1 << PB2); // CSB low
	SPDR = 0xD0|0x80; // read operation --> MSB = 1
	while (!(SPSR & (1 << SPIF)));
	(void)SPDR;
	
	SPDR = 0x00;
	while (!(SPSR & (1 << SPIF)));
	chip_id = SPDR;
	PORTB |= (1 << PB2); // CSB high
	_delay_ms(10);
	return chip_id;
}

void BMP280_exitSleep(void)
{
	// ===== Configure BMP280 ctrl_meas (0xF4) =====
	PORTB &= ~(1 << PB2);           // CS low
	SPDR = 0xF4 & 0x7F;             // Write address (bit7=0)
	while (!(SPSR & (1 << SPIF)));
	(void)SPDR;

	SPDR = 0x27;                    // Write 0x27 ? normal mode, oversampling x1
	while (!(SPSR & (1 << SPIF)));
	(void)SPDR;
	PORTB |= (1 << PB2);            // CS high
	_delay_ms(100);


	// config bmp280
	PORTB &= ~(1 << PB2);           // CS low
	SPDR = 0xF5 & 0x7F;             // Write address (bit7=0)
	while (!(SPSR & (1 << SPIF)));
	(void)SPDR;

	SPDR = 0xA0;                    // Write 0x27 ? normal mode, oversampling x1
	while (!(SPSR & (1 << SPIF)));
	(void)SPDR;
	PORTB |= (1 << PB2);            // CS high
	_delay_ms(100);
}

uint32_t rawTemp(void)
{
	uint32_t temp;
	uint8_t msb, lsb, xlsb;
	// ===== Read Temperature Registers (0xFA, 0xFB, 0xFC) =====
	PORTB &= ~(1 << PB2);           // CS low

	SPDR = 0xFA | 0x80;             // Read address (bit7=1)
	while (!(SPSR & (1 << SPIF)));
	(void)SPDR;

	// Read MSB
	SPDR = 0x00;
	while (!(SPSR & (1 << SPIF)));
	msb = SPDR;

	// Read LSB
	SPDR = 0x00;
	while (!(SPSR & (1 << SPIF)));
	lsb = SPDR;

	// Read XLSB
	SPDR = 0x00;
	while (!(SPSR & (1 << SPIF)));
	xlsb = SPDR;

	PORTB |= (1 << PB2); // CS high

	// ===== Combine 20-bit Temperature Data =====
	temp = ((uint32_t)msb << 12) | ((uint32_t)lsb << 4) | (xlsb >> 4);
	return temp;
}

uint32_t finalTemp(uint32_t adc_temp)
{
	uint8_t lsb, msb;
	uint16_t dig_T1;
	uint16_t dig_T2;
	int dig_T3;
	
	
	
	float var1, var2, temperature;
	int32_t t_fine;
	//uint32_t adc_temp;
	
	UART_Init();
	DDRB |= (1 << PB3) | (1 << PB5) | (1 << PB2); // MOSI, SCK, SS outputs
	DDRB &= ~(1 << PB4);                           // MISO input
	PORTB |= (1 << PB2);                           // SS high initially
	DDRD |= (1 << PD3);
	SPCR = (1 << SPE) | (1 << MSTR) | (1 << SPR0); // Enable SPI Master, fosc/16
	_delay_ms(100); // allow BMP280 to power up
	PORTB &= ~(1 << PB2);           // SS low to start transaction
	_delay_us(1);
	SPDR = 0x88 | 0x80;             // send register address with read bit
	while (!(SPSR & (1 << SPIF)));  // wait for transfer complete
	(void)SPDR;                     // discard received byte (dummy)
	SPDR = 0x00;                    // clock out LSB
	while (!(SPSR & (1 << SPIF)));
	lsb = SPDR;                     // read LSB (0x88)
	SPDR = 0x00;                    // clock out MSB
	while (!(SPSR & (1 << SPIF)));
	msb = SPDR;                     // read MSB (0x89)
	_delay_us(1);
	PORTB |= (1 << PB2);            // SS high to end transaction
	dig_T1 = (uint16_t)msb << 8 | lsb;
	
	
	// dig _2
	
	PORTB &= ~(1 << PB2);
	SPDR = 0x8A | 0x80;             // send register address with read bit
	while (!(SPSR & (1 << SPIF)));  // wait for transfer complete
	(void)SPDR;                     // discard received byte (dummy)
	SPDR = 0x00;                    // clock out LSB
	while (!(SPSR & (1 << SPIF)));
	lsb = SPDR;                     // read LSB (0x88)
	SPDR = 0x00;                    // clock out MSB
	while (!(SPSR & (1 << SPIF)));
	msb = SPDR;
	_delay_us(1);
	PORTB |= (1 << PB2);
	
	dig_T2 = (uint16_t)msb << 8 | lsb;
	
	
	//dig_3
	PORTB &= ~(1 << PB2);
	SPDR = 0x8C| 0x80;             // send register address with read bit
	while (!(SPSR & (1 << SPIF)));  // wait for transfer complete
	(void)SPDR;                     // discard received byte (dummy)
	SPDR = 0x00;                    // clock out LSB
	while (!(SPSR & (1 << SPIF)));
	lsb = SPDR;                     // read LSB (0x88)
	SPDR = 0x00;                    // clock out MSB
	while (!(SPSR & (1 << SPIF)));
	msb = SPDR;
	_delay_us(1);
	PORTB |= (1 << PB2);
	
	dig_T3 = (uint16_t)msb << 8 | lsb;
	var1 = (((float)adc_temp / 16384.0) - ((float)dig_T1 / 1024.0)) * (float)dig_T2;
	var2 = ((((float)adc_temp / 131072.0) - ((float)dig_T1 / 8192.0)) *
	(((float)adc_temp / 131072.0) - ((float)dig_T1 / 8192.0))) * (float)dig_T3;
	t_fine = (int32_t)(var1 + var2);
	temperature = t_fine / 5120.0;   // in °C
	/*while (1) {
		UART_TxString("\nt_fine: ");
		UART_TxNumber(t_fine);
		_delay_ms(1000);           
		UART_TxString("\ntemperature: ");
		UART_TxNumber(temperature);
		_delay_ms(1000);
		
	}*/
	return temperature;
}