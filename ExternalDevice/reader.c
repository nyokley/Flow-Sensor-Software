/*
 * reader.c
 *
 *  Created on: Apr 25, 2018
 *      Author: Nathaniel
 */

#include "reader.h"

void Reader_init(Reader* reader) {
   reader->currState = IDLE;
   reader->nextState = IDLE;
   reader->command = NONE;
   reader->confirm1Incrementer = 0;
   reader->confirm2Incrementer = 0;
   reader->delayIncrementer = 0;
   reader->confirm1Max = 1000;
   reader->confirm2Max = 30;
   reader->delayMax = 500;
}


