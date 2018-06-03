//*****************************************************************************
//
// main.c
//
// Copyright (c) 2015 Texas Instruments Incorporated.  All rights reserved.
// TI Information - Selective Disclosure
//
//*****************************************************************************
#include <ASAL.h>
#include "msp430.h"
#include "types.h"
#include "nfc_controller.h"
#include "sensor.h"
#include "thermometer.h"
#include "eeprom.h"
#include "mcu.h"
#include <stdio.h>
#include "tag_header.h"

//buffer to store packets from other NFC device
uint8_t received[240];
_Bool newReceived = 0;
_Bool canSend = 0;
extern int data_i;
Action actionPending =  NONE;

//
// Buffer to store incoming packets from NFC host
//
uint8_t g_ui8SerialBuffer[265];

//
// Number of bytes received from the host
//
volatile uint16_t g_ui16BytesReceived = 0x00;

bool g_bEnableAutoSDD;
bool g_bExtAmplifier;
bool g_bTRF5VSupply;
tTRF79x0_Version g_eTRFVersion;
bool g_bSupportCertification;
uint16_t g_ui16ListenTime;

#if (NFC_PEER_2_PEER_INITIATOR_ENABLED || NFC_PEER_2_PEER_TARGET_ENABLED)
    t_sNfcP2PMode g_sP2PSupportedModes;
    t_sNfcP2PCommBitrate g_sP2PSupportedTargetBitrates;
    t_sNfcP2PCommBitrate g_sP2PSupportedInitiatorBitrates;
    t_sNfcDEP_P2PSetup g_sP2PSetupOptions;
    uint8_t g_ui8NfcDepInitiatorDID;
#endif


void NFC_configuration(void);
void Serial_processCommand(void);


uint8_t g_ui8TxBuffer[256];
uint8_t g_ui8TxLength;

void NFC_initIDs(void)
{
    // NFC ID's
    uint8_t pui8NfcFId[8] = {0x01,0xFE,0x88,0x77,0x66,0x55,0x44,0x33};  // Type F ID for P2P

    // Set the NFC Id's for Type A, Type B, and Type F
    NFC_F_setNfcId2(pui8NfcFId,8);
}

SensorState sensorState;

void main(void)
{
    int delaySend = 1;
    int sent = 0;
    int p2p_on = 0;
    int rw_on = 0;
    int counter = 0;
    int formatted = 0;
    tNfcState eTempNFCState;
    tNfcState eCurrentNFCState;

    // CE Variables
    t_sNfcCEMode sCEMode;


#if (NFC_PEER_2_PEER_INITIATOR_ENABLED || NFC_PEER_2_PEER_TARGET_ENABLED)
    // Peer to Peer TX Variables
    uint32_t ui32PacketRemaining;
    uint8_t ui8TXBytes;
    uint16_t ui16TxIndex;
    uint32_t ui32PacketLength;
    uint8_t * pui8NdefPointer;
    uint8_t ui8FragmentSize;
    char pcBytesReceivedString[5];

    // Bytes Received from Peer to Peer
    uint16_t ui16BytesReceived = 0x00;

    // Peer to peer RX Status
    tNfcP2PRxStatus sP2PRxStatus;
#endif
    t_sNfcP2PMode sP2PMode;
    t_sNfcP2PCommBitrate sP2PBitrate;

    // Initialize MCU
    MCU_init();

    //Enable interrupts globally
    __enable_interrupt();

    // Initialize USB Communication -- will have to restore since external device isn't working
  //  Serial_init();

    // Initialize TRF7970 -- keep even when not using NFC because it sets up SPI
    TRF79x0_init();

    //TRF79x0_idleMode();

    // Initialize the NFC Controller
    //NFC_init();

    // This function will configure all the settings for each protocol
    //NFC_configuration();

    // Initialize IDs for NFC-A, NFC-B and NFC-F
    //NFC_initIDs();

    uint8_t rdsr = 0;
    uint8_t rdsrTest = 0;
    adt7320_init();
    //sensor_start_timer();
    while(1)
    {
        check = adt7320_readTemp();
        MCU_delayMillisecond(5);

        LEDB_OFF;
        MCU_delayMillisecond(5);

        /*adt7320_init();
        MCU_delayMillisecond(2);
        adt7320_readTemp();
        //__delay_cycles(1000);
        MCU_delayMillisecond(2);*/

        /*EEPROM_SPI_instruction(WREN);
        MCU_delayMillisecond(1);
        rdsr = EEPROM_ReadStatusRegister();
        MCU_delayMillisecond(1);
        EEPROM_SPI_instruction(RES);
        MCU_delayMillisecond(1);
        //__delay_cycles(1000);
        if(rdsr < 255) {
           // printf("%d\n", rdsr);
        }*/

        /*if(rdsr != 0) {
            printf("%d\n", (int)rdsr);
        }*/
        //__delay_cycles(10000);

        /*if(sensorState.readTemp) {
            sensorState.readTemp = 0;
            sensorState.tempInterval = adt7320_readTemp();
        }*/

    }
}

//*****************************************************************************
//
//! NFC_configuration - Handles the initial NFC configuration.
//!
//! Setup all NFC Mode Parameters.
//!
//! Current modes enabled: Card Emulation
//! Current modes supported: Card Emulationa and Peer 2 Peer
//! Reader/Writer is NOT supported yet.
//!
//*****************************************************************************

