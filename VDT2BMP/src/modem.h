/*
//
// Copyright (C) 2022-2023 Jean-François DEL NERO
//
// This file is part of vdt2bmp.
//
// vdt2bmp may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// vdt2bmp is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// vdt2bmp is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vdt2bmp; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

#define DEFAULT_NUMBER_OF_CHANNELS 1
#define DEFAULT_SAMPLERATE 22050

#if defined(M_PI)
#define PI M_PI
#else
#define PI 3.1415926535897932384626433832795
#endif

#define DEFAULT_SOUND_BUFFER_SIZE 2048

typedef struct mean_ctx_
{
	float mean_h[512];
	int  mean_wr_idx;
	int  mean_max_idx;

	float mean;
}mean_ctx;

typedef struct modem_demodulator_ctx_
{
	unsigned int freqs[2];
	float mod_step[2];
	float phase[2];

	unsigned int sample_rate;
	float old_integrator_res[4];
	float power[4];
	float add[2];

	mean_ctx mean[4];

	int bit_time;

	int uart_rx_idx;

	int carrier_detect;

	unsigned int wave_in_pages_cnt;

}modem_demodulator_ctx;

typedef struct serial_rx_
{
	int serial_rx_delay;
	int serial_rx_prev_state;
	int serial_rx_state;
	unsigned short serial_rx_shiftreg;
	int serial_rx_parbitcnt;
	int serial_rx_cnt;

	float bit_time;

	int fifo_idx;

	unsigned int last_rx_tick;
}serial_rx_ctx;

typedef struct modem_ctx_
{
	int serial_pre_idle;
	int serial_nbstart;
	int serial_nbits;
	int serial_nbstop;
	int serial_parity;
	int serial_post_idle;
	int serial_msb_first;

	unsigned int last_tx_tick;

	unsigned int freqs[3];
	float mod_step[3];
	float phase;

	int last_bitstate;

	int sample_rate;
	int Amplitude;

	int baud_rate;

	float bit_time;
	int  tx_bittime_cnt;

	int tx_buffer[64];
	int tx_buffer_size;
	int tx_buffer_offset;

	short * wave_buf;
	int wave_size;

	unsigned int samples_generated_cnt;

	modem_demodulator_ctx demodulators[4];

	serial_rx_ctx serial_rx[2];
	serial_fifo rx_fifo[2];

	serial_fifo tx_fifo;

	unsigned int wave_out_pages_cnt;

}modem_ctx;

void mdm_init(modem_ctx *mdm);

void mdm_cfg_demod(modem_ctx *mdm, modem_demodulator_ctx * demod, int baud_rate, int sample_rate, int zero_freq, int one_freq, int rxuart_idx);

int  mdm_prepare_next_word(modem_ctx * mdm, int * tx_buffer,unsigned char byte);
int  mdm_genWave(modem_ctx *mdm, short * buf, int size);
void mdm_demodulate(modem_ctx *mdm, modem_demodulator_ctx *mdm_dmt, short * wavebuf,int samplescnt);

int  mdm_is_carrier_present(modem_ctx *mdm, modem_demodulator_ctx *mdm_dmt);

int  write_wave_file(char* filename,short * wavebuf,int size,int samplerate);

float freq2step(unsigned int sample_rate, int freq );
