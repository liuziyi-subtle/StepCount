#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "step.h"

#define AXIS_X       0             /* X */
#define AXIS_Y       1             /* Y */
#define AXIS_Z       2             /* Z */
#define MAX_DAT_NUM  FS * 60 * 100 /* max number of samples per axis */

/**
 * Test program for step algorithm
 */
int main(int argc, char *argv[])
{
    FILE *fp;

    if (argc == 1) { /* no args */
        printf("no input file\n");
        return EXIT_FAILURE;
    } else if ((fp = fopen(*++argv, "r")) == NULL) {
        printf("step_test: can't open %s\n", *argv);
        return EXIT_FAILURE;
    }

    char c;
    int i, j;
    int nl = 0;
    while ((c = getc(fp)) != EOF)
    {
        if (c == '\n')
        {
            nl++;
        }
    }
    rewind(fp);
    if (nl > MAX_DAT_NUM)
    {
        nl = MAX_DAT_NUM;
    }

    int *acc_dat[AXIS_NUM];
    for (i = 0; i < AXIS_NUM; i++)
    {
        acc_dat[i] = (int *)malloc(nl * sizeof(int));
    }

    for (j = 0; j < nl; j++)
    {
        for (i = 0; i < AXIS_NUM; i++)
        {
            fscanf(fp, "%d", &acc_dat[i][j]);
        }
    }

    clock_t tstart, tend; 
    tstart = clock();

// #define OFFSET_MODE
    long step_cnt = 0;
    Step_Init();
    for (i = 0; i < nl; i++) {
        int16_t acc[3];
#ifdef OFFSET_MODE        
        acc[0] = acc_dat[AXIS_X][i] - 128;
        acc[1] = acc_dat[AXIS_Y][i] - 128;
        acc[2] = acc_dat[AXIS_Z][i] - 128;
#else
        acc[0] = acc_dat[AXIS_X][i];
        acc[1] = acc_dat[AXIS_Y][i];
        acc[2] = acc_dat[AXIS_Z][i];
#endif
        uint8_t steps = Step_Detection(acc);
        step_cnt = step_cnt + steps;
    }
    tend = clock();
    // printf("%s, steps = %ld, run time = %.2f ms\n", *argv, step_cnt, 1000.0f * (double)(tend - tstart) / CLOCKS_PER_SEC);
    printf("%s, steps = %ld\n", *argv, step_cnt);

    fclose(fp);

    return EXIT_SUCCESS;
}
