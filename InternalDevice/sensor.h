/*
 * sensor.h
 *
 *  Created on: Mar 27, 2018
 *      Author: Nathaniel
 */

#ifndef SENSOR_H_
#define SENSOR_H_


void sensor_startSampling(int numberSamples);
void sensor_stopSampling();
void dataArray_clear();
void sensor_start_timer();
void sensor_init_adc();

#endif /* SENSOR_H_ */
