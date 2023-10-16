#ifndef LOW_PASS_TX_FILTER_H_
#define LOW_PASS_TX_FILTER_H_

/*

FIR filter designed with
 http://t-filter.appspot.com

sampling frequency: 22050 Hz

* 0 Hz - 550 Hz
  gain = 1
  desired ripple = 5 dB
  actual ripple = 3.820014374338185 dB

* 950 Hz - 11025 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = -40.74717682991265 dB

*/

#define LOW_PASS_TX_FILTER_TAP_NUM 65

typedef struct {
  float history[LOW_PASS_TX_FILTER_TAP_NUM];
  unsigned int last_index;
} low_pass_tx_Filter;

void low_pass_tx_Filter_init(low_pass_tx_Filter* f);
void low_pass_tx_Filter_put(low_pass_tx_Filter* f, float input);
float low_pass_tx_Filter_get(low_pass_tx_Filter* f);

#endif

