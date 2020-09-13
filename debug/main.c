#include <dirent.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "../nonwalk_check.h"
#include "debug.h"

char target_dirs[5000][1000]; /* 作为局部变量定义在stack会过大. */
char target_filenames[5000][1000];

int main(int argc, char **argv) {
  uint16_t i = 0, j = 0;

#if ALGO_DEBUG == 1
  char path[1000];

  uint16_t p_count =
      ListFilesRecursively(argv[1], ".csv", target_dirs, target_filenames);

  int16_t accx[1000000];
  int16_t accy[1000000];
  int16_t accz[1000000];
  while (i < p_count) {
    snprintf(path, 1000, "%s/%s", target_dirs[i], target_filenames[i]);
    DebugInit(&PLOT);

    uint16_t acc_length = ReadMTKData(path, 2, accx, accy, accz, 1000000);

    NonWalkCheck(NULL, NULL, NULL, 0, 0, 1);
    for (j = 0u; j < acc_length; j += 25) {
      NonWalkCheck(&accx[j], &accy[j], &accz[j], 25, 0, 0);
      uint8_t result = NonWalkCheck(NULL, NULL, NULL, 25, 1, 0);
      PLOT.results[PLOT.results_length++] = result;
    }

    snprintf(path, 1000, "%s/%s", argv[2], target_filenames[i]);
    WriteInt(path, PLOT.results, PLOT.results_length);

    i++;
  }
#else
#include "test_data.c"

  NonWalkCheck(NULL, NULL, NULL, 0, 0, 1);
  for (i = 0u; i < test_length; i += 25) {
    NonWalkCheck(&accx[i], &accy[i], &accz[i], 25, 0, 0);
    if ((i == 100u) || (i == 225u)) {
      uint8_t result = NonWalkCheck(NULL, NULL, NULL, 25, 1, 0);
      printf("result: %u\n", result);
    }
  }

#endif /* ALGO_DEBUG */

  printf("OK!\n");
  return 0;
}