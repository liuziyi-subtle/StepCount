/**
 * @file utils.h
 *
 * @brief Header file for utility functions.
 *
 * @author Song Yadong
 *
 * @version 1.0
 *
 * @copyright Lifesense 2019-.
 */

#ifndef __UTILS_H__
#define __UTILS_H__

#include <stdint.h>
#ifndef __CC_ARM
void arm_mean_f32(
  float * pSrc,
  uint32_t blockSize,
  float * pResult);

void arm_var_f32(
  float * pSrc,
  uint32_t blockSize,
  float * pResult);
#endif // __CC_ARM

/* maximum of two numbers */
// float step_max(float a, float b);

/* minimum of two numbers */
// float step_min(float a, float b);

/**
 * moving_avg: moving-average filter
 * @param
 *  ndata: new input data
 *  sum: pointer to summation result
 *  reg: pointer to data regester
 *  pos: pointer to last data position in register
 *  len: moving-average window
 * @return
 *  average result
 */
 #define moving_avg _step_um
float moving_avg(float ndata, float *sum, float *reg, uint8_t *pos, uint8_t len);

#endif /* __UTILS_H__ */
