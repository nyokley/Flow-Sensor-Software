/*
 * sensor.c
 *
 *  Created on: Mar 27, 2018
 *      Author: Nathaniel
 *
 *  Contains functions pertaining to the magnetic sensor - particularly concerns the ADC.
 *  Handles the timer which is used to determine when samples are taken from the ADC and the temperature sensor.
 */

#include <ASAL.h>
#include <msp430.h>
#include <stdint.h>
#include "sensor.h"
#include "timer_a.h"
#include "driverlib.h"
#include "adc12_a.h"
#include "config.h"
#include <stdio.h>

extern SensorState sensorState;



int cnt_t = 0;
int toggle = 0;

void dataArray_clear() {
    int i;
    for(i = 0; i < SAMPLE_ARRAY_SIZE; ++i) {
        sensorState.sampleArray1[i] = 0;
        sensorState.sampleArray2[i] = 0;
    }
    for(i = 0; i < TEMP_ARRAY_SIZE; ++i) {
        sensorState.tempArray[i] = 0;
    }

}

//sets up adc and starts timer
//assumes that there is no sampling in progress; might need to write a "continue sampling" function
void sensor_startSampling(int numberSamples) {
    sensor_init_adc();
    sensor_start_timer();
    BRIDGE_VCC_SETDIR;
    sensorState.readTemp = 0;
    sensorState.sampleIndex1 = 0;
    sensorState.sampleIndex2 = 0;
    sensorState.tempIndex = 0;
    sensorState.samplesMax = numberSamples;
}

//turns off adc and timer
void sensor_stopSampling() {
    ADC12CTL0 &= ~ADC12ON;
    TIMER_A_stop(TIMER_A2_BASE);
}

void sensor_init_adc() {
    //P6.5 - A5: connected to sensor output prior to op-amp
    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P6,
            GPIO_PIN5
            );
    //P.3 - A3: connected to sensor output; what will be read and recorded
    GPIO_setAsPeripheralModuleFunctionOutputPin(
                GPIO_PORT_P6,
                GPIO_PIN3
                );

    //set up ADC12
    ADC12_A_init(ADC12_A_BASE,
            ADC12_A_SAMPLEHOLDSOURCE_SC,
            ADC12_A_CLOCKSOURCE_ACLK,
            ADC12_A_CLOCKDIVIDER_1);

    ADC12_A_enable(ADC12_A_BASE);
    ADC12_A_setupSamplingTimer(ADC12_A_BASE,
            ADC12_A_CYCLEHOLD_64_CYCLES,
            ADC12_A_CYCLEHOLD_4_CYCLES,
            ADC12_A_MULTIPLESAMPLESDISABLE);

    //configure P6.5 to store data inside MEMORY_0 reg
    ADC12_A_memoryConfigure(ADC12_A_BASE,
                            ADC12_A_MEMORY_0,
                            ADC12_A_INPUT_A5,
                            ADC12_A_VREFPOS_AVCC,
                            ADC12_A_VREFNEG_AVSS,
                            ADC12_A_NOTENDOFSEQUENCE);
    //configure P6.3 to store data inside MEMORY_1 reg
    ADC12_A_memoryConfigure(ADC12_A_BASE,
                            ADC12_A_MEMORY_1,
                            ADC12_A_INPUT_A3,
                            ADC12_A_VREFPOS_AVCC,
                            ADC12_A_VREFNEG_AVSS,
                            ADC12_A_NOTENDOFSEQUENCE);

        //Enable memory buffer 0 interrupt
        //for P6.5
        ADC12_A_clearInterrupt(ADC12_A_BASE,
            ADC12IFG0);
        ADC12_A_enableInterrupt(ADC12_A_BASE,
            ADC12IE0);
        //for P6.3
        ADC12_A_clearInterrupt(ADC12_A_BASE,
                    ADC12IFG1);
        ADC12_A_enableInterrupt(ADC12_A_BASE,
            ADC12IE1);

}

