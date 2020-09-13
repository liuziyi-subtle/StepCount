#include "wrapper.h"
#include "nrf_delay.h"
#include "app_timer.h"
#include "app_error.h"
#include "CortexM_StackCheck.h"
#include "nrf_log.h"
#include "test_data.h"

#include "../../c/step.h"


// 通过开启和关闭SWIM_ENABLE，对比MAP文件统计信息，计算算法占用RAM、Code的大小
// 双击工程目录nrf52832_xxaa_s312打开MAP文件，文件最后面是统计信息
#define STEP_ENABLE


//-----------------------用于计算时间间隔-------------------------------------------------
#define APP_TIMER_PRESCALER     0
#define ONE_SECOND_INTERVAL     APP_TIMER_TICKS(1000, APP_TIMER_PRESCALER)
APP_TIMER_DEF(m_one_second_timer_id);
uint16_t algo_time_cnt = 0, time_total_cnt = 0;
float time_sum = 0, time_avg = 0;
float time_max = 0, time_min = 0;
float algo_use_time[20] = {0};
void put_use_time(float time)
{
    if (time_max == 0 && time_min == 0)
    {
        time_max = time;
        time_min = time;
    }
    else if (time > time_max)
    {
        time_max = time;
    }
    else if (time < time_min)
    {
        time_min = time;
    }

    time_total_cnt++;
    time_sum += time;
    algo_use_time[algo_time_cnt++] = time;

    if (algo_time_cnt >= 20)
    {
        algo_time_cnt = 0;
        time_avg = time_sum / time_total_cnt;
    }
}

static void one_second_timeout_handler(void * p_context)
{
    UNUSED_PARAMETER(p_context);
}

void application_timers_init(void)
{
    uint32_t err_code;
    err_code = app_timer_create(&m_one_second_timer_id,
        APP_TIMER_MODE_REPEATED,
        one_second_timeout_handler);
    APP_ERROR_CHECK(err_code);
}

void application_timers_start(void)
{
    uint32_t err_code;
    err_code = app_timer_start(m_one_second_timer_id, ONE_SECOND_INTERVAL, NULL);
    APP_ERROR_CHECK(err_code);
}

void application_timers_stop(void)
{
    uint32_t err_code;
    err_code = app_timer_stop(m_one_second_timer_id);
    APP_ERROR_CHECK(err_code);
}

//------------------------------------数据仿真-------------------------------------------------

#define XAXIS_DATA_COUNT (sizeof(const_x_axis_data) / sizeof(const_x_axis_data[0]))
#define YAXIS_DATA_COUNT (sizeof(const_y_axis_data) / sizeof(const_y_axis_data[0]))
#define ZAXIS_DATA_COUNT (sizeof(const_z_axis_data) / sizeof(const_z_axis_data[0]))

uint32_t cnt = 0;
uint32_t t_total = 0;

void program_init(void)
{
    cnt = 0;
    t_total = 0;
    application_timers_init();
    application_timers_start();

    /* Initialization */
    Step_Init();
}
uint32_t total_step = 0;
uint8_t step_handler(void)
{
    uint32_t diff_time, time1 = 0, time2 = 0;

    if (XAXIS_DATA_COUNT != YAXIS_DATA_COUNT || YAXIS_DATA_COUNT != ZAXIS_DATA_COUNT)
    {
        NRF_LOG_PRINTF("data len err\r\n");
        return 1;
    }

    // 把数据放入算法
    if (cnt < XAXIS_DATA_COUNT)
    {
        app_timer_cnt_get(&time1);

#ifdef STEP_ENABLE
        int16_t x = const_x_axis_data[cnt] - 128;
        int16_t y = const_y_axis_data[cnt] - 128;
        int16_t z = const_z_axis_data[cnt] - 128;
        int16_t acc[3] = { x, y, z };
        uint8_t step = Step_Detection(acc);
        total_step += step;
        cnt++;
#endif
        app_timer_cnt_get(&time2);

        if (time2 >= time1)
        {
            diff_time = (time2 - time1) / 32.768f;
            put_use_time(diff_time);
        }

        t_total += diff_time;

        return 0;
    }
    else
    {
        application_timers_stop();

#if ((defined CHECK_RAM_SPACE) && (CHECK_RAM_SPACE == 1))
        CortexM_StackCheck();
#endif
    }

    return 1;
}
