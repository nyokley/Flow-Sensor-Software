README
This is the code for the Cerebral Spinal Fluid Shunt Flow Sensor. It involves two projects: InternalDevice and ExternalDevice. 
InternalDevice is to be loaded to the board which will be used to take samples, and ExternalDevice is to be loaded to an MSP430F5529 Launchpad 
connected to a TRF7970A BoosterPack which will be used to read from the internal device.

A large portion of the NFC part of this code was taken from the NFCLink suite. 
An installer for the original software can be found in the google drive at https://drive.google.com/drive/u/0/folders/0B2I92HBFoId4M3lQU1hUd1d3a3c 
and documentation can be found by searching for sloa192a.

Some portions of this code do not fit well into functions, so in order to remove it in order to focus on other aspects of the project, it can be
copy-pasted to and from clipboard.txt in each project. This is typically the case for the NFC code in main.

RUNNING THE CODE:
These two projects were created in Code Composer Studio (CCS), which can be downloaded from the TI website.  
These projects can either be debugged or allowed to run freely. To debug a program, there is simply a debug button. 
To allow a program to run freely, you first enter debugging mode, make sure it is running and not paused at a line, and press stop. 
You then perform a software reset by unplugging and replugging the USB. The program will now be running on the device. 
The internal device can only be interacted with through the debugger in CCS, or wirelessly via the external device. 
The external device can be controlled through a MATLAB script. An example script is included. 

EXTERNAL DEVICE:
This program takes serial inputs through a program such as MATLAB and allows the user to send commands to the internal device.

INTERNAL DEVICE:
This program handles the target's side of wireless communication as well as the necessary periphreal devices.
Wireless Communication:
Magnetic Sensor (ADC)
Battery status and voltage (ADC)
Temperature Sensor (SPI)
EEPROM External Memory (SPI)

General SPI:
The SPI module used is USCB1, and the pins are as follows:
MOSI - P4.1
MISO - P4.2
SCLK - P4.3

Miscellaneous:
MCU_delayMillisecond(uint32_t n_ms) - delays by n_ms milliseconds
To use debugging leds:
	Three LEDS: LEDB, LEDG, LEDR - set port using LEDx_DIR_OUT and turn on/off using LEDx_ON / LEDx_OFF


Thermometer:
The microcontroller communicates with the ADT7320 via SPI. The slave select pin is P3.0.
Thermometer.c includes functions for communicating with the device through SPI. It takes both 8-bit and 16-bit commands, so caution is to be taken
if using these functions directly.
The necessary SPI configuration for this device is as follows: MSB-First, data captured first and changed second (captured on falling edge), clock inactive high.
adt7320_init() initializes the device by sending 32 "1's" and reads from registers in order to make sure it is initializes properly. It should
return 1 if initialized properly, but occasionally a register will be off even though it works properly, so it might be advised to decrease the
strictness of this initialization check (for instance, only returning 0 if 2+ registers are incorrect, or only if the adt7320 isn't returning
anything at all).
adt7320_readTemp() returns a 16-bit temperature value that can be translated according to the datasheet. 

EEPROM memory device:
The microcontroller communicates with the RM25C256C via SPI. The slave select pin is P5.6.
The SPI configuration for this device is somewhat grey in the datasheet, so some experimentation on that front may yield positive results.
The 8-bit instructions are defined as macros in a manner consistent with the datasheet.


CCS TIPS/ADVICE:
When debugging, local variables can be viewed by clicking on the "Expressions" tab in the debug perspective, and the variable can be typed in after
clicking "Add new expression." If the local variable cannot be read, turn optimization off by clicking File -> Properties -> Build -> MSP430 Compiler
-> Optimization and setting the optimization drop down menu to "off." 


