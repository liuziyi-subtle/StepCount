#include "nonwalk_check.h"

#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "nonwalk_model.h"

// #define AR_TH (2.1024327f)
// #define QUANTILE_TH (4735.843f)

#define BUFF_LEN (125u)

static float k_resultant[256u];
static float k_accx[256u];
static float k_accy[256u];
static float k_accz[256u];
// static float* k_mat[10u];
// static float k_mem_pool[110u];
static uint16_t k_counter;

/* _Mean */
#define _Mean _step_c1
static float _Mean(const float* data, uint16_t data_length) {
  uint16_t count;   /*<< loop counter */
  float sum = 0.0f; /*<< temporary result storage */

  /* Initialize counter with number of samples */
  count = data_length;
  while (count > 0u) {
    sum += *data++;
    count--;
  }

  float mean = (sum / (float)data_length);

  return (float)mean;
}

#define _Variance _step_c2
static float _Variance(const float* data, uint16_t data_length, uint16_t ddof) {
  uint16_t counter; /*<< loop counter */
  float sum = 0.0; /*<< temporary result storage */
  float f_sum = 0.0;
  float f_mean, f_value;
  const float* p_input = data;

  if (data_length <= 1u) {
    return .0;
  }

  /* Initialize counter with number of samples */
  counter = data_length;
  while (counter > 0u) {
    sum += *p_input++;
    counter--;
  }

  f_mean = sum / data_length;

  p_input = data;
  counter = data_length;
  while (counter > 0u) {
    f_value = *p_input++ - f_mean;
    f_sum += f_value * f_value;

    counter--;
  }

  /* Variance */
  float variance = (float)(f_sum / (data_length - ddof));

  return variance;
}

/* AutoCorrelation */
#define _AutoCorrelation _step_c3
static float _AutoCorrelation(const float* data, uint16_t data_length, uint8_t lag) {
  float mean = _Mean(data, data_length);
  float variance = _Variance(data, data_length, 0);
  // double variance = _Variance(data, data_length, 0U);

  float sum = .0;
  const float* p = &data[lag];
  // const float* q = &data[data_length - 1 - lag];
  uint16_t i = (uint16_t)lag;
  while (i++ < data_length) {
    // sum += ((double)*p++ - mean) * ((double)*q-- - mean);
    float sum2p = (*p - mean) * (*(p - lag) - mean);
    sum += sum2p;
    p++;
  }

  float result = sum / ((data_length - (uint16_t)lag) * variance);

  return result;
}

/* _NumPeaks */
#define _NumPeaks _step_c4
static uint16_t _NumPeaks(float* data, uint16_t data_length, uint8_t range) {
  uint16_t num_peaks = 0u, i = 0u, count = 0u;
  float* p = &data[range];

  while (p <= &data[data_length - range - 1u]) {
    while (i++ < range) {
      if ((*p > *(p - i)) && (*p > *(p + i))) {
        count += 1u;
      }
      // i++;
    }

    if (count == range) {
      p += (range + 1u);
      num_peaks += 1u;
    } else {
      p++;
    }

    count = 0u;
    i = 0u;
  }

  return num_peaks;
}

/* To left if shift > 0 else right. */
#define _Roll _step_c5
static void _Roll(float* data, int16_t data_length, int16_t shift) {
  /* first roll within [0, shift - 1] and [shift, data_length - 1],
   * then flip the whole data. */
  if (shift == 0) {
    return;
  }

  /* p points to the element which will be the new data[0],
   * p must exchange with data[data_length - 1]. */
  float* p = &data[(data_length + shift) % data_length];
  float* q = &data[data_length - 1];
  float temp;
  while (p < q) {
    temp = *p;
    *p++ = *q;
    *q-- = temp;
  }

  /* p points to the element which will be the new data[data_length - 1],
   * p must exchange with data[0]. */
  p = &data[(data_length + shift - 1) % data_length];
  q = data;
  while (p > q) {
    temp = *p;
    *p-- = *q;
    *q++ = temp;
  }

  /* roll the whole data*/
  p = data;
  q = &data[data_length - 1];
  while (p < q) {
    temp = *p;
    *p++ = *q;
    *q-- = temp;
  }

  return;
}

#define _CmpFunc _step_c6
static inline int _CmpFunc(const void* a, const void* b) {
  return ((*(float*)a > *(float*)b) ? 1 : (*(float*)a < *(float*)b) ? -1 : 0);
  // return (*(float*)a > *(float*)b) ? 1 : -1;
}

/* sorted. */
#define _Quantile _step_c7
static float _Quantile(const float* data, uint16_t data_length, float q) {
  float qIndexRight = 1.0f + (data_length - 1.0f) * q;
  float qIndexLeft = floorf(qIndexRight);
  float fraction = qIndexRight - qIndexLeft;
  uint16_t qIndex = (uint16_t)qIndexLeft;
  float quantile =
      data[qIndex - 1u] + (data[qIndex] - data[qIndex - 1u]) * fraction;
  return quantile;
}

#define _ChangeQuantile _step_c8
static float _ChangeQuantile(float* data, float* diff, uint16_t data_length,
                             float qh, float ql, bool is_abs, uint8_t method) {
  uint16_t i;
  // float* diff = k_data + data_length;
  uint16_t diff_length = 0u;

  for (i = 1; i < data_length; ++i) {
    if (((data[i] >= ql) && (data[i - 1] >= ql)) &&
        ((data[i] <= qh) && (data[i - 1] <= qh))) {
      if (is_abs) {
        diff[diff_length++] = fabsf(data[i] - data[i - 1]);
      } else {
        diff[diff_length++] = data[i] - data[i - 1];
      }
    }
  }

  if (method == 0u) {
    return _Mean(diff, diff_length);
  } else {
    return _Variance(diff, diff_length, 0u);
  }
}

