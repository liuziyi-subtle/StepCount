#include "nonwalk_model.h"

#include <math.h>
#include <stdlib.h>

static const unsigned char is_categorical[] = {
  0, 0, 0, 0, 0, 0
};

static const float threshold[] = {
  -0.502006f, -0.40273f, -0.341221f, -0.301764f, -0.28743f, -0.250695f, -0.224336f, 
  -0.207466f, -0.133154f, -0.110443f, -0.0920632f, -0.0335199f, 0.0142852f, 
  0.0608041f, 0.139955f, 0.158974f, 0.216639f, 0.283983f, 0.284043f, 0.324471f, 
  0.325226f, 0.333164f, 0.335503f, -55.5f, -11.5f, -10.5f, -8.5f, -7.5f, -6.5f, -4.5f, 
  -0.5f, 0.5f, 2.5f, 6.5f, 40.5f, 41.5f, 44.5f, 71.5f, 79.5f, 7.5f, 10.5f, 13.5f, 14.5f, 
  16.5f, 17.5f, 18.5f, 19.5f, 20.5f, 21.5f, 22.5f, 23.5f, 24.5f, 25.5f, 26.5f, 27.5f, -4.5f, 
  -4.0f, -0.5f, 9.5f, 12.5f, 19.5f, 31.5f, 33.5f, 43.5f, 44.5f, 45.5f, 50.5f, 61.5f, 68.5f, 
  74.5f, 93.5f, 94.5f, 98.5f, 99.5f, 0.594043f, 4.5546f, 4.73358f, 5.4772f, 6.35949f, 
  6.42859f, 7.42059f, 8.35676f, 9.46654f, 11.3858f, 13.6485f, 14.1823f, 15.1754f, 
  16.8012f, 27.5249f, 6834.61f, 7696.76f, 9211.34f, 10554.4f, 11929.5f, 13271.6f, 
  13732.1f, 27417.4f, 27976.0f, 34136.4f, 40971.0f, 47463.8f, 50428.8f, 50446.4f, 53406.2f, 
  60140.8f, 65425.2f, 72144.4f, 78457.5f, 107288.0f, 121655.0f, 140528.0f, 141440.0f, 194498.0f, 
  524611.0f, 975745.0f, 2019420.0f, 9089060.0f
};

static const int th_begin[] = {
  0, 23, 39, 55, 74, 89
};

static const int th_len[] = {
  23, 16, 16, 19, 15, 28 
};

