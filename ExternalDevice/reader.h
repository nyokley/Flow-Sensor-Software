/*
 * reader.h
 *
 *  Created on: Apr 25, 2018
 *      Author: Nathaniel
 */

#ifndef READER_H_
#define READER_H_

#include "stdint.h"
#include "stdbool.h"
typedef enum
{
    NONE = 0x00,
    START_POLL = 0x01,
    STOP_POLL = 0x02,
    READ_DATA = 0x03,
    READ_DSTATUS = 0x04,
    READ_BSTATUS = 0x05,
    CONFIRM = 0x06,

}tReaderImplantCommands;



typedef enum {
    IDLE = 0x00,
    SEND_COMMAND = 0x01,
    SEND_CONFIRM = 0x02,
    WAIT_RESPONSE = 0x03,
    WAIT_CONFIRM = 0x04,
    PRINT_SERIAL = 0x05,
    DELAY = 0x06
}tReaderState;

typedef struct t_config {
    double maxSamples;
}tConfig;

typedef struct t_reader{
    tReaderState currState;
    tReaderState nextState;
    tReaderImplantCommands command;
    bool receivedTx;
    bool confirmed;
    double confirm1Incrementer;
    double confirm2Incrementer;
    double delayIncrementer;
    double confirm1Max;
    double confirm2Max;
    double delayMax;
    char serialString[200];
    uint8_t transmitString[200];
    int transmitStringLength;
    char receivedString[200];
    int receivedStringLength;

} Reader;

Reader reader;

void Reader_init(Reader* reader);

#endif /* READER_H_ */
