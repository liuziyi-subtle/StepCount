/**
 * @file step.h
 *
 * @brief Header file for step counting algorithm.
 *
 * @author Song Yadong
 *
 * @version 1.0
 *
 * @copyright Lifesense 2019-.
 */

#ifndef __STEP_H__
#define __STEP_H__

#include <stdint.h>

/* Constants */
#define FS            25                                                /* Hz, sampling frequency */
#define AXIS_NUM      3                                                 /* x, y and z axes */

/* InitStepPara: initialize step algorithm parameters */
void Step_Init(void);

/* Step_Detection: push raw x, y and z axes data and return steps */
/* G-sensor dynamic range: +/- 8G; 64 represents 1G */
uint8_t Step_Detection(const int16_t acc[3]);


#endif /* STEP_H */