/* Interface
 * params:
 * @data           input.
 * @data_length    input size.
 * @enable         update buffer without performing complex computing if 0.
 * @init           initialize.
 */
uint8_t NonWalkCheck(const int16_t* accx, const int16_t* accy, const int16_t* accz,
                     uint8_t data_length, uint8_t enable, uint8_t init) {
  uint16_t i;
  /* Initialize. */
  if (init) {
    for (i = 0u; i < 2 * BUFF_LEN; ++i) {
      k_resultant[i] = .0f;
      k_accx[i] = .0f;
      k_accy[i] = .0f;
      k_accz[i] = .0f;
    }

    k_counter = 0u;

    return 0u;
  }

  k_counter += 1;

  /* data_length must < BUFF_LEN. */
  if ((data_length == 0u) || (data_length > BUFF_LEN)) {
    return 0u;
  }

  uint8_t check_result = 0u; /*<< 0: walk, 1: noise. */

  if (enable == 0u) {
    /* update x, y, z, resultant buffer. */
    _Roll(k_accx, BUFF_LEN, (int16_t)data_length);
    _Roll(k_accy, BUFF_LEN, (int16_t)data_length);
    _Roll(k_accz, BUFF_LEN, (int16_t)data_length);
    _Roll(k_resultant, BUFF_LEN, (int16_t)data_length);

    for (i = 0u; i < data_length; ++i) {
      k_accx[BUFF_LEN - data_length + i] = (float)accx[i];
      k_accy[BUFF_LEN - data_length + i] = (float)accy[i];
      k_accz[BUFF_LEN - data_length + i] = (float)accz[i];
      k_resultant[BUFF_LEN - data_length + i] =
          (float)(accx[i] * accx[i] + accy[i] * accy[i] + accz[i] * accz[i]);
    }

    return check_result;
  }

  /*
   * 'AccX__autocorrelation__lag_7',
   * 'AccX__maximum',
   * 'AccY__number_peaks__n_1',
   * 'AccZ__maximum',
   * 'AccZ__change_quantiles__f_agg_"var"__isabs_True__qh_1.0__ql_0.0',
   * 'Resultant__change_quantiles__f_agg_"var"__isabs_False__qh_0.8__ql_0.4'
   */

  /* AccX__autocorrelation__lag_7. */
  float autocorr_x_7 = _AutoCorrelation(k_accx, BUFF_LEN, 7u);
  // printf("autocorr_x_7: %f\n", autocorr_x_7);

  /* AccX__maximum. */
  float* sorted_accx = k_accx + BUFF_LEN;
  for (i = 0; i < BUFF_LEN; ++i) {
    sorted_accx[i] = k_accx[i];
  }
  qsort(sorted_accx, BUFF_LEN, sizeof(float), _CmpFunc);
  float maximum_accx = sorted_accx[BUFF_LEN - 1];
  // printf("maximum_accx: %f\n", maximum_accx);

  /* AccY__number_peaks__n_1. */
  uint16_t num_peaks_accy = _NumPeaks(k_accy, BUFF_LEN, 1u);
  // printf("num_peaks_accy: %u\n", num_peaks_accy);

  /* AccZ__maximum. */
  float* sorted_accz = k_accz + BUFF_LEN;
  for (i = 0; i < BUFF_LEN; ++i) {
    sorted_accz[i] = k_accz[i];
  }
  qsort(sorted_accz, BUFF_LEN, sizeof(float), _CmpFunc);
  float maximum_accz = sorted_accz[BUFF_LEN - 1];
  // printf("maximum_accz: %f\n", maximum_accz);

  /* AccZ__change_quantiles__f_agg_"var"__isabs_True__qh_1.0__ql_0.0 */
  float minimum_accz = sorted_accz[0];
  float cq_000_100_accz = _ChangeQuantile(k_accz, k_accz + BUFF_LEN,
                                          BUFF_LEN, maximum_accz, minimum_accz,
                                          true, 1u);
  // printf("cq_000_100_accz: %f\n", cq_000_100_accz);

  /* sort resultant data. */
  float* sorted_resultant = k_resultant + BUFF_LEN;
  for (i = 0; i < BUFF_LEN; ++i) {
    sorted_resultant[i] = k_resultant[i];
  }
  qsort(sorted_resultant, BUFF_LEN, sizeof(float), _CmpFunc);

  float q_040 = _Quantile(sorted_resultant, BUFF_LEN, 0.4f);
  float q_080 = _Quantile(sorted_resultant, BUFF_LEN, 0.8f);

  /* Resultant__change_quantiles__f_agg_"mean"__isabs_True__qh_0.8__ql_0.4. */
  float cq_040_080 = _ChangeQuantile(k_resultant, k_resultant + BUFF_LEN,
                                     BUFF_LEN, q_080, q_040, false, 1u);
  // printf("cq_040_080: %f\n", cq_040_080);

  /* set feats */
  union NonWalkModelEntry feats[6];
  feats[0].fvalue = autocorr_x_7;
  feats[1].fvalue = maximum_accx;
  feats[2].fvalue = (float)num_peaks_accy;
  feats[3].fvalue = maximum_accz;
  feats[4].fvalue = cq_000_100_accz;
  feats[5].fvalue = cq_040_080;

  /* predict */
  float proba = NonWalkModelPredict(feats);
  if (proba > 0.7f) {
    check_result = 1u;
  }
  printf("---------> proba: %f\n", proba);

  /* noise if k_counter < 5 seconds after initialization. */
  if (k_counter < 5u) {
    check_result = 1u;
  }
  check_result = 0;

  return check_result;
}
