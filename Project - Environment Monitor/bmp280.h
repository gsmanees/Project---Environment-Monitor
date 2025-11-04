/*
 * bmp280.h
 *
 * Created: 03-11-2025 11.41.03 PM
 *  Author: anees
 */ 

#include <avr/io.h>

#ifndef BMP280_H_
#define BMP280_H_
uint8_t BMP280_readChipID(void);
uint32_t rawTemp(void);
void BMP280_exitSleep(void);
uint32_t finalTemp(uint32_t);




#endif /* BMP280_H_ */