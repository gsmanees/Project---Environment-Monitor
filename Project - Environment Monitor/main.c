/*
 * Project - Environment Monitor.c
 *
 * Created: 04-11-2025 6.53.05 AM
 * Author : anees
 */ 


#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include "uart.h"
#include "spi.h"
#include "bmp280.h"
#include "lcd.h"



int main(void)
{
	uint8_t Chip_ID;
	uint32_t rawTemp_value;
	uint32_t FinalTempVal;
	
	UART_Init();

	SPI_init();
	
	lcd_init();

	

	while (1)
	{
		Chip_ID = BMP280_readChipID();
		BMP280_exitSleep();
		rawTemp_value = rawTemp();
		FinalTempVal = finalTemp(rawTemp_value);
		UART_TxString("\nChip ID: ");
		UART_TxHex(Chip_ID);
		//UART_TxChar('\n');
		
		UART_TxString("Raw Temp: ");
		UART_TxNumber(rawTemp_value);
		BMP280_exitSleep();
		UART_TxString("\nFinal Temp: ");
		UART_TxNumber(FinalTempVal);
		_delay_ms(1000);
		UART_TxChar('\n');
		
		lcd_set_cursor(0, 0);
		lcd_print("Temperature: ");
		lcd_set_cursor(1, 0);
		lcd_print_uint16(FinalTempVal);
		
		
	}
}
