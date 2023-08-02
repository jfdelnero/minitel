#ifndef BAND_PASS_RX_FILTER_H_
#define BAND_PASS_RX_FILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 22050 Hz

* 0 Hz - 700 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -42.0614511053001 dB

* 1100 Hz - 2300 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.2697594994676322 dB

* 2700 Hz - 11025 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -42.0614511053001 dB

*/

#define BAND_PASS_RX_FILTER_TAP_NUM 69

typedef struct {
  float history[BAND_PASS_RX_FILTER_TAP_NUM];
  unsigned int last_index;
} band_pass_rx_Filter;

void band_pass_rx_Filter_init(band_pass_rx_Filter* f);
void band_pass_rx_Filter_put(band_pass_rx_Filter* f, float input);
float band_pass_rx_Filter_get(band_pass_rx_Filter* f);

#endif
