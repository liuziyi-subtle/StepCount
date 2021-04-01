#include "debug.h"

#include <stdio.h>

INTERMEDIATES PLOT = {{0}, 0,   {0}, 0,   {0}, 0,   {0}, 0,   {0},
                      0,   {0}, 0,   {0}, 0,   {0}, 0,   {0}, 0};

/* Init*/
void DebugInit() {
  PLOT.num_peaks_accys_length = 0;
  PLOT.maximum_acczs_length = 0;
  PLOT.medain_acczs_length = 0;
  PLOT.ar_1s_length = 0;
  PLOT.agg_autocorrelations_length = 0;
  PLOT.cq_040_080s_length = 0;
  PLOT.probas_length = 0;
  PLOT.preds_length = 0;
  PLOT.results_length = 0;
}