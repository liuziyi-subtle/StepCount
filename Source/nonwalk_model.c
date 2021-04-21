#include "nonwalk_model.h"

#include <math.h>
#include <stdint.h>
#include <stdlib.h>

#define is_categorical _step_n1
static const unsigned char is_categorical[] = {
  0, 0, 0, 0, 0, 0
};

#define threshold _step_n2
static const float threshold[] = {
  -0.5850721f, -0.50200588f, -0.49833483f, -0.40534955f, -0.3740027f, -0.34299359f, 
  -0.34122074f, -0.32576525f, -0.30839628f, -0.21507791f, -0.20882803f, -0.20504041f, 
  -0.17453784f, -0.17073126f, -0.13315438f, -0.11044325f, -0.089135036f, 
  -0.044769451f, -0.028282959f, 0.025735851f, 0.041192606f, 0.066316888f, 
  0.13751197f, 0.13917971f, 0.15429236f, 0.17296818f, 0.18642007f, 0.21540171f, 
  0.22037081f, 0.23109925f, 0.2636233f, 0.35360348f, 0.35969359f, 0.3646175f, 
  0.3659277f, 0.40985084f, 0.45380241f, 0.48944342f, -55.5f, -40.5f, -38.5f, -24.5f, 
  -18.5f, -9.5f, -8.5f, -6.5f, -4.5f, -3.5f, -0.5f, 0.5f, 2.5f, 8.5f, 16.5f, 17.5f, 18.5f, 
  29.5f, 40.5f, 41.5f, 42.5f, 58.5f, 61.5f, 79.5f, 6.5f, 8.5f, 9.5f, 10.5f, 11.5f, 12.5f, 
  13.5f, 14.5f, 15.5f, 16.5f, 17.5f, 18.5f, 19.5f, 20.5f, 21, 21.5f, 22.5f, 23.5f, 24.5f, 
  25.5f, 26.5f, 27.5f, -5.5f, -2.5f, 2, 10.5f, 11.5f, 12.5f, 14.5f, 21.5f, 30.5f, 46.5f, 
  47.5f, 48.5f, 61.5f, 63.5f, 64.5f, 68.5f, 69.5f, 74.5f, 76.5f, 79.5f, 92.5f, 93.5f, 99.5f, 
  0.54415971f, 0.54562306f, 1.0237708f, 1.2463579f, 1.7096124f, 1.7647632f, 
  2.1272111f, 2.3885274f, 2.4717417f, 2.9432557f, 2.9487185f, 3.2574792f, 4.3861537f, 
  4.569849f, 4.6119604f, 4.7269449f, 4.9562626f, 6.002862f, 6.047802f, 6.1991091f, 
  6.3823814f, 7.1516323f, 7.3700247f, 7.429728f, 7.6205454f, 8.2064905f, 8.8825111f, 
  9.1039925f, 11.268926f, 11.527575f, 11.746292f, 13.025462f, 14.216669f, 18.402185f, 
  27.322418f, 33.887848f, 54.43132f, 104.68015f, 568.64966f, 3934.6519f, 5743.4688f, 
  6834.6055f, 7033.8262f, 9211.3398f, 10521.265f, 12609.779f, 13822.158f, 16694.887f, 
  22602.148f, 24037.023f, 27975.973f, 42775.875f, 46222.75f, 47226.672f, 49247.57f, 
  50446.438f, 57763.988f, 65425.207f, 72144.398f, 83866.578f, 84425.344f, 84989.633f, 
  90781.562f, 107661.22f, 140509, 141055.81f, 142104.62f, 176173.38f, 176859.59f, 
  194498.08f, 200891.09f, 202996.22f, 290151.56f, 627924.5f, 640072.12f, 1730398, 
  2482080.2f, 6790573
};

#define th_begin _step_n3
static const int th_begin[] = {
  0, 38, 62, 84, 107, 145, 
};

#define th_len _step_n4
static const int th_len[] = {
  38, 24, 22, 23, 38, 40, 
};

