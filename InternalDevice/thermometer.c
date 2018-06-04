/*
 * thermometer.c
 *
 *  Created on: Mar 27, 2018
 *      Author: Nathaniel
 */

#include "thermometer.h"
#include "nfc_spi.h"

//Temperature Sensor initialization
//Returns 1 if success, 0 if fail
_Bool adt7320_init() {
    _Bool success = 1;
    uint8_t check8;
    uint16_t check16;

    ADT_SS_SETDIR;

    ADT_SS_LOW;
    //reset serial interface - initialize all internal circuitry - 32 1's
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    SPI_SendByte(0xFF);
    ADT_SS_HIGH;


    //check device ID
    ADT_SPI_read8(&check8, 0x03);
    if(check8 != 0xC3) success = 0;

    //check config register
    //SPI_read8(&check8, 0x01);
    //if(check8 != 0x00) success = 0;

    //check T-crit
    ADT_SPI_read16(&check16, 0x04);
    if(check16 != 0x4980) success = 0;

    //check T-high
    ADT_SPI_read16(&check16, 0x06);
    if(check16 != 0x2000) success = 0;

    //check T-low
    ADT_SPI_read16(&check16, 0x07);
    if(check16 != 0x0500) success = 0;

    //set config register
    ADT_SPI_write8(0x80, 0x01);     //16-bit resolution, all else default

    return success;
}

//returns 16-bit temperature value
uint16_t adt7320_readTemp() {
    uint16_t val = 0;
    ADT_SPI_read16(&val, 0x02);
    return val;
}


//writes 8 bit 'data' to 8 bit 'reg' according to ADT specs
void ADT_SPI_write8(uint8_t data, uint8_t reg)
{
    volatile int8_t x;

    ADT_SS_LOW;                       // Start SPI Mode

    //parse reg for ADT
    reg = (0x38 & (reg << 3));      // register address - place addr bits to b5, b4, b3


    SPI_SendByte(reg);
    SPI_SendByte(data);

    ADT_SS_HIGH;                      // Stop SPI Mode

}

//read 8 bit register from ADT
void ADT_SPI_read8(uint8_t *pbuf, uint8_t reg)
{
    volatile uint8_t x;

    ADT_SS_LOW;                       // Start SPI Mode

    //parse reg for ADT - include read bit (b6)
    reg = (0x38 & (reg << 3));      // register address - place addr bits to b5, b4, b3
    reg = (0x40 | reg);             // add read bit


    SPI_SendByte(reg);              // Previous data to TX, RX

    *pbuf = SPI_ReceiveByte();

    ADT_SS_HIGH;                      // Stop SPI Mode

}

//write to 16 bit register -- HAVE NOT TESTED YET!!!
void ADT_SPI_write16(uint16_t data, uint8_t reg)
{
    volatile int8_t x;

    ADT_SS_LOW;                       // Start SPI Mode

    // Address/Command Word Bit Distribution
    // address, write, single (fist 3 bits = 0)
    reg = (0x38 & (reg << 3));      // register address - place addr bits to b5, b4, b3


    SPI_SendByte(reg);
    SPI_SendByte((uint8_t) (data >> 8));        //send higher byte
    SPI_SendByte((uint8_t) ((data*0x10) >> 8)); //send lower byte

    ADT_SS_HIGH;                      // Stop SPI Mode

}

//read from 16 bit register
void ADT_SPI_read16(uint16_t *pbuf, uint8_t reg)
{
    volatile uint8_t x;

    ADT_SS_LOW;                       // Start SPI Mode

    // Address/Command Word Bit Distribution
    // address, write, single (fist 3 bits = 0)
    reg = (0x38 & (reg << 3));      // register address - place addr bits to b5, b4, b3
    reg = (0x40 | reg);             // add read bit (b6)


    SPI_SendByte(reg);              // Previous data to TX, RX

    *pbuf = SPI_ReceiveByte();
    *pbuf = (*pbuf << 8) | SPI_ReceiveByte();
    ADT_SS_HIGH;                      // Stop SPI Mode

}






