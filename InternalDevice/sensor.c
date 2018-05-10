/*
 * sensor.c
 *
 *  Created on: Mar 27, 2018
 *      Author: Nathaniel
 */

#include <msp430.h>
#include <stdint.h>
#include "sensor.h"
#include "timer_a.h"
#include "driverlib.h"
#include "adc12_a.h"
#include "HBAL.h"
#include <stdio.h>

extern SensorState sensorState;

uint16_t data[128];
int data_i = 0;
int data_max = 0;
int cnt_t = 0;
int toggle = 0;

void dataArray_clear() {
    int i;
    for(i = 0; i < 128; ++i) {
        data[i] = 0;
    }
}

void sensor_startSampling(int numberSamples) {
    sensor_init_adc();
    sensor_start_timer();
    data_max = numberSamples;
    data_i = 0;
}

void sensor_stopSampling() {
    ADC12CTL0 &= ~ADC12ON;
    TIMER_A_stop(TIMER_A2_BASE);
}

void sensor_init_adc() {
    /*ADC12CTL0 = ADC12SHT02 + ADC12ON;         // Sampling time, ADC12 on
    ADC12CTL1 = ADC12SHP;                     // Use sampling timer
    ADC12IE = 0x01;                           // Enable interrupt
    ADC12CTL0 |= ADC12ENC;
    P6SEL |= 0x01;                            // P6.0 ADC option select
    P1DIR |= 0x01;
                        // P1.0 output*/
    /*P6SEL |= BIT0;
    ADC12CTL0 = ADC12SHT0_2 | ADC12ON;      // Sampling time, S&H=16, ADC12 on
    ADC12CTL1 = ADC12SHP;                   // Use sampling timer
    ADC12CTL2 |= ADC12RES_2;                // 12-bit conversion results
    ADC12MCTL0 |= ADC12INCH_0;              // A0 ADC input select; Vref=AVCC
    ADC12IE = ADC12IE0;                  // Enable ADC conv complete interrupt
    */

    GPIO_setAsPeripheralModuleFunctionOutputPin(
            GPIO_PORT_P6,
            GPIO_PIN5
            );
    GPIO_setAsPeripheralModuleFunctionOutputPin(
                GPIO_PORT_P6,
                GPIO_PIN3
                );
    //
    //GPIO_setAsOutputPin(GPIO_PORT_P3, GPIO_PIN5);
    //GPIO_setOutputHighOnPin(GPIO_PORT_P3, GPIO_PIN5);

    ADC12_A_init(ADC12_A_BASE,
            ADC12_A_SAMPLEHOLDSOURCE_SC,
            ADC12_A_CLOCKSOURCE_ACLK,
            ADC12_A_CLOCKDIVIDER_1);

    ADC12_A_enable(ADC12_A_BASE);
    ADC12_A_setupSamplingTimer(ADC12_A_BASE,
            ADC12_A_CYCLEHOLD_64_CYCLES,
            ADC12_A_CYCLEHOLD_4_CYCLES,
            ADC12_A_MULTIPLESAMPLESDISABLE);

        ADC12_A_memoryConfigure(ADC12_A_BASE,
                                ADC12_A_MEMORY_0,
                                ADC12_A_INPUT_A5,
                                ADC12_A_VREFPOS_AVCC,
                                ADC12_A_VREFNEG_AVSS,
                                ADC12_A_NOTENDOFSEQUENCE);
        ADC12_A_memoryConfigure(ADC12_A_BASE,
                                        ADC12_A_MEMORY_1,
                                        ADC12_A_INPUT_A3,
                                        ADC12_A_VREFPOS_AVCC,
                                        ADC12_A_VREFNEG_AVSS,
                                        ADC12_A_NOTENDOFSEQUENCE);

        //Enable memory buffer 0 interrupt
        ADC12_A_clearInterrupt(ADC12_A_BASE,
            ADC12IFG0);
        ADC12_A_enableInterrupt(ADC12_A_BASE,
            ADC12IE0);

        ADC12_A_clearInterrupt(ADC12_A_BASE,
                    ADC12IFG1);
        ADC12_A_enableInterrupt(ADC12_A_BASE,
            ADC12IE1);

}

void sensor_start_timer() {
    TIMER_A_clearTimerInterruptFlag(TIMER_A2_BASE);

    //Start timer in up mode sourced by ACLK
    TIMER_A_configureUpMode(TIMER_A2_BASE,
                            TIMER_A_CLOCKSOURCE_ACLK,
                            TIMER_A_CLOCKSOURCE_DIVIDER_1,
                            (3277),
                            TIMER_A_TAIE_INTERRUPT_ENABLE,
                            TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
                            TIMER_A_DO_CLEAR);
    __enable_interrupt();

    TIMER_A_startCounter(TIMER_A2_BASE, TIMER_A_UP_MODE);
}

#pragma vector=TIMER2_A1_VECTOR
__interrupt void TIMER_A2_ISR(void)
{
    TA2CTL &= ~TAIFG;
    TA2CTL |= TACLR;
    if(cnt_t >= 5) {
    //ADC12CTL0 |= ADC12ENC | ADC12SC;    // Start sampling/conversion
    if(toggle == 0) {
        toggle = 0;             //toggle disabled
        ADC12_A_startConversion(ADC12_A_BASE,
               ADC12_A_MEMORY_0,
               ADC12_A_SINGLECHANNEL);
    }
    else if(toggle == 1) {
        toggle = 0;
        ADC12_A_startConversion(ADC12_A_BASE,
                  ADC12_A_MEMORY_1,
                  ADC12_A_SINGLECHANNEL);
    }
    sensorState.readTemp = 1;
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
            //Is Memory Buffer 0 = A0 > 0.5AVcc?
            if(sensorState.sampleIndex1<100) {
            sensorState.sampleArray1[0] = (uint16_t) ADC12_A_getResults(ADC12_A_BASE,
                                       ADC12_A_MEMORY_0);
            }
            //printf("    A4: %d\n", (int) sensorState.sampleArray1[sensorState.sampleIndex1-1]);
            break;
            //Exit active CPU
            //__bic_SR_register_on_exit(LPM0_bits);
        case  8:    //Vector  8:  ADC12IFG1
           sensorState.sampleArray2[sensorState.sampleIndex2++] = (uint16_t) ADC12_A_getResults(ADC12_A_BASE,
                                             ADC12_A_MEMORY_1);
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