void NFC_configuration(void)
{
#if (NFC_PEER_2_PEER_INITIATOR_ENABLED || NFC_PEER_2_PEER_TARGET_ENABLED)
    g_sP2PSupportedModes.ui8byte = 0x00;
    g_sP2PSupportedTargetBitrates.ui8byte = 0x00;
    g_sP2PSupportedInitiatorBitrates.ui8byte = 0x00;
    g_sP2PSetupOptions.ui8byte = 0x00;
#endif

#if NFC_CARD_EMULATION_ENABLED
    g_sCESupportedModes.ui8byte = 0x00;
#endif
#if NFC_READER_WRITER_ENABLED
    g_sRWSupportedModes.ui8byte = 0x00;
    g_sRWSupportedBitrates.ui16byte = 0x0000;
    g_sRWSetupOptions.ui16byte = 0x0000;
#endif

    // Set the TRF7970 Version being used
    g_eTRFVersion = TRF7970_A;

    // External Amplifer (disconnected by default)
    g_bExtAmplifier = false;

    // Configure TRF External Amplifier for the transceiver
    TRF79x0_setExtAmplifer(g_bExtAmplifier);

    // Configure TRF Power Supply (5V = true, 3V = false)
#ifdef MSP430F5529_EXP_BOARD_ENABLED
    g_bTRF5VSupply = true;
#else
    g_bTRF5VSupply = false;
#endif

    // Configure TRF Power Supply
    TRF79x0_setPowerSupply(g_bTRF5VSupply);

    // Milliseconds the NFC stack will be in listen mode
    g_ui16ListenTime = 500;

    // Set the time the NFC stack will be with the RF field disabled (listen mode)
    NFC_setListenTime(g_ui16ListenTime);

    // Enable (1) or disable (0) the Auto SDD Anti-collision function of the TRF7970A
    g_bEnableAutoSDD = 0;

#if (NFC_PEER_2_PEER_INITIATOR_ENABLED || NFC_PEER_2_PEER_TARGET_ENABLED)

    // Enable Peer 2 Peer Supported Modes
    g_sP2PSupportedModes.bits.bTargetEnabled = 1;
    g_sP2PSupportedModes.bits.bInitiatorEnabled = 1;

    // Set P2P Supported Bit Rates - Target mode
    g_sP2PSupportedTargetBitrates.bits.bPassive106kbps = 1;
    g_sP2PSupportedTargetBitrates.bits.bPassive212kbps = 1;
    g_sP2PSupportedTargetBitrates.bits.bPassive424kbps = 1;
    g_sP2PSupportedTargetBitrates.bits.bActive106kbps = 0;
    g_sP2PSupportedTargetBitrates.bits.bActive212kbps = 0;
    g_sP2PSupportedTargetBitrates.bits.bActive424kbps = 0;

    // Set P2P Supported Bit Rates - Initiator mode
    g_sP2PSupportedInitiatorBitrates.bits.bPassive106kbps = 1;
    g_sP2PSupportedInitiatorBitrates.bits.bPassive212kbps = 1;
    g_sP2PSupportedInitiatorBitrates.bits.bPassive424kbps = 1;
    g_sP2PSupportedInitiatorBitrates.bits.bActive106kbps = 0;
    g_sP2PSupportedInitiatorBitrates.bits.bActive212kbps = 0;
    g_sP2PSupportedInitiatorBitrates.bits.bActive424kbps = 0;

    // Certification Config Start //

    // Enable (1) or disable (0) Wave 1 NFC Forum Certification functionality
    // Note: Enabling this feature can affect interoperability with NFC Devices that are not certified.
    g_bSupportCertification = 0;

    // Allows for Customization of the DID (Device Identification) number when in initiator mode
    g_ui8NfcDepInitiatorDID = 0x00;

    // Enable LLCP
    g_sP2PSetupOptions.bits.bP2PSupportLLCP = 1;

    // Enable Loopback
    g_sP2PSetupOptions.bits.bP2PSupportLoopback = 0;

    // Specify maximum number of timeouts and protocol errors allowed before resetting
    g_sP2PSetupOptions.bits.ui3P2PMaxTimeouts = 2;
    g_sP2PSetupOptions.bits.ui3P2PMaxProtocolErrors = 2;
#endif

#if (NFC_PEER_2_PEER_INITIATOR_ENABLED || NFC_PEER_2_PEER_TARGET_ENABLED)
    // Configure Peer 2 Peer functions for the correct modes and communication bitrates
    NFC_P2P_configure(g_sP2PSupportedModes,g_sP2PSupportedTargetBitrates,g_sP2PSupportedInitiatorBitrates);

    // Configure NFC DEP functions including passing the DID
    NFCDEP_configure_P2P(g_sP2PSetupOptions,g_bSupportCertification,g_ui8NfcDepInitiatorDID);
#endif

// Set the Auto SDD flag within nfc_a.c
NFC_A_setAutoSDD(g_bEnableAutoSDD);

// Set the current TRF version within trf79x0.c
TRF79x0_setVersion(g_eTRFVersion);

// Set Certification Support for all Protocols - Required for NFC Forum Certification
NFC_setSupportCertification(g_bSupportCertification);

// Set Test Enable flag within trf79x0.c - Required for NFC Forum Certification
TRF79x0_testFlag(g_bSupportCertification);

}






