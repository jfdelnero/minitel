#ifndef FIR_2100_1300_22050_FILTER_H_
#define FIR_2100_1300_22050_FILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 22050 Hz

fixed point precision: 16 bits

* 0 Hz - 1000 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 1200 Hz - 1400 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 1600 Hz - 1800 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

* 2000 Hz - 2200 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = n/a

* 2350 Hz - 11025 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define FIR_2100_1300_22050_FILTER_TAP_NUM 157

typedef struct {
  int history[FIR_2100_1300_22050_FILTER_TAP_NUM];
  unsigned int last_index;
} FIR_2100_1300_22050_Filter;

void FIR_2100_1300_22050_Filter_init(FIR_2100_1300_22050_Filter* f);
void FIR_2100_1300_22050_Filter_put(FIR_2100_1300_22050_Filter* f, int input);
int FIR_2100_1300_22050_Filter_get(FIR_2100_1300_22050_Filter* f);

#endif
