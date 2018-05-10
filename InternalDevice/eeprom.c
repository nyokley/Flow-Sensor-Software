/*
 * eeprom.c
 *
 *  Created on: Apr 30, 2018
 *      Author: Nathaniel
 */

#include <msp430.h>
#include <stdint.h>
#include "eeprom.h"
#include "nfc_spi.h"

void EEPROM_SPI_instruction(uint8_t cmd) {


        EEPROM_SS_LOW;
        SPI_SendByte(cmd);
        EEPROM_SS_HIGH;

}

uint8_t EEPROM_ReadStatusRegister() {
    uint8_t result = 0;
    EEPROM_SS_LOW;
    SPI_SendByte(RDSR);
    result = SPI_ReceiveByte();
    EEPROM_SS_HIGH;
    return result;
}



