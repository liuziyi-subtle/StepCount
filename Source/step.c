/**
 * @file step.c
 *
 * @brief Count steps using accelerometer data.
 *
 * @author Yadong Song, Guanqun Zhang, Ziyi Liu
 *
 * @version v2.2
 *
 * @copyright Lifesense 2020-.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <float.h>
#include "step.h"
#include "nonwalk_check.h"
#include "utils.h"

#if (defined(__CC_ARM))
#include <arm_math.h>
#else
#include <math.h>
#endif

#define ACC_RANGE     4                                                 /* acceleration range, i.e. +/-2g */
#define ADC_BITS      8                                                 /* ADC resolution */
#define AVG_WIN       4                                                 /* moving-average window */
#define MIN_ADIFF     (0.6758f * (1 << ADC_BITS)) / (FS * ACC_RANGE)    /* min average amplitude change between adjacent points in one axis */
#define ADIFF_TH      AXIS_NUM * MIN_ADIFF * MIN_ADIFF                  /* amplitude change threshold of square sum acc data */
#define MIN_P2V_TIME  3                                                 /* min time between adjacent peak and valley: (0.12f * FS) */
#define MAX_P2V_TIME  (2 * FS)                                          /* max time between adjacent peak and valley */
#define SILENCE_TH    (1 * MAX_P2V_TIME)                                /* silence time interval threshold */
#define DATA_BLOCK    (FS)                                              /* < SILENCE_TH, data block length */
#define STARTUPPVNUM  25                                                /* min candidate peak and valley number for start up check */
#define CANDPVNUM     30                                                /* memory depth for candidate peaks and valleys: STARTUPPVNUM + (DATA_BLOCK / MIN_P2V_TIME) */
#define MIN_P2V_AMP   MIN_P2V_TIME * ADIFF_TH                           /* min amplitude between adjacent peak and valley */
#define RFT_VAR_TH    (0.55f)                                           /* threshold for rising and falling time interval variation */
#define ALPHA         (0.25f)                                           /* updating coefficient of time */
#define BETA          (0.125f)                                          /* updating coefficient of amplitude */

// #define EXPORT_STEP_DEBUG

/* Static variables: Do NOT use global vars since they can be seen externally */

/* moving-average filter for raw acc data */
#define sum1 v1
#define reg1 v2
#define pos1 v3
static float sum1[AXIS_NUM];                /* summation result */
static float reg1[AVG_WIN * AXIS_NUM];      /* moving-average register */
static uint8_t pos1[AXIS_NUM];              /* last data position in register */

/* moving-average filter for square sum acc data */
#define sum2 v4
#define reg2 v5
#define pos2 v6
static float sum2;
static float reg2[AVG_WIN];
static uint8_t pos2;

#define nfill v7
#define acc_dat_flt v8
#define acc_sqsm_flt v9
#define not_moving_buffer v10
#define steps_last_second v11
#define mag_8s_buffer v12
#define acc_2s_buffer v13

static uint8_t nfill;                              /* number of filled data */
static float acc_dat_flt[DATA_BLOCK * AXIS_NUM];   /* filtered acc data */
static float acc_sqsm_flt[DATA_BLOCK];             /* filtered acc square sum */
static int8_t not_moving_buffer[8];
static uint8_t steps_last_second;
static float mag_8s_buffer[8 * DATA_BLOCK];
static float acc_2s_buffer[AXIS_NUM][2 * DATA_BLOCK];

#define mag_valley_value v14
#define mag_valley_index v15

static float mag_valley_value;
static int32_t mag_valley_index;

#define acc_valley_value v16
#define acc_valley_index v17

static float acc_valley_value[AXIS_NUM];
static int32_t acc_valley_index[AXIS_NUM];

#define tick v18
#define tick_index v19
#define x_buf v20
#define y_buf v21
#define z_buf v22
#define consecutive_walk_counter v23
#define consecutive_stop_counter v24

static int32_t tick;
static int32_t tick_index;
static int16_t x_buf[DATA_BLOCK];
static int16_t y_buf[DATA_BLOCK];
static int16_t z_buf[DATA_BLOCK];
static int32_t consecutive_walk_counter;
static int32_t consecutive_stop_counter;

struct bpv
{
    bool type;      /* true -> peak, false -> valley */
    int pos;        /* sampling index */
    float val;      /* amplitude */
};
typedef struct
{
    bool npv;       /* effective number of bpv */
    struct bpv pv;
} spv_t;
#define lastpv v25
static spv_t lastpv;

typedef struct
{
    int32_t npv;
    struct bpv pv[CANDPVNUM]; /* bpv array */
} mpv_t;

#define mag_candpv v26
#define acc_candpv v27

static mpv_t mag_candpv;
static mpv_t acc_candpv[AXIS_NUM];

#define avg_rtime v28
#define avg_ftime v29
#define avg_ramp v30
#define avg_famp v31
#define is_walking v32

static float avg_rtime;   /* average rising time */
static float avg_ftime;   /* average falling time */
static float avg_ramp;    /* average rising amplitude */
static float avg_famp;    /* average falling amplitude */
static bool  is_walking;  /* is walking or not */

#define make_bpv _step_fm
struct bpv make_bpv(bool type, int pos, float val)
{
    struct bpv temp;
    temp.type = type;
    temp.pos = pos;
    temp.val = val;
    return temp;
}

