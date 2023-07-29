#include "FIR_V22_Minitel.h"

static int filter_taps[FIR_2100_1300_22050_FILTER_TAP_NUM] = {
  -157,
  16,
  47,
  85,
  112,
  116,
  87,
  32,
  -34,
  -87,
  -106,
  -79,
  -12,
  72,
  140,
  158,
  109,
  -3,
  -146,
  -273,
  -335,
  -297,
  -155,
  57,
  282,
  450,
  504,
  423,
  227,
  -24,
  -255,
  -395,
  -406,
  -299,
  -125,
  39,
  124,
  92,
  -42,
  -215,
  -337,
  -326,
  -143,
  186,
  572,
  889,
  1012,
  866,
  453,
  -135,
  -746,
  -1209,
  -1389,
  -1231,
  -782,
  -171,
  429,
  857,
  1017,
  906,
  607,
  257,
  -8,
  -104,
  -29,
  139,
  275,
  258,
  27,
  -390,
  -876,
  -1262,
  -1384,
  -1147,
  -563,
  242,
  1060,
  1668,
  1891,
  1668,
  1060,
  242,
  -563,
  -1147,
  -1384,
  -1262,
  -876,
  -390,
  27,
  258,
  275,
  139,
  -29,
  -104,
  -8,
  257,
  607,
  906,
  1017,
  857,
  429,
  -171,
  -782,
  -1231,
  -1389,
  -1209,
  -746,
  -135,
  453,
  866,
  1012,
  889,
  572,
  186,
  -143,
  -326,
  -337,
  -215,
  -42,
  92,
  124,
  39,
  -125,
  -299,
  -406,
  -395,
  -255,
  -24,
  227,
  423,
  504,
  450,
  282,
  57,
  -155,
  -297,
  -335,
  -273,
  -146,
  -3,
  109,
  158,
  140,
  72,
  -12,
  -79,
  -106,
  -87,
  -34,
  32,
  87,
  116,
  112,
  85,
  47,
  16,
  -157
};

void FIR_2100_1300_22050_Filter_init(FIR_2100_1300_22050_Filter* f) {
  int i;
  for(i = 0; i < FIR_2100_1300_22050_FILTER_TAP_NUM; ++i)
    f->history[i] = 0;
  f->last_index = 0;
}

void FIR_2100_1300_22050_Filter_put(FIR_2100_1300_22050_Filter* f, int input) {
  f->history[f->last_index++] = input;
  if(f->last_index == FIR_2100_1300_22050_FILTER_TAP_NUM)
    f->last_index = 0;
}

int FIR_2100_1300_22050_Filter_get(FIR_2100_1300_22050_Filter* f) {
  long long acc = 0;
  int index = f->last_index, i;
  for(i = 0; i < FIR_2100_1300_22050_FILTER_TAP_NUM; ++i) {
    index = index != 0 ? index-1 : FIR_2100_1300_22050_FILTER_TAP_NUM-1;
    acc += (long long)f->history[index] * filter_taps[i];
  };
  return acc >> 16;
}