#define quantize _step_n5
static inline int quantize(float val, unsigned fid) {
  const size_t offset = th_begin[fid];
  const float* array = &threshold[offset];
  int len = th_len[fid];
  int low = 0;
  int high = len;
  int mid;
  float mval;
  // It is possible th_begin[i] == [total_num_threshold]. This means that
  // all features i, (i+1), ... are not used for any of the splits in the model.
  // So in this case, just return something
  if (offset == 185 || val < array[0]) {
    return -10;
  }
  while (low + 1 < high) {
    mid = (low + high) / 2;
    mval = array[mid];
    if (val == mval) {
      return mid * 2;
    } else if (val < mval) {
      high = mid;
    } else {
      low = mid;
    }
  }
  if (array[low] == val) {
    return low * 2;
  } else if (high == len) {
    return len * 2;
  } else {
    return low * 2 + 1;
  }
}

float NonWalkModelPredict(union NonWalkModelEntry* data) {
  float sum = 0.0f;
  
  for (int i = 0; i < 6; ++i) {
    if (data[i].missing != -1 && !is_categorical[i]) {
      data[i].qvalue = quantize(data[i].fvalue, i);
    }
  }

if (!(data[5].missing != -1) || (data[5].qvalue < 40)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 24)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 12)) {
        sum += (float)0.59674036503f;
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 2)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 8)) {
            sum += (float)-0.29230770469f;
          } else {
            sum += (float)0.52000004053f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 20)) {
            sum += (float)0.44976815581f;
          } else {
            sum += (float)0.55970805883f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 16)) {
        if (!(data[4].missing != -1) || (data[4].qvalue < 38)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 68)) {
            sum += (float)-0.54899328947f;
          } else {
            sum += (float)0.18620689213f;
          }
        } else {
          if (!(data[3].missing != -1) || (data[3].qvalue < 12)) {
            sum += (float)-0.26511630416f;
          } else {
            sum += (float)0.38507464528f;
          }
        }
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 12)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 30)) {
            sum += (float)0.28073394299f;
          } else {
            sum += (float)-0.25565218925f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 18)) {
            sum += (float)0.39130437374f;
          } else {
            sum += (float)0.57395213842f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 30)) {
      if (!(data[2].missing != -1) || (data[2].qvalue < 22)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 34)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 54)) {
            sum += (float)-0.59159338474f;
          } else {
            sum += (float)-0.46164280176f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 66)) {
            sum += (float)0.2553191781f;
          } else {
            sum += (float)-0.41243699193f;
          }
        }
      } else {
        if (!(data[1].missing != -1) || (data[1].qvalue < 24)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 50)) {
            sum += (float)-0.57991075516f;
          } else {
            sum += (float)0.028571430594f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 54)) {
            sum += (float)-0.48056873679f;
          } else {
            sum += (float)0.34285718203f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 12)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 48)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 38)) {
            sum += (float)-0.52111560106f;
          } else {
            sum += (float)-0.046153850853f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 42)) {
            sum += (float)0.46000000834f;
          } else {
            sum += (float)-0.051428575069f;
          }
        }
      } else {
        if (!(data[0].missing != -1) || (data[0].qvalue < 2)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.38918918371f;
          } else {
            sum += (float)0.5f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 50)) {
            sum += (float)0.12380953133f;
          } else {
            sum += (float)0.56312060356f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 38)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 22)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 14)) {
        if (!(data[1].missing != -1) || (data[1].qvalue < 46)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 6)) {
            sum += (float)0.46310210228f;
          } else {
            sum += (float)0.43416216969f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 8)) {
            sum += (float)0.31321120262f;
          } else {
            sum += (float)-0.1630487442f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 10)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 26)) {
            sum += (float)0.35110151768f;
          } else {
            sum += (float)-0.17201350629f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 38)) {
            sum += (float)0.44238075614f;
          } else {
            sum += (float)0.3217625916f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 14)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 16)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 46)) {
            sum += (float)-0.4434376955f;
          } else {
            sum += (float)0.08202560246f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 24)) {
            sum += (float)-0.15279957652f;
          } else {
            sum += (float)0.41398817301f;
          }
        }
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 10)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 32)) {
            sum += (float)-0.19895155728f;
          } else {
            sum += (float)0.24230889976f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 28)) {
            sum += (float)0.30133765936f;
          } else {
            sum += (float)0.44381022453f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 26)) {
      if (!(data[3].missing != -1) || (data[3].qvalue < 36)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 40)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 30)) {
            sum += (float)-0.46051520109f;
          } else {
            sum += (float)-0.26888737082f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 20)) {
            sum += (float)-0.382229954f;
          } else {
            sum += (float)0.15168647468f;
          }
        }
      } else {
        if (!(data[5].missing != -1) || (data[5].qvalue < 64)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 48)) {
            sum += (float)-0.3805962801f;
          } else {
            sum += (float)0.4471308887f;
          }
        } else {
          if (!(data[3].missing != -1) || (data[3].qvalue < 44)) {
            sum += (float)-0.38059744239f;
          } else {
            sum += (float)0.12933586538f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 18)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 32)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 4)) {
            sum += (float)0.04914759472f;
          } else {
            sum += (float)-0.44413655996f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 6)) {
            sum += (float)-0.3220154047f;
          } else {
            sum += (float)0.15495222807f;
          }
        }
      } else {
        if (!(data[0].missing != -1) || (data[0].qvalue < 4)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.31421375275f;
          } else {
            sum += (float)0.40283849835f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 36)) {
            sum += (float)-0.087969101965f;
          } else {
            sum += (float)0.42880132794f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 42)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 26)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 14)) {
        if (!(data[5].missing != -1) || (data[5].qvalue < 6)) {
          sum += (float)0.40148764849f;
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 2)) {
            sum += (float)0.19436334074f;
          } else {
            sum += (float)0.38120472431f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 18)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 4)) {
            sum += (float)-0.23175655305f;
          } else {
            sum += (float)0.20392429829f;
          }
        } else {
          if (!(data[3].missing != -1) || (data[3].qvalue < 2)) {
            sum += (float)0.0024939514697f;
          } else {
            sum += (float)0.3603720367f;
          }
        }
      }
    } else {
      if (!(data[4].missing != -1) || (data[4].qvalue < 44)) {
        if (!(data[1].missing != -1) || (data[1].qvalue < 20)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 52)) {
            sum += (float)-0.39666321874f;
          } else {
            sum += (float)-0.1296569556f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 14)) {
            sum += (float)-0.215668872f;
          } else {
            sum += (float)0.25693520904f;
          }
        }
      } else {
        if (!(data[3].missing != -1) || (data[3].qvalue < 8)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 28)) {
            sum += (float)-0.32157039642f;
          } else {
            sum += (float)0.36414825916f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 8)) {
            sum += (float)0.08722935617f;
          } else {
            sum += (float)0.40953859687f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 30)) {
      if (!(data[3].missing != -1) || (data[3].qvalue < 18)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 56)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 36)) {
            sum += (float)-0.40253898501f;
          } else {
            sum += (float)-0.34560197592f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 16)) {
            sum += (float)-0.32984283566f;
          } else {
            sum += (float)0.13449685276f;
          }
        }
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 18)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 42)) {
            sum += (float)-0.35969725251f;
          } else {
            sum += (float)-0.057808745652f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 30)) {
            sum += (float)-0.29144203663f;
          } else {
            sum += (float)0.20366546512f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 22)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 44)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 74)) {
            sum += (float)-0.31227850914f;
          } else {
            sum += (float)0.24626122415f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 40)) {
            sum += (float)0.39087852836f;
          } else {
            sum += (float)-0.0023392497096f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 58)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.32305079699f;
          } else {
            sum += (float)0.35400265455f;
          }
        } else {
          if (!(data[0].missing != -1) || (data[0].qvalue < 10)) {
            sum += (float)0.22009564936f;
          } else {
            sum += (float)0.38877525926f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 44)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 20)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 10)) {
        sum += (float)0.3654050231f;
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 0)) {
          if (!(data[1].missing != -1) || (data[1].qvalue < 34)) {
            sum += (float)-0.27247807384f;
          } else {
            sum += (float)0.32317876816f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 36)) {
            sum += (float)0.3268660605f;
          } else {
            sum += (float)0.19015142322f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 16)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 60)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.32713872194f;
          } else {
            sum += (float)0.28172418475f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 14)) {
            sum += (float)-0.24558891356f;
          } else {
            sum += (float)0.29573851824f;
          }
        }
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 14)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 70)) {
            sum += (float)-0.17229749262f;
          } else {
            sum += (float)0.23562970757f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 40)) {
            sum += (float)0.23795045912f;
          } else {
            sum += (float)0.36564868689f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 24)) {
      if (!(data[0].missing != -1) || (data[0].qvalue < 20)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 32)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 46)) {
            sum += (float)-0.08883073926f;
          } else {
            sum += (float)-0.36831265688f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 76)) {
            sum += (float)0.085285767913f;
          } else {
            sum += (float)-0.39549651742f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 68)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 54)) {
            sum += (float)-0.15464709699f;
          } else {
            sum += (float)-0.34853443503f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 70)) {
            sum += (float)0.5053268075f;
          } else {
            sum += (float)-0.088881775737f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 22)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 28)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.36030936241f;
          } else {
            sum += (float)-0.083585172892f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 6)) {
            sum += (float)-0.24443465471f;
          } else {
            sum += (float)0.10211825371f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 52)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 56)) {
            sum += (float)0.16063173115f;
          } else {
            sum += (float)-0.34830254316f;
          }
        } else {
          if (!(data[0].missing != -1) || (data[0].qvalue < 16)) {
            sum += (float)0.052096504718f;
          } else {
            sum += (float)0.34143745899f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 36)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 14)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 6)) {
        if (!(data[1].missing != -1) || (data[1].qvalue < 0)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 42)) {
            sum += (float)-0.10878991336f;
          } else {
            sum += (float)0.27604067326f;
          }
        } else {
          sum += (float)0.34459537268f;
        }
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 2)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 4)) {
            sum += (float)-0.36814439297f;
          } else {
            sum += (float)0.24925047159f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 40)) {
            sum += (float)0.33694675565f;
          } else {
            sum += (float)0.16797873378f;
          }
        }
      }
    } else {
      if (!(data[4].missing != -1) || (data[4].qvalue < 18)) {
        if (!(data[1].missing != -1) || (data[1].qvalue < 14)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 18)) {
            sum += (float)0.33406561613f;
          } else {
            sum += (float)-0.32849487662f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 6)) {
            sum += (float)-0.19533915818f;
          } else {
            sum += (float)0.226792261f;
          }
        }
      } else {
        if (!(data[3].missing != -1) || (data[3].qvalue < 10)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 58)) {
            sum += (float)-0.1887473464f;
          } else {
            sum += (float)0.27908220887f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 26)) {
            sum += (float)0.20129770041f;
          } else {
            sum += (float)0.30269604921f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 24)) {
      if (!(data[0].missing != -1) || (data[0].qvalue < 30)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 30)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 20)) {
            sum += (float)-0.34684196115f;
          } else {
            sum += (float)-0.31047016382f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 74)) {
            sum += (float)0.22404477f;
          } else {
            sum += (float)-0.3144119978f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 56)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 52)) {
            sum += (float)-0.18174920976f;
          } else {
            sum += (float)-0.33645993471f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 68)) {
            sum += (float)0.40168553591f;
          } else {
            sum += (float)-0.12209213525f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 22)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 28)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.33247739077f;
          } else {
            sum += (float)-0.049216415733f;
          }
        } else {
          if (!(data[0].missing != -1) || (data[0].qvalue < 64)) {
            sum += (float)-0.056290775537f;
          } else {
            sum += (float)0.28776302934f;
          }
        }
      } else {
        if (!(data[0].missing != -1) || (data[0].qvalue < 12)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 32)) {
            sum += (float)-0.27705419064f;
          } else {
            sum += (float)0.2059853971f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 62)) {
            sum += (float)0.060051273555f;
          } else {
            sum += (float)0.32020533085f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 34)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 12)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 4)) {
        if (!(data[1].missing != -1) || (data[1].qvalue < 0)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 0)) {
            sum += (float)0.26027914882f;
          } else {
            sum += (float)-0.12704192102f;
          }
        } else {
          sum += (float)0.33014470339f;
        }
      } else {
        if (!(data[2].missing != -1) || (data[2].qvalue < 2)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 26)) {
            sum += (float)0.24121494591f;
          } else {
            sum += (float)-0.18749022484f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 36)) {
            sum += (float)0.32510942221f;
          } else {
            sum += (float)0.2033496052f;
          }
        }
      }
    } else {
      if (!(data[2].missing != -1) || (data[2].qvalue < 22)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 72)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 22)) {
            sum += (float)-0.13360413909f;
          } else {
            sum += (float)0.14592930675f;
          }
        } else {
          if (!(data[3].missing != -1) || (data[3].qvalue < 24)) {
            sum += (float)0.28960806131f;
          } else {
            sum += (float)0.080809541047f;
          }
        }
      } else {
        if (!(data[3].missing != -1) || (data[3].qvalue < 22)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)0.15902903676f;
          } else {
            sum += (float)0.33622068167f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 6)) {
            sum += (float)0.075237520039f;
          } else {
            sum += (float)0.32363328338f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 24)) {
      if (!(data[0].missing != -1) || (data[0].qvalue < 34)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 30)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 20)) {
            sum += (float)-0.33152222633f;
          } else {
            sum += (float)-0.26004874706f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 62)) {
            sum += (float)0.37772506475f;
          } else {
            sum += (float)-0.21631546319f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 72)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 60)) {
            sum += (float)-0.020107088611f;
          } else {
            sum += (float)-0.26316574216f;
          }
        } else {
          if (!(data[3].missing != -1) || (data[3].qvalue < 14)) {
            sum += (float)-0.24077530205f;
          } else {
            sum += (float)0.50092881918f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 10)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 46)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 28)) {
            sum += (float)-0.30727002025f;
          } else {
            sum += (float)-0.15018884838f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)0.34288907051f;
          } else {
            sum += (float)-0.077090546489f;
          }
        }
      } else {
        if (!(data[0].missing != -1) || (data[0].qvalue < 14)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 34)) {
            sum += (float)-0.21412746608f;
          } else {
            sum += (float)0.22351664305f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 60)) {
            sum += (float)0.035327967256f;
          } else {
            sum += (float)0.28406560421f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 50)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 16)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 6)) {
        if (!(data[1].missing != -1) || (data[1].qvalue < 0)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 0)) {
            sum += (float)0.24528436363f;
          } else {
            sum += (float)-0.11963183433f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 2)) {
            sum += (float)0.3229303956f;
          } else {
            sum += (float)0.30041196942f;
          }
        }
      } else {
        if (!(data[1].missing != -1) || (data[1].qvalue < 28)) {
          if (!(data[1].missing != -1) || (data[1].qvalue < 2)) {
            sum += (float)-0.10158975422f;
          } else {
            sum += (float)0.32320386171f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 16)) {
            sum += (float)-0.01733415015f;
          } else {
            sum += (float)0.24919308722f;
          }
        }
      }
    } else {
      if (!(data[4].missing != -1) || (data[4].qvalue < 30)) {
        if (!(data[5].missing != -1) || (data[5].qvalue < 32)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 66)) {
            sum += (float)-0.030217513442f;
          } else {
            sum += (float)0.2010666281f;
          }
        } else {
          if (!(data[2].missing != -1) || (data[2].qvalue < 32)) {
            sum += (float)-0.30365562439f;
          } else {
            sum += (float)0.13169440627f;
          }
        }
      } else {
        if (!(data[3].missing != -1) || (data[3].qvalue < 0)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 20)) {
            sum += (float)-0.33244097233f;
          } else {
            sum += (float)0.32476627827f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 8)) {
            sum += (float)-0.1005403176f;
          } else {
            sum += (float)0.29323965311f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 32)) {
      if (!(data[3].missing != -1) || (data[3].qvalue < 40)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 22)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 32)) {
            sum += (float)-0.32141426206f;
          } else {
            sum += (float)-0.18116903305f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 64)) {
            sum += (float)-0.27344304323f;
          } else {
            sum += (float)0.037686467171f;
          }
        }
      } else {
        if (!(data[5].missing != -1) || (data[5].qvalue < 78)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 6)) {
            sum += (float)0.067921727896f;
          } else {
            sum += (float)0.39831262827f;
          }
        } else {
          if (!(data[0].missing != -1) || (data[0].qvalue < 18)) {
            sum += (float)-0.3252761364f;
          } else {
            sum += (float)0.27652418613f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 10)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 6)) {
          if (!(data[4].missing != -1) || (data[4].qvalue < 66)) {
            sum += (float)0.041245725006f;
          } else {
            sum += (float)0.52446138859f;
          }
        } else {
          if (!(data[0].missing != -1) || (data[0].qvalue < 62)) {
            sum += (float)-0.21792821586f;
          } else {
            sum += (float)0.23689265549f;
          }
        }
      } else {
        if (!(data[0].missing != -1) || (data[0].qvalue < 0)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 38)) {
            sum += (float)-0.23326741159f;
          } else {
            sum += (float)0.18337766826f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 26)) {
            sum += (float)0.12349400669f;
          } else {
            sum += (float)0.26644510031f;
          }
        }
      }
    }
  }
  if (!(data[5].missing != -1) || (data[5].qvalue < 28)) {
    if (!(data[5].missing != -1) || (data[5].qvalue < 10)) {
      if (!(data[5].missing != -1) || (data[5].qvalue < 2)) {
        sum += (float)0.31285017729f;
      } else {
        if (!(data[1].missing != -1) || (data[1].qvalue < 32)) {
          if (!(data[1].missing != -1) || (data[1].qvalue < 4)) {
            sum += (float)-0.10573152453f;
          } else {
            sum += (float)0.30708804727f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 12)) {
            sum += (float)0.044101625681f;
          } else {
            sum += (float)0.30234992504f;
          }
        }
      }
    } else {
      if (!(data[2].missing != -1) || (data[2].qvalue < 24)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 74)) {
          if (!(data[0].missing != -1) || (data[0].qvalue < 24)) {
            sum += (float)0.20167540014f;
          } else {
            sum += (float)-0.049139205366f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 44)) {
            sum += (float)0.21960128844f;
          } else {
            sum += (float)-0.22656454146f;
          }
        }
      } else {
        if (!(data[3].missing != -1) || (data[3].qvalue < 22)) {
          if (!(data[1].missing != -1) || (data[1].qvalue < 36)) {
            sum += (float)0.25173470378f;
          } else {
            sum += (float)0.034772865474f;
          }
        } else {
          if (!(data[1].missing != -1) || (data[1].qvalue < 42)) {
            sum += (float)0.31938615441f;
          } else {
            sum += (float)0.17333266139f;
          }
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || (data[2].qvalue < 20)) {
      if (!(data[0].missing != -1) || (data[0].qvalue < 38)) {
        if (!(data[3].missing != -1) || (data[3].qvalue < 28)) {
          if (!(data[3].missing != -1) || (data[3].qvalue < 20)) {
            sum += (float)-0.31178051233f;
          } else {
            sum += (float)-0.23422501981f;
          }
        } else {
          if (!(data[0].missing != -1) || (data[0].qvalue < 8)) {
            sum += (float)-0.27882015705f;
          } else {
            sum += (float)0.068798534572f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 70)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 58)) {
            sum += (float)-0.0058056171983f;
          } else {
            sum += (float)-0.26157686114f;
          }
        } else {
          if (!(data[3].missing != -1) || (data[3].qvalue < 14)) {
            sum += (float)-0.20446254313f;
          } else {
            sum += (float)0.32547551394f;
          }
        }
      }
    } else {
      if (!(data[1].missing != -1) || (data[1].qvalue < 22)) {
        if (!(data[0].missing != -1) || (data[0].qvalue < 26)) {
          if (!(data[2].missing != -1) || (data[2].qvalue < 36)) {
            sum += (float)-0.2955134511f;
          } else {
            sum += (float)-0.018231973052f;
          }
        } else {
          if (!(data[4].missing != -1) || (data[4].qvalue < 34)) {
            sum += (float)-0.30216526985f;
          } else {
            sum += (float)0.046320073307f;
          }
        }
      } else {
        if (!(data[4].missing != -1) || (data[4].qvalue < 42)) {
          if (!(data[5].missing != -1) || (data[5].qvalue < 48)) {
            sum += (float)0.13660566509f;
          } else {
            sum += (float)-0.25923037529f;
          }
        } else {
          if (!(data[5].missing != -1) || (data[5].qvalue < 72)) {
            sum += (float)0.28026509285f;
          } else {
            sum += (float)0.0077993869781f;
          }
        }
      }
    }
  }
  
  return 1.0f / (1 + expf(-sum));
}