void sensor_start_timer() {
    TIMER_A_clearTimerInterruptFlag(TIMER_A2_BASE);

    //Start timer in up mode sourced by ACLK (32768 Hz)
    TIMER_A_configureUpMode(TIMER_A2_BASE,
                            TIMER_A_CLOCKSOURCE_ACLK,
                            TIMER_A_CLOCKSOURCE_DIVIDER_1,
                            (3277),     // = 32768/desired_frequency
                            TIMER_A_TAIE_INTERRUPT_ENABLE,
                            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
                            TIMER_A_DO_CLEAR);
    __enable_interrupt();

    TIMER_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
}

#pragma vector=TIMER2_A1_VECTOR
__interrupt void TIMER_A2_ISR(void)
{
    //clear timer register and interrupt flag
    TA2CTL &= ~TAIFG;
    TA2CTL |= TACLR;

    //use cnt_t for a timer period if timer register too small (e.g. if timer period is 0.1s, cnt_t = 5
    //creates timer period 0.5s)
    if(cnt_t >= 5) {
        BRIDGE_VCC_HIGH;    //note: might need to give sensors time to settle

        //toggles between the two ADC sources, since using both would take too much time. This will be taken out.
        if(toggle == 0) {
            toggle = 0;             //toggle disabled
            ADC12_A_startConversion(ADC12_A_BASE,
                   ADC12_A_MEMORY_0,        //P6.5
                   ADC12_A_SINGLECHANNEL);
        }
        else if(toggle == 1) {
            toggle = 0;
            ADC12_A_startConversion(ADC12_A_BASE,
                      ADC12_A_MEMORY_1,     //P6.3
                      ADC12_A_SINGLECHANNEL);
        }
        sensorState.readTemp = 1;   //can create another counter to control how often the temp is read
        cnt_t = 0;
    }
    else {
        cnt_t++;
    }
}



#if defined(__TI_COMPILER_VERSION__) || defined(__IAR_SYSTEMS_ICC__)
#pragma vector=ADC12_VECTOR
__interrupt
#elif defined(__GNUC__)
__attribute__((interrupt(ADC12_VECTOR)))
#endif
void ADC12_A_ISR (void)
{

    switch (__even_in_range(ADC12IV,34)){
        case  0: break;   //Vector  0:  No interrupt
        case  2: break;   //Vector  2:  ADC overflow
        case  4: break;   //Vector  4:  ADC timing overflow
        case  6:          //Vector  6:  ADC12IFG0
            if(sensorState.sampleIndex1<SAMPLE_ARRAY_SIZE) {
                sensorState.sampleArray1[sensorState.sampleIndex1++] = (uint16_t) ADC12_A_getResults(ADC12_A_BASE,
                                                                                                   ADC12_A_MEMORY_0);
            }
            BRIDGE_VCC_LOW;
            //printf("    A5: %d\n", (int) sensorState.sampleArray1[sensorState.sampleIndex1-1]);
            break;
            //Exit active CPU
            //__bic_SR_register_on_exit(LPM0_bits);
        case  8:    //Vector  8:  ADC12IFG1
           if(sensorState.sampleIndex2 < SAMPLE_ARRAY_SIZE) {
               sensorState.sampleArray2[sensorState.sampleIndex2++] = (uint16_t) ADC12_A_getResults(ADC12_A_BASE,
                                                                                                    ADC12_A_MEMORY_1);
           }
           BRIDGE_VCC_LOW;
           //printf("A3: %d\n", (int) sensorState.sampleArray2[sensorState.sampleIndex2-1]);
            break;
        case 10: break;   //Vector 10:  ADC12IFG2
        case 12: break;   //Vector 12:  ADC12IFG3
        case 14: break;   //Vector 14:  ADC12IFG4
        case 16: break;   //Vector 16:  ADC12IFG5
        case 18: break;   //Vector 18:  ADC12IFG6
        case 20: break;   //Vector 20:  ADC12IFG7
        case 22: break;   //Vector 22:  ADC12IFG8
        case 24: break;   //Vector 24:  ADC12IFG9
        case 26: break;   //Vector 26:  ADC12IFG10
        case 28: break;   //Vector 28:  ADC12IFG11
        case 30: break;   //Vector 30:  ADC12IFG12
        case 32: break;   //Vector 32:  ADC12IFG13
        case 34: break;   //Vector 34:  ADC12IFG14
        default: break;
    }
}



