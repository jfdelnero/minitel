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
#define DEFAULT_SAMPLERATE 44100

#if defined(M_PI)
#define PI M_PI
#else
#define PI 3.1415926535897932384626433832795
#endif

#define SERIAL_RX_FIFO_SIZE 1024

typedef struct serial_rx_fifo_
{
	int in_ptr;
	int out_ptr;

	unsigned char fifo[SERIAL_RX_FIFO_SIZE];
}serial_rx_fifo;

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

	unsigned int sample_rate;
	unsigned int sinoffs;
	float old_integrator_res[4];
	float power[4];
	float add[2];
	float mul[4];

	mean_ctx mean[4];

	int oldbit;
	int bit_time;

}modem_demodulator_ctx;

typedef struct modem_ctx_
{
	int serial_pre_idle;
	int serial_nbstart;
	int serial_nbits;
	int serial_nbstop;
	int serial_parity;
	int serial_post_idle;
	int serial_msb_first;

	double one_freq;
	double zero_freq;
	double idle_freq;

	double sinoffset;
	double Frequency,OldFrequency;

	int sample_rate;
	int Amplitude;

	int baud_rate;

	float bit_time;
	float next_bitlimit;
	int sample_offset;

	int tx_buffer[64];
	int tx_buffer_size;
	int tx_buffer_offset;

	short * wave_buf;
	int wave_size;

	unsigned int samples_generated_cnt;

	modem_demodulator_ctx demodulators[4];

	int serial_rx_delay;
	int serial_rx_prev_state;
	int serial_rx_state;
	unsigned short serial_rx_shiftreg;
	int serial_rx_parbitcnt;
	int serial_rx_cnt;

	serial_rx_fifo rx_fifo;

}modem_ctx;

void init_modem(modem_ctx *mdm);
int write_wave_file(char* filename,short * wavebuf,int size,int samplerate);
int prepare_next_word(modem_ctx * mdm, int * tx_buffer,unsigned char byte);
int FillWaveBuff(modem_ctx *mdm, int size,int offset);
int BitStreamToWave(modem_ctx *mdm);
void demodulate(modem_ctx *mdm, modem_demodulator_ctx *mdm_dmt, short * wavebuf,int samplescnt);
int push_to_rx_fifo(modem_ctx *mdm, unsigned char c);
int pop_rx_fifo(modem_ctx *mdm, unsigned char * c);
