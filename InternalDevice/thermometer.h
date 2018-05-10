/*
 * thermometer.h
 *
 *  Created on: Mar 27, 2018
 *      Author: Nathaniel
 */

#ifndef THERMOMETER_H_
#define THERMOMETER_H_

#include <stdint.h>

#define ADT_SS_SETDIR       P3DIR |= BIT0
#define ADT_SS_HIGH         P3OUT |= BIT0
#define ADT_SS_LOW          P3OUT &= ~BIT0

_Bool adt7320_init();
uint16_t adt7320_readTemp();
void ADT_SPI_directCommand(uint8_t command);
void ADT_SPI_write8(uint8_t data, uint8_t reg);
void ADT_SPI_read8(uint8_t *pbuf, uint8_t reg);
void ADT_SPI_write16(uint16_t data, uint8_t reg);
void ADT_SPI_read16(uint16_t *pbuf, uint8_t reg);




#endif /* THERMOMETER_H_ */
