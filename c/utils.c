/**
 * @file utils.c
 *
 * @brief Utility functions.
 *
 * @author Song Yadong
 *
 * @version 1.0
 *
 * @copyright Lifesense 2019-.
 */


#include "utils.h"

#ifndef __CC_ARM
void arm_mean_f32(
  float * pSrc,
  uint32_t blockSize,
  float * pResult)
{
    float sum = 0.0f;
    if (blockSize == 0)
    {
        *pResult = 0.0f;
        return;
    }

    for (uint32_t i = 0; i < blockSize; i++)
    {
        sum += pSrc[i];
    }
    *pResult = sum / blockSize;
}

void arm_var_f32(
  float * pSrc,
  uint32_t blockSize,
  float * pResult)
{

  float sum = 0.0f;                          /* Temporary result storage */
  float sumOfSquares = 0.0f;                 /* Sum of squares */
  float in;                                  /* input value */
  uint32_t blkCnt;                               /* loop counter */

#ifndef ARM_MATH_CM0_FAMILY
   
  /* Run the below code for Cortex-M4 and Cortex-M3 */

  float meanOfSquares, mean, squareOfMean;   /* Temporary variables */

    if(blockSize == 1)
    {
        *pResult = 0;
        return;
    }

  if (blockSize == 0)
  {
    *pResult = 0;
    return;
  }

  /*loop Unrolling */
  blkCnt = blockSize >> 2u;

  /* First part of the processing with loop unrolling.  Compute 4 outputs at a time.    
   ** a second loop below computes the remaining 1 to 3 samples. */
  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1])  */
    /* Compute Sum of squares of the input samples    
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* If the blockSize is not a multiple of 4, compute any remaining output samples here.    
   ** No loop unrolling is used. */
  blkCnt = blockSize % 0x4u;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples    
     * and then store the result in a temporary variable, sum. */
    in = *pSrc++;
    sum += in;
    sumOfSquares += in * in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute Mean of squares of the input samples    
   * and then store the result in a temporary variable, meanOfSquares. */
  meanOfSquares = sumOfSquares / ((float) blockSize - 1.0f);

  /* Compute mean of all input values */
  mean = sum / (float) blockSize;

  /* Compute square of mean */
  squareOfMean = (mean * mean) * (((float) blockSize) /
                                  ((float) blockSize - 1.0f));

  /* Compute variance and then store the result to the destination */
  *pResult = meanOfSquares - squareOfMean;

#else

  /* Run the below code for Cortex-M0 */
  float squareOfSum;                         /* Square of Sum */

    if(blockSize == 1)
    {
        *pResult = 0;
        return;
    }

  if(blockSize == 0)
  {
    *pResult = 0;
    return;
  }

  
  /* Loop over blockSize number of values */
  blkCnt = blockSize;

  while(blkCnt > 0u)
  {
    /* C = (A[0] * A[0] + A[1] * A[1] + ... + A[blockSize-1] * A[blockSize-1]) */
    /* Compute Sum of squares of the input samples     
     * and then store the result in a temporary variable, sumOfSquares. */
    in = *pSrc++;
    sumOfSquares += in * in;

    /* C = (A[0] + A[1] + ... + A[blockSize-1]) */
    /* Compute Sum of the input samples     
     * and then store the result in a temporary variable, sum. */
    sum += in;

    /* Decrement the loop counter */
    blkCnt--;
  }

  /* Compute the square of sum */
  squareOfSum = ((sum * sum) / (float) blockSize);

  /* Compute the variance */
  *pResult = ((sumOfSquares - squareOfSum) / (float) (blockSize - 1.0f));

#endif /* #ifndef ARM_MATH_CM0_FAMILY */

}
#endif // __CC_ARM

// float step_max(float a, float b) {
//     return a > b ? a : b;
// }

// float step_min(float a, float b) {
//     return a < b ? a : b;
// }

float moving_avg(float ndata, float *sum, float *reg, uint8_t *pos, uint8_t len) {
    *sum = *sum + ndata - reg[*pos];
    reg[*pos] = ndata;
    (*pos)++;
    if (*pos >= len)
        *pos = 0;
    return (*sum / len);
}
