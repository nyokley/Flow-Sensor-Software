/*
 * eeprom.h
 *
 *  Created on: Apr 30, 2018
 *      Author: Nathaniel
 */

#ifndef EEPROM_H_
#define EEPROM_H_

#include <stdint.h>

#define EEPROM_SS_SETDIR       P5DIR |= BIT6
#define EEPROM_SS_HIGH         P5OUT |= BIT6
#define EEPROM_SS_LOW          P5OUT &= ~BIT6

//EEPROM Instructions
#define WRSR    0x01
#define WR      0x02
#define READ    0x03
#define FREAD   0x0B
#define WRDI    0x04
#define RDSR    0x05
#define WREN    0x06
#define PERS    0x42
#define CERS    0x60
#define PD      0xB9
#define UDPD    0x79
#define RES     0xAB

//functions
void EEPROM_SPI_instruction(uint8_t cmd);
uint8_t EEPROM_ReadStatusRegister();



#endif /* EEPROM_H_ */