#define step_counter v33
static int32_t step_counter;
void Step_Init(void)
{
    tick = 0;
    tick_index = 0;
    steps_last_second = 0;
    // steps_last2_second = 0;
    sum2 = 0;
    pos2 = 0;
    nfill = 0;

    memset(sum1, 0, sizeof(sum1));
    memset(pos1, 0, sizeof(pos1));
    memset(reg1, 0, sizeof(reg1));
    memset(reg2, 0, sizeof(reg2));
    memset(acc_dat_flt, 0, sizeof(acc_dat_flt));
    memset(acc_sqsm_flt, 0, sizeof(acc_sqsm_flt));

    lastpv.npv = false;
    lastpv.pv = make_bpv(false, 0, 0);
    mag_candpv.npv = 0;
    acc_candpv[0].npv = 0;
    acc_candpv[1].npv = 0;
    acc_candpv[2].npv = 0;

    for (int i = 0; i < CANDPVNUM; i++)
    {
        mag_candpv.pv[i] = lastpv.pv;
        acc_candpv[0].pv[i] = lastpv.pv;
        acc_candpv[1].pv[i] = lastpv.pv;
        acc_candpv[2].pv[i] = lastpv.pv;
    }
    avg_rtime = MIN_P2V_TIME;
    avg_ftime = MIN_P2V_TIME;
    avg_ramp = MIN_P2V_AMP;
    avg_famp = MIN_P2V_AMP;
    is_walking = false;

    step_counter = 0;
    consecutive_walk_counter = 0;
    consecutive_stop_counter = 0;

    // Init
    NonWalkCheck(x_buf, y_buf, z_buf, DATA_BLOCK, false, true);

    memset(not_moving_buffer, 0, sizeof(not_moving_buffer));
    memset(mag_8s_buffer, 0, sizeof(mag_8s_buffer));
    memset(acc_2s_buffer, 0, sizeof(acc_2s_buffer));

    mag_valley_value = FLT_MAX;
    mag_valley_index = -1;

    for (int i = 0; i < AXIS_NUM; i++)
    {
        acc_valley_value[i] = FLT_MAX;
        acc_valley_index[i] = -1;
    }

    memset(x_buf, 0, sizeof(x_buf));
    memset(y_buf, 0, sizeof(y_buf));
    memset(z_buf, 0, sizeof(z_buf));
}

#define _calc_rftamp _step_fr
static void _calc_rftamp(struct bpv latest_pv, mpv_t candpv, float *rt, \
    float *ramp, uint8_t *nr, float *ft, float *famp, uint8_t *nf)
{
    for (int i = 0; i < candpv.npv; i++)
    {
        if (candpv.pv[i].type != latest_pv.type)
        {
            if (candpv.pv[i].type)
            {
                rt[*nr] = (float)(candpv.pv[i].pos - latest_pv.pos);
                ramp[*nr] = candpv.pv[i].val - latest_pv.val;
                *nr = *nr + 1;
            }
            else
            {
                ft[*nf] = (float)(candpv.pv[i].pos - latest_pv.pos);
                famp[*nf] = latest_pv.val - candpv.pv[i].val;
                *nf = *nf + 1;
            }
        }
        latest_pv = candpv.pv[i];
    }
}

#define _kurtosis _step_fk
static float _kurtosis(const float *array, int16_t L)
{
    int16_t i;
    float m = 0;
    float s = 0;

    // Calculate mean of the array.
    for (i = 0; i < L; i++)
    {
        s += array[i];
    }

    m = s / L;

    float num = 0;
    float den = 0;

    for (i = 0; i < L; i++)
    {
        float diff = array[i] - m;
        float diff_sqrd = diff * diff;
        float diff_quad = diff_sqrd * diff_sqrd;

        num += diff_quad;
        den += diff_sqrd;
    }

    if (den != 0)
    {
        return L * num / (den * den);
    }
    else
    {
        return 0;
    }
}

#define _AutoCorrelation _step_fa
static float _AutoCorrelation(float* data, uint16_t data_length, uint8_t lag) {
    float sum = .0;
    const float* p = &data[lag];
    uint16_t i = (uint16_t)lag;
    while (i++ < data_length) {
        float sum2p = (*p) * (*(p - lag));
        sum += sum2p;
        p++;
    }

    return sum;
}

#define _find_peaks _step_ff
static bool _find_peaks(const float *array, int32_t i, int16_t radius, int32_t minPH, int32_t maxPH)
{
    float x = array[i];

    if (x < minPH || x > maxPH)
    {
        return false;
    }

    // Detecting local maxima
    for (int16_t j = 1; j <= radius; j++)
    {
        // Local maxima
        if (x >= array[i + j] && x >= array[i - j])
        {
            continue;
        }
        else
        {
            return false;
        }
    }

    return true;
}