static inline int quantize(float val, unsigned fid) {
  const float* array = &threshold[th_begin[fid]];
  int len = th_len[fid];
  int low = 0;
  int high = len;
  int mid;
  float mval;
  if (val < array[0]) {
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

  if (!(data[5].missing != -1) || data[5].qvalue < 34) {
    if (!(data[5].missing != -1) || data[5].qvalue < 20) {
      if (!(data[5].missing != -1) || data[5].qvalue < 10) {
        if (!(data[1].missing != -1) || data[1].qvalue < 30) {
          sum += 0.596871f;
        } else {
          sum += 0.266667f;
        }
      } else {
        if (!(data[3].missing != -1) || data[3].qvalue < 10) {
          sum += 0.245283f;
        } else {
          sum += 0.551951f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 12) {
        if (!(data[3].missing != -1) || data[3].qvalue < 16) {
          sum += -0.52228f;
        } else {
          sum += 0.410526f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 4) {
          sum += -0.075f;
        } else {
          sum += 0.570513f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 18) {
      if (!(data[2].missing != -1) || data[2].qvalue < 12) {
        if (!(data[3].missing != -1) || data[3].qvalue < 28) {
          sum += -0.580222f;
        } else {
          sum += -0.315742f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 18) {
          sum += -0.553642f;
        } else {
          sum += 0.154987f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 6) {
        if (!(data[0].missing != -1) || data[0].qvalue < 16) {
          sum += -0.563636f;
        } else {
          sum += 0.383607f;
        }
      } else {
        if (!(data[0].missing != -1) || data[0].qvalue < 0) {
          sum += -0.178723f;
        } else {
          sum += 0.575972f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 36) {
    if (!(data[5].missing != -1) || data[5].qvalue < 18) {
      if (!(data[5].missing != -1) || data[5].qvalue < 6) {
        sum += 0.462601f;
      } else {
        if (!(data[3].missing != -1) || data[3].qvalue < 4) {
          sum += 0.119217f;
        } else {
          sum += 0.415861f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 12) {
        if (!(data[3].missing != -1) || data[3].qvalue < 12) {
          sum += -0.432221f;
        } else {
          sum += 0.213111f;
        }
      } else {
        if (!(data[3].missing != -1) || data[3].qvalue < 2) {
          sum += -0.294208f;
        } else {
          sum += 0.431168f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 18) {
      if (!(data[3].missing != -1) || data[3].qvalue < 22) {
        if (!(data[0].missing != -1) || data[0].qvalue < 32) {
          sum += -0.46312f;
        } else {
          sum += -0.197856f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 10) {
          sum += -0.360007f;
        } else {
          sum += 0.212634f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 6) {
        if (!(data[0].missing != -1) || data[0].qvalue < 16) {
          sum += -0.436237f;
        } else {
          sum += 0.281664f;
        }
      } else {
        if (!(data[0].missing != -1) || data[0].qvalue < 0) {
          sum += -0.136963f;
        } else {
          sum += 0.441709f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 32) {
    if (!(data[5].missing != -1) || data[5].qvalue < 14) {
      if (!(data[5].missing != -1) || data[5].qvalue < 4) {
        sum += 0.401331f;
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 22) {
          sum += 0.385062f;
        } else {
          sum += 0.250651f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 10) {
        if (!(data[3].missing != -1) || data[3].qvalue < 12) {
          sum += -0.386356f;
        } else {
          sum += 0.285008f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 6) {
          sum += 0.063115f;
        } else {
          sum += 0.393635f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 16) {
      if (!(data[0].missing != -1) || data[0].qvalue < 18) {
        if (!(data[3].missing != -1) || data[3].qvalue < 26) {
          sum += -0.404258f;
        } else {
          sum += -0.182931f;
        }
      } else {
        if (!(data[4].missing != -1) || data[4].qvalue < 22) {
          sum += -0.349997f;
        } else {
          sum += 0.132705f;
        }
      }
    } else {
      if (!(data[0].missing != -1) || data[0].qvalue < 8) {
        if (!(data[2].missing != -1) || data[2].qvalue < 26) {
          sum += -0.317314f;
        } else {
          sum += 0.367878f;
        }
      } else {
        if (!(data[4].missing != -1) || data[4].qvalue < 10) {
          sum += -0.377976f;
        } else {
          sum += 0.39587f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 38) {
    if (!(data[5].missing != -1) || data[5].qvalue < 22) {
      if (!(data[5].missing != -1) || data[5].qvalue < 8) {
        if (!(data[1].missing != -1) || data[1].qvalue < 28) {
          sum += 0.365408f;
        } else {
          sum += 0.140173f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 4) {
          sum += 0.0141877f;
        } else {
          sum += 0.313207f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 8) {
        if (!(data[0].missing != -1) || data[0].qvalue < 44) {
          sum += -0.347781f;
        } else {
          sum += 0.0324755f;
        }
      } else {
        if (!(data[3].missing != -1) || data[3].qvalue < 6) {
          sum += -0.0985035f;
        } else {
          sum += 0.423418f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 20) {
      if (!(data[3].missing != -1) || data[3].qvalue < 34) {
        if (!(data[0].missing != -1) || data[0].qvalue < 20) {
          sum += -0.361831f;
        } else {
          sum += -0.20027f;
        }
      } else {
        if (!(data[0].missing != -1) || data[0].qvalue < 6) {
          sum += -0.0902116f;
        } else {
          sum += 0.559571f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 20) {
        if (!(data[2].missing != -1) || data[2].qvalue < 28) {
          sum += -0.287383f;
        } else {
          sum += 0.303415f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 2) {
          sum += 0.0845935f;
        } else {
          sum += 0.359104f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 40) {
    if (!(data[5].missing != -1) || data[5].qvalue < 16) {
      if (!(data[5].missing != -1) || data[5].qvalue < 6) {
        if (!(data[1].missing != -1) || data[1].qvalue < 0) {
          sum += 0.187513f;
        } else {
          sum += 0.343678f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 24) {
          sum += 0.307471f;
        } else {
          sum += 0.134463f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 6) {
        if (!(data[3].missing != -1) || data[3].qvalue < 14) {
          sum += -0.317755f;
        } else {
          sum += 0.0225352f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 8) {
          sum += -0.0582542f;
        } else {
          sum += 0.359683f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 14) {
      if (!(data[3].missing != -1) || data[3].qvalue < 36) {
        if (!(data[0].missing != -1) || data[0].qvalue < 14) {
          sum += -0.346872f;
        } else {
          sum += -0.254641f;
        }
      } else {
        if (!(data[0].missing != -1) || data[0].qvalue < 10) {
          sum += -0.153039f;
        } else {
          sum += 0.441945f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 24) {
        if (!(data[0].missing != -1) || data[0].qvalue < 30) {
          sum += -0.325249f;
        } else {
          sum += 0.107178f;
        }
      } else {
        if (!(data[0].missing != -1) || data[0].qvalue < 4) {
          sum += -0.0380582f;
        } else {
          sum += 0.340648f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 30) {
    if (!(data[5].missing != -1) || data[5].qvalue < 6) {
      if (!(data[5].missing != -1) || data[5].qvalue < 0) {
        if (!(data[1].missing != -1) || data[1].qvalue < 0) {
          sum += 0.161327f;
        } else {
          sum += 0.330317f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 22) {
          sum += 0.312944f;
        } else {
          sum += 0.151243f;
        }
      }
    } else {
      if (!(data[3].missing != -1) || data[3].qvalue < 10) {
        if (!(data[0].missing != -1) || data[0].qvalue < 36) {
          sum += -0.2133f;
        } else {
          sum += 0.314189f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 24) {
          sum += 0.291743f;
        } else {
          sum += 0.159693f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 10) {
      if (!(data[3].missing != -1) || data[3].qvalue < 20) {
        if (!(data[0].missing != -1) || data[0].qvalue < 38) {
          sum += -0.330795f;
        } else {
          sum += -0.149223f;
        }
      } else {
        if (!(data[5].missing != -1) || data[5].qvalue < 46) {
          sum += 0.150256f;
        } else {
          sum += -0.249155f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 14) {
        if (!(data[0].missing != -1) || data[0].qvalue < 26) {
          sum += -0.306172f;
        } else {
          sum += 0.260247f;
        }
      } else {
        if (!(data[4].missing != -1) || data[4].qvalue < 14) {
          sum += -0.191963f;
        } else {
          sum += 0.30611f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 42) {
    if (!(data[5].missing != -1) || data[5].qvalue < 12) {
      if (!(data[5].missing != -1) || data[5].qvalue < 2) {
        if (!(data[1].missing != -1) || data[1].qvalue < 0) {
          sum += 0.137675f;
        } else {
          sum += 0.318919f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 22) {
          sum += 0.300075f;
        } else {
          sum += 0.0492495f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 2) {
        if (!(data[5].missing != -1) || data[5].qvalue < 24) {
          sum += 0.0419876f;
        } else {
          sum += -0.297823f;
        }
      } else {
        if (!(data[3].missing != -1) || data[3].qvalue < 0) {
          sum += -0.168652f;
        } else {
          sum += 0.290339f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 22) {
      if (!(data[4].missing != -1) || data[4].qvalue < 26) {
        if (!(data[0].missing != -1) || data[0].qvalue < 40) {
          sum += -0.320908f;
        } else {
          sum += -0.248892f;
        }
      } else {
        if (!(data[5].missing != -1) || data[5].qvalue < 50) {
          sum += 0.273166f;
        } else {
          sum += -0.267272f;
        }
      }
    } else {
      if (!(data[0].missing != -1) || data[0].qvalue < 2) {
        if (!(data[2].missing != -1) || data[2].qvalue < 30) {
          sum += -0.191911f;
        } else {
          sum += 0.340832f;
        }
      } else {
        if (!(data[4].missing != -1) || data[4].qvalue < 16) {
          sum += -0.058074f;
        } else {
          sum += 0.304611f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 28) {
    if (!(data[5].missing != -1) || data[5].qvalue < 6) {
      if (!(data[4].missing != -1) || data[4].qvalue < 0) {
        sum += 0.315811f;
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 0) {
          sum += 0.14761f;
        } else {
          sum += 0.290135f;
        }
      }
    } else {
      if (!(data[3].missing != -1) || data[3].qvalue < 18) {
        if (!(data[0].missing != -1) || data[0].qvalue < 34) {
          sum += -0.15822f;
        } else {
          sum += 0.239575f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 26) {
          sum += 0.288655f;
        } else {
          sum += 0.127752f;
        }
      }
    }
  } else {
    if (!(data[2].missing != -1) || data[2].qvalue < 8) {
      if (!(data[3].missing != -1) || data[3].qvalue < 30) {
        if (!(data[0].missing != -1) || data[0].qvalue < 24) {
          sum += -0.303404f;
        } else {
          sum += -0.173849f;
        }
      } else {
        if (!(data[5].missing != -1) || data[5].qvalue < 54) {
          sum += 0.375139f;
        } else {
          sum += -0.317793f;
        }
      }
    } else {
      if (!(data[1].missing != -1) || data[1].qvalue < 16) {
        if (!(data[0].missing != -1) || data[0].qvalue < 28) {
          sum += -0.266156f;
        } else {
          sum += 0.213107f;
        }
      } else {
        if (!(data[5].missing != -1) || data[5].qvalue < 48) {
          sum += 0.295964f;
        } else {
          sum += -0.0011941f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 44) {
    if (!(data[5].missing != -1) || data[5].qvalue < 6) {
      if (!(data[4].missing != -1) || data[4].qvalue < 0) {
        sum += 0.310251f;
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 0) {
          sum += 0.122358f;
        } else {
          sum += 0.277071f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 12) {
        if (!(data[1].missing != -1) || data[1].qvalue < 4) {
          sum += -0.240661f;
        } else {
          sum += 0.0578132f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 2) {
          sum += -0.167264f;
        } else {
          sum += 0.296104f;
        }
      }
    }
  } else {
    if (!(data[4].missing != -1) || data[4].qvalue < 28) {
      if (!(data[2].missing != -1) || data[2].qvalue < 24) {
        if (!(data[0].missing != -1) || data[0].qvalue < 22) {
          sum += -0.306f;
        } else {
          sum += -0.200516f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 30) {
          sum += -0.0319073f;
        } else {
          sum += 0.300985f;
        }
      }
    } else {
      if (!(data[5].missing != -1) || data[5].qvalue < 52) {
        if (!(data[3].missing != -1) || data[3].qvalue < 8) {
          sum += -0.142197f;
        } else {
          sum += 0.334089f;
        }
      } else {
        if (!(data[3].missing != -1) || data[3].qvalue < 32) {
          sum += -0.28034f;
        } else {
          sum += 0.149983f;
        }
      }
    }
  }
  if (!(data[5].missing != -1) || data[5].qvalue < 26) {
    if (!(data[5].missing != -1) || data[5].qvalue < 2) {
      if (!(data[1].missing != -1) || data[1].qvalue < 0) {
        if (!(data[3].missing != -1) || data[3].qvalue < 8) {
          sum += 0.158143f;
        } else {
          sum += -0.137235f;
        }
      } else {
        if (!(data[4].missing != -1) || data[4].qvalue < 0) {
          sum += 0.305769f;
        } else {
          sum += 0.26408f;
        }
      }
    } else {
      if (!(data[2].missing != -1) || data[2].qvalue < 14) {
        if (!(data[0].missing != -1) || data[0].qvalue < 42) {
          sum += -0.0856605f;
        } else {
          sum += 0.169478f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 26) {
          sum += 0.193259f;
        } else {
          sum += 0.340826f;
        }
      }
    }
  } else {
    if (!(data[0].missing != -1) || data[0].qvalue < 12) {
      if (!(data[2].missing != -1) || data[2].qvalue < 26) {
        if (!(data[3].missing != -1) || data[3].qvalue < 24) {
          sum += -0.299114f;
        } else {
          sum += -0.0481128f;
        }
      } else {
        if (!(data[1].missing != -1) || data[1].qvalue < 20) {
          sum += -0.0220594f;
        } else {
          sum += 0.349032f;
        }
      }
    } else {
      if (!(data[4].missing != -1) || data[4].qvalue < 18) {
        if (!(data[4].missing != -1) || data[4].qvalue < 4) {
          sum += -0.278784f;
        } else {
          sum += -0.0946173f;
        }
      } else {
        if (!(data[2].missing != -1) || data[2].qvalue < 4) {
          sum += -0.120593f;
        } else {
          sum += 0.273089f;
        }
      }
    }
  }
  
  return 1.0f / (1 + expf(-sum));
}
