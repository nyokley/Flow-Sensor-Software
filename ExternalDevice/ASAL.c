/*
 * HBAL.c
 *
 *  Created on: Apr 25, 2018
 *      Author: Nathaniel
 */

#include <ASAL.h>
#include "stdint.h"
#include "stdlib.h"
#include "snep.h"

void transmitMessage(uint8_t message_i[], int length) {

    int i;
    //int messageLength = 0;
    uint8_t  message_o[40];
    uint32_t ui32NdefLength;
    uint32_t ui32PacketIndex;
    uint8_t ui8FragmentLength;

    //while(message_i[messageLength] != 0x00) {
    //    messageLength++;
    //}
    //messageLength += 2;
    //message_o = (uint8_t*)calloc(length+7, sizeof(uint8_t));

        //if(g_ui8SerialBuffer[3] & 0x10)
        //{
        message_o[0] = 0xD1;        //D1
        message_o[1] = 0x01;
        message_o[2] = length + 3;
        message_o[3] = 84;
        message_o[4] = 2;
        message_o[5] = 101;
        message_o[6] = 110;
        for(i = 7; i < length+7; ++i) {
            message_o[i] = message_i[i-7];
        }
        //message_o[messageLength+7-2] = P2P_ENDCHAR;
        //message_o[messageLength+7-1] = 0x00;

        ui32NdefLength = (uint32_t) length + 7;       //text length + 7
        ui32PacketIndex = 0;
        ui8FragmentLength = SNEP_getTxBufferStatus();

        if((uint32_t) ui8FragmentLength > ui32NdefLength)
        {
            ui8FragmentLength = (uint8_t) ui32NdefLength;
        }

        SNEP_setupPacket(&message_o[0], ui32NdefLength,ui8FragmentLength);
}