uint8_t Step_Detection(const int16_t acc[3])
{
    /* TODO: move to firmware. */
    int16_t accx = (acc[0] >> 6) >> 0;
    int16_t accy = (acc[1] >> 6) >> 0;
    int16_t accz = (acc[2] >> 6) >> 0;

    /* Load the last 6-bit to get the sync signal. */
    /* Q: the meaning of sync_exist? */
    bool sync_exist = false;
    if ( ((acc[0] & 0x3F) >= 1)
        && ((acc[0] & 0x3F) <= 25)
        && ((acc[0] & 0x3F) == (acc[1] & 0x3F))
        && ((acc[0] & 0x3F) == (acc[2] & 0x3F)) )
    {
        sync_exist = true;
        nfill = (acc[0] & 0x3F) - 1;
    }

    x_buf[nfill] = accx;
    y_buf[nfill] = accy;
    z_buf[nfill] = accz;

    /* 4-pt moving-average on raw accelerometer data */
    acc_dat_flt[nfill] = moving_avg(accx, &sum1[0], &reg1[0], &pos1[0], AVG_WIN);
    acc_dat_flt[nfill + DATA_BLOCK] = moving_avg(accy, &sum1[1], &reg1[AVG_WIN], &pos1[1], AVG_WIN);
    acc_dat_flt[nfill + 2 * DATA_BLOCK] = moving_avg(accz, &sum1[2], &reg1[2 * AVG_WIN], &pos1[2], AVG_WIN);

    // Does not exist sync signal
    if (!sync_exist)
    {
        nfill++;
    }
    step_counter++;

    /* walk detection */
    uint8_t steps = 0;

    // Every second
    float activity = 0;
    if (nfill == DATA_BLOCK - sync_exist)
    {
        // float activity_3axis[AXIS_NUM];
        // memset(activity_3axis, 0, sizeof(activity_3axis));

        bool step_boosted = false;
        /* check activity */
        for (int i = 0; i < AXIS_NUM; i++)
        {
            for (int j = 0; j < DATA_BLOCK - 1; j++)
            {
                activity += fabsf(acc_dat_flt[j + 1 + i * DATA_BLOCK] - acc_dat_flt[j + i * DATA_BLOCK]);
                // activity_3axis[i] += fabsf(acc_dat_flt[j + 1 + i * DATA_BLOCK] - acc_dat_flt[j + i * DATA_BLOCK]);
            }
        }

        /* compute square sum */
        float acc_sqsm[DATA_BLOCK];
        memset(acc_sqsm, 0, sizeof(acc_sqsm));
        for (int i = 0; i < DATA_BLOCK; i++)
        {
            for (int j = 0; j < AXIS_NUM; j++)
            {
                acc_sqsm[i] += (acc_dat_flt[i + j * DATA_BLOCK]) * (acc_dat_flt[i + j * DATA_BLOCK]);
            }
        }

        int8_t non_moving_counter = 0;
        for (int i = 0; i < DATA_BLOCK; i++)
        {
            acc_sqsm_flt[i] = moving_avg(acc_sqsm[i], &sum2, reg2, &pos2, AVG_WIN);
            if (acc_sqsm_flt[i] > 3850 && acc_sqsm_flt[i] < 4250 && fabsf(acc_sqsm_flt[i] - acc_sqsm_flt[i - 1]) < 30)
            {
                non_moving_counter++;
            }
        }

        memcpy(not_moving_buffer, &not_moving_buffer[1], 7 * sizeof(not_moving_buffer[0]));
        not_moving_buffer[7] = non_moving_counter;
        int16_t not_moving_sum = 0;
        for (int i = 0; i < 8; i++)
        {
            not_moving_sum += not_moving_buffer[i];
        }

        // To save power. Do not check walking if not moving during the previous 8 seconds
        if (not_moving_sum > 165 && not_moving_buffer[7] >= 20)
        {
            is_walking = false;
        }
        else
        {
            // Set
            NonWalkCheck(x_buf, y_buf, z_buf, DATA_BLOCK, false, false);

            // Get
            bool not_walk_model = NonWalkCheck(x_buf, y_buf, z_buf, DATA_BLOCK, true, false);
            printf("not_walk_model: %d\n", not_walk_model);

            // Check consecutive walking or non-walking
            if (!is_walking)
            {
                /* If Zhang has detected multiple is_walking=false, he may want
                 * to know whether my model has the same results. If different,
                 * consecutive_walk_counter that control counting decision would
                 * depend on my model.
                 */
                if (!not_walk_model)
                {
                    consecutive_walk_counter++;
                }
                else
                {
                    consecutive_walk_counter = 0;
                }
                consecutive_stop_counter = 0;
            }
            else
            {
                if (not_walk_model)
                {
                    consecutive_stop_counter++;
                }
                else
                {
                    consecutive_stop_counter = 0;
                }
                /* Q: if both the last and current is_walking are
                 * true, consecutive_walk_counter changes to 0? */
                consecutive_walk_counter = 0;
            }

            /* Update mag_8s_buffer. */
            memcpy(mag_8s_buffer, &mag_8s_buffer[DATA_BLOCK], 7 * DATA_BLOCK * sizeof(mag_8s_buffer[0]));
            memcpy(&mag_8s_buffer[7 * DATA_BLOCK], acc_sqsm_flt, DATA_BLOCK * sizeof(mag_8s_buffer[0]));

            /* Update acc_2s_buffer. */
            for (int axis = 0; axis < AXIS_NUM; axis++)
            {
                memcpy(acc_2s_buffer[axis], &acc_2s_buffer[axis][DATA_BLOCK], DATA_BLOCK * sizeof(acc_2s_buffer[axis][0]));
                memcpy(&acc_2s_buffer[axis][DATA_BLOCK], &acc_dat_flt[axis * DATA_BLOCK], DATA_BLOCK * sizeof(acc_2s_buffer[axis][0]));
            }

            uint8_t mag_nr = 0;
            uint8_t mag_nf = 0;
            float mag_rt[STARTUPPVNUM + 2];
            float mag_ft[STARTUPPVNUM + 2];
            float mag_ramp[STARTUPPVNUM + 2];
            float mag_famp[STARTUPPVNUM + 2];

            memset(mag_rt, 0, sizeof(mag_rt));
            memset(mag_ft, 0, sizeof(mag_ft));
            memset(mag_ramp, 0, sizeof(mag_ramp));
            memset(mag_famp, 0, sizeof(mag_famp));

            uint8_t acc_nr[AXIS_NUM];
            uint8_t acc_nf[AXIS_NUM];
            float acc_rt[AXIS_NUM][STARTUPPVNUM + 2];
            float acc_ft[AXIS_NUM][STARTUPPVNUM + 2];
            float acc_ramp[AXIS_NUM][STARTUPPVNUM + 2];
            float acc_famp[AXIS_NUM][STARTUPPVNUM + 2];

            memset(acc_nr, 0, sizeof(acc_nr));
            memset(acc_nf, 0, sizeof(acc_nf));
            memset(acc_rt, 0, sizeof(acc_rt));
            memset(acc_ft, 0, sizeof(acc_ft));
            memset(acc_ramp, 0, sizeof(acc_ramp));
            memset(acc_famp, 0, sizeof(acc_famp));

            /* Magnitude: Scan the buffer to detect peaks and valleys. */
            for (int i = 4; i < DATA_BLOCK + 4; i++) /* Q: Because the radius for detecting peak is 4? */
            {
                float mag_peak_value = mag_8s_buffer[i + 6 * DATA_BLOCK];
                /* Continuously assign the min value to mag_valley_value. */
                if (mag_valley_value > mag_peak_value)
                {
                    mag_valley_value = mag_peak_value;
                    /*<< Q: (step_counter - 50) means start of all the index is from 2s? */
                    mag_valley_index = (step_counter - 50) + i + 1;
                    if (mag_valley_index < 0)
                    {
                        mag_valley_index = 1;
                    }
                }

                // radius = 4 @ 25 Hz
                bool is_peak = _find_peaks(&mag_8s_buffer[6 * DATA_BLOCK], i, 4, 3800, 50000) && (activity >= 35) && (mag_peak_value - mag_valley_value > 100);
                if (is_peak)
                {
                    int32_t mag_peak_index = (step_counter - 50) + i + 1;
                    if (mag_peak_index - mag_valley_index < FS)
                    {
                        if (mag_candpv.npv > 0)
                        {
                            /* If speed is not high, gap between peak and valley
                             * values must >= 250. If high (peak and valley pos-
                             * -itions <=7), gap must >= 500.
                             */
                            if (mag_peak_value - mag_valley_value < 250 || (mag_peak_index - mag_candpv.pv[mag_candpv.npv - 1].pos <= 7 && mag_peak_value - mag_valley_value < 500)) /* Q: Noise peak ? */
                            {
                                continue;
                            }
                        }

                        if (mag_candpv.npv > CANDPVNUM - 2)
                        {
                            for (int j = 0; j < mag_candpv.npv - 2; j++)
                            {
                                mag_candpv.pv[j] = mag_candpv.pv[j + 2];
                            }
                            mag_candpv.npv -= 2;
                        }

                        mag_candpv.pv[mag_candpv.npv] = make_bpv(false, mag_valley_index, mag_valley_value);
#ifdef EXPORT_STEP_DEBUG
                        printf("%d, %d, %f, %d\n", step_counter, mag_valley_index, mag_valley_value, false);
#endif

                        mag_candpv.pv[mag_candpv.npv + 1] = make_bpv(true, mag_peak_index, mag_peak_value);
#ifdef EXPORT_STEP_DEBUG
                        printf("%d, %d, %f, %d\n", step_counter, mag_peak_index, mag_peak_value, true);
#endif

                        mag_candpv.npv += 2;

                    }

                    /* Reset valley to search new peak and valley pair. */
                    mag_valley_value = FLT_MAX;
                }
            }

            int32_t acc_peak_index[AXIS_NUM];
            for (int axis = 0; axis < AXIS_NUM; axis++)
            {
                for (int i = 4; i < DATA_BLOCK + 4; i++)
                {
                    float acc_peak_value = acc_2s_buffer[axis][i];
                    /* Continuously assign the min value to acc_valley_value[axis]. */
                    if (acc_valley_value[axis] > acc_peak_value)
                    {
                        acc_valley_value[axis] = acc_peak_value;
                        acc_valley_index[axis] = (step_counter - 50) + i + 1;
                        if (acc_valley_index[axis] < 0)
                        {
                            acc_valley_index[axis] = 1;
                        }
                    }

                    bool is_peak = _find_peaks(acc_2s_buffer[axis], i, 4, -128, 128) && (activity >= 35) && (acc_peak_value - acc_valley_value[axis] >= 5) && (acc_peak_value - acc_valley_value[axis] < 5000);
                    if (is_peak)
                    {
                        acc_peak_index[axis] = (step_counter - 50) + i + 1;
                        if (acc_peak_index[axis] - acc_valley_index[axis] < FS)
                        {
                            if (acc_candpv[axis].npv > CANDPVNUM - 2)
                            {
                                for (int j = 0; j < acc_candpv[axis].npv - 2; j++)
                                {
                                    acc_candpv[axis].pv[j] = acc_candpv[axis].pv[j + 2];
                                }
                                acc_candpv[axis].npv -= 2;
                            }
                            acc_candpv[axis].pv[acc_candpv[axis].npv] = make_bpv(false, acc_valley_index[axis], acc_valley_value[axis]);
#ifdef EXPORT_STEP_DEBUG
                            printf("%d, %d, %f, %d\n", step_counter, acc_valley_index[axis], acc_valley_value[axis], false + axis * 2 + 4);
#endif
                            acc_candpv[axis].pv[acc_candpv[axis].npv + 1] = make_bpv(true, acc_peak_index[axis], acc_peak_value);
#ifdef EXPORT_STEP_DEBUG
                            printf("%d, %d, %f, %d\n", step_counter, acc_peak_index[axis], acc_peak_value, true + axis * 2 + 4);
#endif
                            acc_candpv[axis].npv += 2;

                        }
                        acc_valley_value[axis] = FLT_MAX;
                    }
                }
            }

            // Do not calculate steps for the first 2 seconds
            if (step_counter >= 2 * FS)
            {
                /* calculate steps */
                if (mag_candpv.npv > 0)
                {
                    float x_min = acc_dat_flt[0];
                    float x_max = acc_dat_flt[0];

                    float y_min = acc_dat_flt[DATA_BLOCK];
                    float y_max = acc_dat_flt[DATA_BLOCK];

                    float z_min = acc_dat_flt[DATA_BLOCK * 2];
                    float z_max = acc_dat_flt[DATA_BLOCK * 2];

                    for (int32_t i = 1; i < DATA_BLOCK; i++)
                    {
                        // X
                        if (x_min > acc_dat_flt[i])
                        {
                            x_min = acc_dat_flt[i];
                        }

                        if (x_max < acc_dat_flt[i])
                        {
                            x_max = acc_dat_flt[i];
                        }

                        // Y
                        if (y_min > acc_dat_flt[i + DATA_BLOCK])
                        {
                            y_min = acc_dat_flt[i + DATA_BLOCK];
                        }

                        if (y_max < acc_dat_flt[i + DATA_BLOCK])
                        {
                            y_max = acc_dat_flt[i + DATA_BLOCK];
                        }

                        // Z
                        if (z_min > acc_dat_flt[i + 2 * DATA_BLOCK])
                        {
                            z_min = acc_dat_flt[i + 2 * DATA_BLOCK];
                        }

                        if (z_max < acc_dat_flt[i + 2 * DATA_BLOCK])
                        {
                            z_max = acc_dat_flt[i + 2 * DATA_BLOCK];
                        }
                    }

                    float x_avg = 0;
                    float y_avg = 0;
                    float z_avg = 0;
                    arm_mean_f32(acc_dat_flt, DATA_BLOCK, &x_avg);
                    arm_mean_f32(&acc_dat_flt[DATA_BLOCK], DATA_BLOCK, &y_avg);
                    arm_mean_f32(&acc_dat_flt[2 * DATA_BLOCK], DATA_BLOCK, &z_avg);

                    /* remove silence period */
                    if (!is_walking)
                    {
                        // Not walking
                        // Magnitude
                        int k = -1;
                        for (int j = 0; j < mag_candpv.npv - 1; j++)
                        {
                            if (mag_candpv.pv[j + 1].pos - mag_candpv.pv[j].pos >= SILENCE_TH)
                            {
                                k = j;
                            }
                        }
                        if (k >= 0)
                        {
                            for (int j = 0; j < mag_candpv.npv - k - 1; j++)
                            {
                                mag_candpv.pv[j] = mag_candpv.pv[j + k + 1];
                            }
                            mag_candpv.npv = mag_candpv.npv - k - 1;
                        }

                        // ACC
                        for (int axis = 0; axis < AXIS_NUM; axis++)
                        {
                            k = -1;
                            for (int j = 0; j < acc_candpv[axis].npv - 1; j++)
                            {
                                if (acc_candpv[axis].pv[j + 1].pos - acc_candpv[axis].pv[j].pos >= SILENCE_TH)
                                {
                                    k = j;
                                }
                            }
                            if (k >= 0)
                            {
                                for (int j = 0; j < acc_candpv[axis].npv - k - 1; j++)
                                {
                                    acc_candpv[axis].pv[j] = acc_candpv[axis].pv[j + k + 1];
                                }
                                acc_candpv[axis].npv = acc_candpv[axis].npv - k - 1;
                            }
                        }

                        /* check time interval variance */
                        while (mag_candpv.npv >= STARTUPPVNUM)
                        {
                            int32_t n_peak = 0;
                            for (int i = 0; i < mag_candpv.npv; i++)
                            {
                                if (mag_candpv.pv[i].type == true)
                                {
                                    n_peak++;
                                }
                            }

                            if (n_peak < 9)
                            {
                                break;
                            }
                            mag_nr = mag_nf = 0;
                            _calc_rftamp(mag_candpv.pv[0], mag_candpv, mag_rt, mag_ramp, &mag_nr, mag_ft, mag_famp, &mag_nf);
                            for (int axis = 0; axis < AXIS_NUM; axis++)
                            {
                                acc_nr[axis] = 0;
                                acc_nf[axis] = 0;
                                _calc_rftamp(acc_candpv[axis].pv[0], acc_candpv[axis], acc_rt[axis], acc_ramp[axis], &acc_nr[axis], acc_ft[axis], acc_famp[axis], &acc_nf[axis]);
                            }

                            if (mag_nr < ((STARTUPPVNUM >> 1) - 1) || mag_nf < ((STARTUPPVNUM >> 1) - 1))
                            {
                                for (int i = 0; i < mag_candpv.npv - 2; i++)
                                {
                                    mag_candpv.pv[i] = mag_candpv.pv[i + 2];
                                }
                                mag_candpv.npv -= 2;
                                break;
                            }
                            else
                            {
                                // Magnitude
                                float mag_rt_mean = 0;
                                float mag_ft_mean = 0;
                                arm_mean_f32(mag_rt, mag_nr, &mag_rt_mean);
                                arm_mean_f32(mag_ft, mag_nf, &mag_ft_mean);

                                float rt_var = 0;
                                arm_var_f32(mag_rt, mag_nr, &rt_var);
                                float ft_var = 0;
                                arm_var_f32(mag_ft, mag_nf, &ft_var);

                                float mag_ramp_mean = 0;
                                float mag_famp_mean = 0;
                                arm_mean_f32(mag_ramp, mag_nr, &mag_ramp_mean);
                                arm_mean_f32(mag_famp, mag_nf, &mag_famp_mean);

                                float ramp_var = 0;
                                arm_var_f32(mag_ramp, mag_nr, &ramp_var);

                                // ACC
                                float acc_ft_var[AXIS_NUM];

                                for (int axis = 0; axis < AXIS_NUM; axis++)
                                {
                                    arm_var_f32(acc_ft[axis], acc_nf[axis], &acc_ft_var[axis]);
                                }

                                // bool walk_rule1 = rt_var < (RFT_VAR_TH * mag_rt_mean) * (RFT_VAR_TH * mag_rt_mean);
                                bool walk_rule2 = ft_var < (RFT_VAR_TH * mag_ft_mean) * (RFT_VAR_TH * mag_ft_mean);

                                // X needs to shake for true walking.
                                // Otherwise, rotating arms may contribute to steps.
                                bool x_shake = false;

                                // X should be greater than 1/8 G while walking
                                if (x_max - x_min > 8)
                                {
                                    x_shake = true;
                                }

                                bool walking_flag = false;
                                // Kick off walking after 10 rising onsets
                                if (walk_rule2 && x_shake && mag_nr >= 10) // walk_rule1 &&
                                {
                                    // walk_rule4: Do not increase steps when the motor vibrates
                                    bool walk_rule4 = (not_moving_sum > 25) && (mag_ramp_mean + mag_ft_mean > 3500);

                                    // O is walking
                                    // bool tf7 = (activity_3axis[0] > 45 && activity_3axis[1] > 20) || (activity_3axis[0] > 20 && activity_3axis[1] > 45) || (activity_3axis[0] > 20 && activity_3axis[1] > 20 && activity_3axis[2] > 45) || (activity > 80 && activity_3axis[0] < 60 && activity_3axis[1] < 40 && activity_3axis[2] < 40);

                                    float ramp_std = 0;
#if (defined(__CC_ARM))
                                    arm_sqrt_f32(ramp_var, &ramp_std);
#else
                                    ramp_std = sqrtf(ramp_var);
#endif
                                    float cv_ramp = ramp_std / mag_ramp_mean;
                                    if (!walk_rule4 && cv_ramp < 0.79f)// && tf7)
                                    {
                                        if (_kurtosis(&mag_8s_buffer[6 * DATA_BLOCK], 2 * DATA_BLOCK) < 2.9f)
                                        {
                                            // if (ft_var <= 16 || rt_var <= 16)
                                            {
                                                walking_flag = true;
                                            }
                                        }
                                    }
                                }

                                if (walking_flag)
                                {
                                    if (acc_ft_var[0] > 250 || acc_ft_var[1] > 350)
                                    {
                                        walking_flag = false;
                                    }
                                    else if (z_avg < 32 && z_avg > -32 && (x_avg < -45 || y_avg > -45 || (x_avg < -25 && x_avg > -30 && y_avg < -50 && y_avg > -55) || (x_avg < -30 && x_avg > -35 && y_avg < -45 && y_avg > -51 && fabsf(z_avg) < 15 && rt_var < 3.5f)) && (y_max - y_min) > 5)
                                    {
                                        if (mag_candpv.npv > (acc_candpv[0].npv + acc_candpv[1].npv + acc_candpv[2].npv) * 2)
                                        {
                                            walking_flag = false;
                                        }
                                        else
                                        {
                                            walking_flag = true;
                                        }
                                    }
                                    else if (z_avg < -45 && x_avg > 20 && x_avg < 40 && y_avg < -32)
                                    {
                                        walking_flag = true;
                                    }
                                    else if (x_avg > 32.5f && x_avg < 37.5f && y_avg > -45 && y_avg < -42.5f && z_avg > -35 && z_avg < -32.5f)
                                    {
                                        walking_flag = true;
                                    }
                                    else
                                    {
                                        // printf("%d, %f, %f, %f\n", step_counter, x_avg, y_avg, z_avg);
                                        walking_flag = false;

                                        // Auto-correlation
                                        float auto_corr[8 * DATA_BLOCK];
                                        memset(auto_corr, 0, sizeof(auto_corr));
                                        for (int i = 0; i < 8 * DATA_BLOCK; i++)
                                        {
                                            auto_corr[i] = _AutoCorrelation(mag_8s_buffer, 8 * DATA_BLOCK, i);
                                        }

                                        float v0 = auto_corr[0];
                                        for (int i = 0; i < 8 * DATA_BLOCK; i++)
                                        {
                                            auto_corr[i] /= v0;
                                        }

                                        // Must find a peak in one second
                                        for (int i = 4; i < FS; i++)
                                        {
                                            bool is_peak = _find_peaks(auto_corr, i, 4, -1, 1);
                                            if (is_peak)
                                            {
                                                walking_flag = true;
                                                break;
                                            }
                                        }
                                    }
                                }

                                // At least 5 seconds for checking walking, but skip the upholding phone position for now
                                if (walking_flag && (consecutive_walk_counter >= 5 || (x_avg > 57.7f && x_avg < 64 && y_avg > -20 && y_avg < 12.5f && z_avg > 17.5f && z_avg < 25)))
                                {
                                    {
                                        is_walking = true;
                                        steps = mag_nr;
#ifdef EXPORT_STEP_DEBUG
                                        for (int i = 0; i < mag_nr; i++)
                                        {
                                            printf("%f, 0, %d, 2\n", mag_ramp[i], step_counter);
                                        }

                                        for (int i = 0; i < mag_nf; i++)
                                        {
                                            printf("%f, 1, %d, 3\n", mag_famp[i], step_counter);
                                        }
#endif
                                    }
                                    break;
                                }
                                else
                                {
                                    for (int i = 0; i < mag_candpv.npv - 2; i++)
                                    {
                                        mag_candpv.pv[i] = mag_candpv.pv[i + 2];
                                    }
                                    mag_candpv.npv = mag_candpv.npv - 2;
                                }
                            }
                        }

                        avg_rtime = MIN_P2V_TIME;
                        avg_ftime = MIN_P2V_TIME;
                        avg_ramp = MIN_P2V_AMP;
                        avg_famp = MIN_P2V_AMP;
                    }
                    else
                    {
                        // Walking
                        mag_nr = mag_nf = 0;
                        _calc_rftamp(lastpv.pv, mag_candpv, mag_rt, mag_ramp, &mag_nr, mag_ft, mag_famp, &mag_nf);
                        for (int axis = 0; axis < AXIS_NUM; axis++)
                        {
                            acc_nr[axis] = 0;
                            acc_nf[axis] = 0;
                            _calc_rftamp(lastpv.pv, acc_candpv[axis], acc_rt[axis], acc_ramp[axis], &acc_nr[axis], acc_ft[axis], acc_famp[axis], &acc_nf[axis]);
                        }

                        bool stop_rule1 = false;
                        if (mag_nr > 1)
                        {
                            float rt_max = mag_rt[0];
                            for (int i = 1; i < mag_nr; i++)
                            {
                                if (rt_max < mag_rt[i])
                                {
                                    rt_max = mag_rt[i];
                                }
                            }
                            stop_rule1 = rt_max > (4 * avg_rtime);
                        }
                        bool stop_rule2 = false;
                        if (mag_nf > 0)
                        {
                            float ft_max = mag_ft[0];
                            for (int i = 1; i < mag_nf; i++)
                            {
                                if (ft_max < mag_ft[i])
                                {
                                    ft_max = mag_ft[i];
                                }
                            }
                            stop_rule2 = (ft_max > (4 * avg_ftime)) || (mag_nf == 1);
                        }

                        // Stop the walking after 3-second confirmations
                        bool stop_rule3 = (consecutive_stop_counter > 2);
                        if (stop_rule3)
                        {
                            if (fabsf(x_avg + 64) <= 10)
                            {
                                // Lifting
                                stop_rule3 = false;
                            }
                            else if (x_avg > 50 && x_avg < 66 && y_avg > -30 && y_avg < 15 && z_avg > 0 && z_avg < 30 && activity > 30 && activity < 70)
                            {
                                // Indoor holding phone near the ear
                                stop_rule3 = false;
                            }
                            else if (x_avg > 27.5f && x_avg < 42.5f && y_avg > -50 && y_avg < -32.5f && z_avg > -42.5f && z_avg < -27.5f && activity > 35 && activity < 70)
                            {
                                // Indoor holding phone walking
                                stop_rule3 = false;
                            }
                        }

                        float mag_rt_mean = 0;
                        float mag_ft_mean = 0;
                        arm_mean_f32(mag_rt, mag_nr, &mag_rt_mean);
                        arm_mean_f32(mag_ft, mag_nf, &mag_ft_mean);

                        float mag_ramp_mean = 0;
                        float mag_famp_mean = 0;
                        arm_mean_f32(mag_ramp, mag_nr, &mag_ramp_mean);
                        arm_mean_f32(mag_famp, mag_nf, &mag_famp_mean);

                        // Palm down
                        // bool stop_rule4 = false;
                        // if ((mag_rt_mean + mag_ft_mean < 9.8f) && (mag_ramp_mean + mag_famp_mean < 1850))
                        // {
                        //     if (fabsf(fabsf(z_avg) - 64) < 9)
                        //     {
                        //         stop_rule4 = true;
                        //     }
                        //     else
                        //     {
                        //         stop_rule4 = false;
                        //     }
                        // }

                        // Rules for quitting walking
                        if ((stop_rule1 && stop_rule2) || stop_rule3)// || stop_rule4)
                        {
                            is_walking = false;
                        }
                        else
                        {
                            /* update avg_rtime, avg_ftime, avg_ramp and avg_famp */
                            if (mag_nr > 0)
                            {
                                steps = mag_nr;
                                avg_rtime = avg_rtime + ALPHA * (mag_rt_mean - avg_rtime);
                                avg_ramp = avg_ramp + BETA * (mag_ramp_mean - avg_ramp);
                            }
                            else
                            {
                                steps = mag_nf;
                                avg_ftime = avg_ftime + ALPHA * (mag_ft_mean - avg_ftime);
                                avg_famp = avg_famp + BETA * (mag_famp_mean - avg_famp);
                            }

                            if (steps_last_second == 1 && steps == 1 && activity >= 120 && activity <= 250) // steps_last2_second == 1 && 
                            {
                                // printf("Boosted: %d, %f, %d, %d, %d, %d, %d, %f, %f, %f, %f\n", 
                                //     step_counter, activity, mag_nr, mag_nf, acc_nr[0], acc_nr[1], acc_nr[2], 
                                //     mag_rt_mean / mag_ft_mean, x_avg, y_avg, z_avg);
                                if (!not_walk_model && acc_nr[0] <= 2 && acc_nr[1] <= 2 && acc_nr[2] <= 2)
                                {
                                    if (mag_rt_mean / mag_ft_mean >= 1.35f && mag_rt_mean / mag_ft_mean < 3.0f)
                                    {
                                        if (x_avg > -60 && x_avg < -37.5f && y_avg > -45 && y_avg < -25 && z_avg > -22.5f && z_avg < 20)
                                        {
                                            // printf("cond1: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -27.5f && x_avg < 15 && y_avg > -67 && y_avg < -45 && z_avg > -30 && z_avg < 0)
                                        {
                                            // printf("cond2: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -40 && x_avg < -15 && y_avg > -60 && y_avg < -40 && z_avg > -40 && z_avg < -20)
                                        {
                                            // printf("cond3: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > 10 && x_avg < 40 &&  y_avg > -52 && y_avg < -35 && ((z_avg > -47 && z_avg < -25) || (z_avg > -57.5f && z_avg < -52.5f)))
                                        {
                                            // printf("cond4: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -5 && x_avg < 10 && y_avg > -35 && y_avg < -15 && z_avg > -65 && z_avg < -50)
                                        {
                                            // printf("cond5: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -67.5f  && x_avg < -58 && y_avg > -9 && y_avg < 7.5f && z_avg > -25 && z_avg < 17.5f && fabsf(z_avg) >= 2)
                                        {
                                            // printf("cond6: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -25 && x_avg < -15 && y_avg > -65 && y_avg < -55 && z_avg > 0 && z_avg < 20)
                                        {
                                            // printf("cond7: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > 5 && x_avg < 10 && y_avg > -55 && y_avg < -50 && z_avg > -10 && z_avg < 0)
                                        {
                                            // printf("cond8: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > 60 && x_avg < 75 && y_avg > -30 && y_avg < -5 && z_avg > 0 && z_avg < 20)
                                        {
                                            // printf("cond9: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > 45 && x_avg < 56 && y_avg > -45 && y_avg < -30 && z_avg > -5 && z_avg < 5)
                                        {
                                            // printf("cond10: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -40 && x_avg < -15 && y_avg > -60 && y_avg < -40 && z_avg > 5 && z_avg < 10)
                                        {
                                            // printf("cond11: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -60 && x_avg < -50 && y_avg > -25 && y_avg < -20 && z_avg > -5 && z_avg < 5)
                                        {
                                            // printf("cond12: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -30 && x_avg < -25 && y_avg > -40 && y_avg < -35 && z_avg > 15 && z_avg < 25)
                                        {
                                            // printf("cond13: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -35 && x_avg < -20 && y_avg > -5 && y_avg < 10 && z_avg > -60 && z_avg < -45)
                                        {
                                            // printf("cond14: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > 52.5f && x_avg < 70 && y_avg > -27.5f && y_avg < -20 && z_avg > 0 && z_avg < 20)
                                        {
                                            // printf("cond15: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -57.5f && x_avg < -45 && y_avg > -30 && y_avg < 12.5f && z_avg > 0 && z_avg < 27.5f)
                                        {
                                            // printf("cond16: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -12.5f && x_avg < 7.5f && y_avg > 7.5f && y_avg < 10 && z_avg > 55 && z_avg < 60)
                                        {
                                            // printf("cond17: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -25 && x_avg < -20 && y_avg > -5 && y_avg < 5 && z_avg > 45 && z_avg < 47.5f)
                                        {
                                            // printf("cond18: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -82.5f && x_avg < -72 && y_avg > -12.5f && y_avg < 0 && z_avg > -32.5f && z_avg < 5)
                                        {
                                            // printf("cond19: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else if (x_avg > -20 && x_avg < -12.5f && y_avg > -52.5f && y_avg < -50 && z_avg > -40 && z_avg < -30)
                                        {
                                            // printf("cond20: %f, %f, %f\n", x_avg, y_avg, z_avg);
                                            ;
                                        }
                                        else
                                        {
                                            steps = 3;
                                            // printf("Confirmed\n");
                                            step_boosted = true;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
                else if (is_walking)
                {
                    if (step_counter - tick_index == FS)
                    {
                        tick++;
                    }
                    else
                    {
                        tick = 0;
                    }
                    tick_index = step_counter;

                    if (tick == 6)
                    {
                        // Not walking for small vibrations
                        is_walking = false;
                        tick = 0;
                    }
                }
            }
        }

        /* update lastpv and mag_candpv */
        if (is_walking)
        {
            if (mag_candpv.npv > 0)
            {
                lastpv.npv = true;
                lastpv.pv = mag_candpv.pv[mag_candpv.npv - 1];
                mag_candpv.npv = 0;
                acc_candpv[0].npv = 0;
                acc_candpv[1].npv = 0;
                acc_candpv[2].npv = 0;
            }
        }

        /* reset nfill */
        nfill = 0;

        // steps_last2_second = steps_last_second;
        if (step_boosted)
        {
            steps_last_second = 1;
        }
        else
        {
            steps_last_second = steps;
        }
    }
#ifdef EXPORT_STEP_DEBUG
    if (steps > 0)
    {
        printf("%d, %d, 0, -1\n", step_counter, steps);
    }
#endif
    return steps;
}
