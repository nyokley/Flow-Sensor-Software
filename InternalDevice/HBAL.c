/*
 * HBAL.c
 *
 *  Created on: Mar 20, 2018
 *      Author: Nathaniel
 */

#include "HBAL.h"
#include "snep.h"
#include "sensor.h"
#include <stdlib.h>

extern uint16_t data[128];
extern uint8_t received[240];
extern int data_i;
extern Action actionPending;

void processReceived(){
    uint16_t command = received[0];
    switch(command) {
    case 0x01:
        actionPending = SEND_DATA;
        break;
    case 0x31:
        actionPending = NONE;
        sensor_startSampling((received[2]-'0')*10 + ((received[3]-'0')));
        break;
    case 0x32:
        actionPending = SEND_DATA;
        break;
    case 0x33:
        actionPending = CLEAR_DATA;
        break;
    case 0x34:
        actionPending = SEND_PROGRESS;
        break;
    }
}

//compresses 16 bit array to 8 bit array
uint8_t* compressData(uint16_t array[]) {
    uint8_t arrayC[172];
    int offset = 0;
    int mod = 0;
    int i;
    for(i = 0; i < 172; ++i) {
        switch(mod) {
        case 0:
            arrayC[i] = (uint8_t) (array[i-offset] & 0x00FF);
        case 1:
            arrayC[i] = (uint8_t) (((array[i-offset-1] & 0x0F00) >> 8) | ((array[i-offset] & 0x000F) << 4));
        case 2:
            arrayC[i] = (uint8_t) (array[i-offset-1] & 0x00FF);
            offset++;
        }
        mod = (mod+1)%3;
    }

    return arrayC;

}

void sendData(){
    uint8_t arrayC[172];
        int offset = 0;
        int mod = 0;
        int i;
        for(i = 0; i < 172; ++i) {
            switch(mod) {
            case 0:
                arrayC[i] = (uint8_t) (data[i-offset] & 0x00FF);
            case 1:
                arrayC[i] = (uint8_t) (((data[i-offset-1] & 0x0F00) >> 8) | ((data[i-offset] & 0x000F) << 4));
            case 2:
                arrayC[i] = (uint8_t) (data[i-offset-1] & 0x00FF);
                offset++;
            }
            mod = (mod+1)%3;
        }
    transmitMessage(arrayC);
}

void sendProgress() {
    int temp_i = data_i;
    uint8_t toSend[4];
    int i = 0;
    /*while(temp_i > 0) {
        toSend[i] = (uint8_t)((data_i >> (8*i)) & 0x000F);
        temp_i = temp_i>>4;
        i++;
    }*/
    toSend[0] = (uint8_t) ('0' + data_i);
    transmitMessage(toSend);
}

void transmitMessage(uint8_t message_i[]) {

    int i;
    int messageLength = 0;
    uint8_t * message_o;
    uint32_t ui32NdefLength;
    uint32_t ui32PacketIndex;
    uint8_t ui8FragmentLength;

    while(message_i[messageLength] != 0x00) {
        messageLength++;
    }
    messageLength += 2;
    message_o = (uint8_t*)calloc(messageLength+7, sizeof(uint8_t));

        //if(g_ui8SerialBuffer[3] & 0x10)
        //{
        message_o[0] = 209;
        message_o[1] = 1;
        message_o[2] = messageLength + 3;
        message_o[3] = 84;
        message_o[4] = 2;
        message_o[5] = 101;
        message_o[6] = 110;
        for(i = 7; i<messageLength+7-2; ++i) {
            message_o[i] = message_i[i-7];
        }
        message_o[messageLength+7-2] = P2P_ENDCHAR;
        message_o[messageLength+7-1] = 0x00;

        ui32NdefLength = (uint32_t) messageLength + 7;       //text length + 7
        ui32PacketIndex = 0;
        ui8FragmentLength = SNEP_getTxBufferStatus();

        if((uint32_t) ui8FragmentLength > ui32NdefLength)
        {
            ui8FragmentLength = (uint8_t) ui32NdefLength;
        }

        SNEP_setupPacket(&message_o[0], ui32NdefLength,ui8FragmentLength);
}


