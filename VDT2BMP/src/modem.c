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

/*
 FSK Modem

 Modulation and demodulation support.
 Full duplex UART support

 (C) 2022-2O23 Jean-François DEL NERO
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "cache.h"
#include "wave.h"
#include "fifo.h"
#include "modem.h"
#include "dtmf.h"

int write_wave_file(char* filename,short * wavebuf,int size,int samplerate)
{
	FILE * f;
	wav_hdr wavhdr;

	memset(&wavhdr,0,sizeof(wavhdr));

	f = fopen(filename,"r+b");
	if(f)
	{
		if(fread(&wavhdr,sizeof(wav_hdr),1,f) != 1)
		{
			fclose(f);
			return -1;
		}

		wavhdr.ChunkSize += size*2;
		wavhdr.Subchunk2Size += size*2;

		fseek(f,0,SEEK_END);

		fwrite(wavebuf,size*2,1,f);

		fseek(f,0,SEEK_SET);

		fwrite((void*)&wavhdr,sizeof(wav_hdr),1,f);

		fclose(f);
	}
	else
	{
		f = fopen(filename,"wb");
		if(f)
		{
			memcpy((char*)&wavhdr.RIFF,"RIFF",4);
			memcpy((char*)&wavhdr.WAVE,"WAVE",4);
			memcpy((char*)&wavhdr.fmt,"fmt ",4);
			wavhdr.Subchunk1Size = 16;
			wavhdr.AudioFormat = 1;
			wavhdr.NumOfChan = DEFAULT_NUMBER_OF_CHANNELS;
			wavhdr.SamplesPerSec = samplerate;
			wavhdr.bitsPerSample = 16;
			wavhdr.bytesPerSec = ((samplerate*wavhdr.bitsPerSample)/8);
			wavhdr.blockAlign = (wavhdr.bitsPerSample/8);
			memcpy((char*)&wavhdr.Subchunk2ID,"data",4);

			wavhdr.ChunkSize = size*2 + sizeof(wav_hdr) - 8;
			wavhdr.Subchunk2Size = size*2;

			fwrite((void*)&wavhdr,sizeof(wav_hdr),1,f);

			fwrite(wavebuf,size*2,1,f);

			fclose(f);
		}
		else
			return -1;
	}

	return 0;
}

int mdm_prepare_next_word(modem_ctx * mdm, int * tx_buffer,unsigned char byte)
{
	int i,j;
	unsigned char mask;
	int one_bits;
	int tx_buffer_size;

	tx_buffer_size = mdm->serial_pre_idle + mdm->serial_nbstart +  mdm->serial_nbits + mdm->serial_nbstop + mdm->serial_post_idle;

	if(mdm->serial_parity)
		tx_buffer_size++;

	if(tx_buffer_size > 64)
	{
		tx_buffer_size = 0;
		return tx_buffer_size;
	}

	j = 0;
	for(i=0;i<mdm->serial_pre_idle;i++)
	{
		tx_buffer[j++] = 2;
	}

	for(i=0;i<mdm->serial_nbstart;i++)
	{
		tx_buffer[j++] = 0;
	}

	one_bits = 0;
	if(mdm->serial_msb_first)
	{
		mask = 0x01 << (mdm->serial_nbits - 1);

		for(i=0;i<mdm->serial_nbits;i++)
		{
			if(byte & mask)
			{
				tx_buffer[j++] = 1;
				one_bits++;
			}
			else
			{
				tx_buffer[j++] = 0;
			}

			mask >>= 1;
		}
	}
	else
	{
		mask = 0x01;

		for(i=0;i<mdm->serial_nbits;i++)
		{
			if(byte & mask)
			{
				tx_buffer[j++] = 1;
				one_bits++;
			}
			else
			{
				tx_buffer[j++] = 0;
			}

			mask <<= 1;
		}
	}

	if(mdm->serial_parity)
	{
		switch(mdm->serial_parity)
		{
			case 1: // even parity - make sure that numbers of '1' bits is even. (data + parity bits included)
				tx_buffer[j++] = one_bits & 1;
			break;

			case 2: // odd parity - make sure that numbers of '1' bits is odd. (data + parity bits included)
				tx_buffer[j++] = (one_bits & 1) ^ 1;
			break;
		}
	}

	for(i=0;i<mdm->serial_nbstop;i++)
	{
		tx_buffer[j++] = 1;
	}

	for(i=0;i<mdm->serial_post_idle;i++)
	{
		tx_buffer[j++] = 2;
	}

	tx_buffer_size = j;

	return tx_buffer_size;
}

int mdm_serial_rx(modem_ctx *mdm, serial_rx_ctx * rx_ctx, int state)
{
	unsigned char mask;

	if(rx_ctx->serial_rx_delay)
	{
		rx_ctx->serial_rx_delay--;
		rx_ctx->serial_rx_prev_state = state;

		return 0;
	}

	switch(rx_ctx->serial_rx_state)
	{
		case 0: // Wait start bit
			if(!state && rx_ctx->serial_rx_prev_state)
			{
				rx_ctx->serial_rx_delay = rx_ctx->bit_time + (rx_ctx->bit_time/2);
				rx_ctx->serial_rx_shiftreg = 0x0000;
				rx_ctx->serial_rx_parbitcnt = 0;
				rx_ctx->serial_rx_cnt = 0;

				rx_ctx->serial_rx_state = 1;
			}
		break;
		case 1: // Rx Shift
			mask = 0x01;

			if(state)
			{
				rx_ctx->serial_rx_shiftreg |= (mask << rx_ctx->serial_rx_cnt);
				rx_ctx->serial_rx_parbitcnt++;
			}

			rx_ctx->serial_rx_cnt++;
			if(rx_ctx->serial_rx_cnt == mdm->serial_nbits)
			{
				rx_ctx->serial_rx_state = 2;
			}

			rx_ctx->serial_rx_delay = rx_ctx->bit_time;
		break;

		case 2: // Parity
			rx_ctx->serial_rx_state = 3;
			rx_ctx->serial_rx_delay = rx_ctx->bit_time;

			switch(mdm->serial_parity)
			{
				case 1: // even parity - check that the total numbers or '1' are even (data + parity bits included.)
					if( (rx_ctx->serial_rx_parbitcnt & 1)  != (state&1) )
					{
						// bad parity
						rx_ctx->serial_rx_state = 0;
						rx_ctx->serial_rx_delay = 0;

#if 0
						printf("Parity Error !\n");
#endif
					}
				break;

				case 2: // odd parity - check that the total numbers or '1' are odd (data + parity bits included.)
					if( (rx_ctx->serial_rx_parbitcnt & 1)  != ((state^1)&1) )
					{
						// bad parity
						rx_ctx->serial_rx_state = 0;
						rx_ctx->serial_rx_delay = 0;

#if 0
						printf("Parity Error !\n");
#endif
					}
				break;

				default:
				break;
			}
		break;

		case 3: // Stop bit
			rx_ctx->serial_rx_state = 0;
			rx_ctx->serial_rx_delay = 0;

			if( rx_ctx->fifo_idx == 1 )
			{
				printf("Rx byte : 0x%.2X %c\n",rx_ctx->serial_rx_shiftreg, rx_ctx->serial_rx_shiftreg&0x7F);
			}

			if( !push_to_fifo(&mdm->rx_fifo[rx_ctx->fifo_idx], rx_ctx->serial_rx_shiftreg) )
			{
				printf("RX Fifo full !\n");
			}

			rx_ctx->last_rx_tick = mdm->wave_out_pages_cnt;

#if 0
			printf("RX : 0x%.2X\n",rx_ctx->serial_rx_shiftreg);
#endif
		break;
	}

	rx_ctx->serial_rx_prev_state = state;

	return 0;
}

int mdm_genWave(modem_ctx *mdm, short * buf, int size)
{
	int i;
	unsigned char c;
	short * buffer;

	buffer = mdm->wave_buf;

	if(buf)
		buffer = buf;

	i = 0;
	do
	{
		if(mdm->tx_bittime_cnt <= 0)
		{
			if( mdm->tx_buffer_offset < mdm->tx_buffer_size )
			{
				mdm->last_bitstate = mdm->tx_buffer[mdm->tx_buffer_offset];

				mdm->tx_buffer_offset++;

				mdm->tx_bittime_cnt = mdm->bit_time;
			}
			else
			{
				if( pop_from_fifo(&mdm->tx_fifo, &c) )
				{
					mdm->tx_buffer_size = mdm_prepare_next_word( mdm, (int*)mdm->tx_buffer, c );
					mdm->tx_buffer_offset = 0;

					mdm->last_bitstate = mdm->tx_buffer[mdm->tx_buffer_offset];

					mdm->tx_buffer_offset++;

					mdm->tx_bittime_cnt = (int)mdm->bit_time;

					mdm->last_tx_tick = mdm->wave_out_pages_cnt;
				}
				else
				{
					mdm->tx_buffer_size = 0;
					mdm->last_bitstate = 1;
				}
			}
		}
		else
		{
			mdm->tx_bittime_cnt--;
		}

		*buffer++ = (int)(sinf(mdm->phase)*(float)mdm->Amplitude);
		mdm->phase += mdm->mod_step[mdm->last_bitstate];
		if( mdm->phase > 2 * PI )
			mdm->phase -= 2 * PI;

		mdm->samples_generated_cnt++;
		i++;

	}while(i<size);

	mdm->wave_out_pages_cnt++;

	return 0;
}

// ----------------------------------------------------------------------------------------------
//
// Correlation demodulator
// (!! This not the currently implemented method !!)
// The correlator multiplies an input signal by a delayed copy of itself.
//
//                                                      _____       ______
//                                 |------------------>|     |     |      |
//           ____________________  |  __________       |     |     |      |
//          |                    | | |          |      | Xor |---->|Filter|-----> To UART Input
//Signal--->| Analog to Digital  |-->|  Delay   |----->|     |     |      |
//          |   (Comparator>0)   |   |__________|      |_____|     |______|
//          |____________________|
//
// Delay selection :
//
// Example :
// HF = 2100Hz;
// LF = 1300Hz;
// COS(2*PI()*LF*delay) - COS(2*PI()*HF*delay) = diff
// Take the delay value having the higher diff value.
//
// ----------------------------------------------------------------------------------------------
// Quadrature Non-coherent FSK receiver
// (Implemented)
//                          _____         _________        _________
//                         |     |       |         |      |         |
//           |------------>| MUL |------>|   AVG   |----->|   ^2    |------|
//           |             |_____|       |_________|      |_________|   ___v__
//           |                 ^SIN                                    |      |
//           |                 |                                       |  ADD |----------
//           |       _____     |          _________        _________   |______|         |
//           |      |     |    |         |         |      |         |      ^            |
//           |----->| MUL |------------->|   AVG   |----->|   ^2    |------|            |
//           |      |_____|    |         |_________|      |_________|                   |
//           |        ^COS     |                                                        |
//           |        |        |                                                        |
//           |        |        |                                                        |
//           |       _|___     |                                                        |
//           |      |     |____|                                                     ___v___       _________
//   RX      |      | OSC |                                                         |  ADD  |     |         |
// Signal -->|      |  f1 |                                                         |       |---->| Compare |----> To UART Input
//           |      |_____|                                                         |__SUB__|     |_________|
//           |                                                                          ^
//           |                                                                          |
//           |                                                                          |
//           |              _____         _________        _________                    |
//           |             |     |       |         |      |         |                   |
//           |------------>| MUL |------>|   AVG   |----->|   ^2    |------|            |
//           |             |_____|       |_________|      |_________|   ___v__          |
//           |                 ^SIN                                    |      |         |
//           |                 |                                       |  ADD |----------
//           |       _____     |          _________        _________   |______|
//           |      |     |    |         |         |      |         |      ^
//           |----->| MUL |------------->|   AVG   |----->|   ^2    |------|
//                  |_____|    |         |_________|      |_________|
//                    ^COS     |
//                    |        |
//                    |        |
//                   _|___     |
//                  |     |____|
//                  | OSC |
//                  |  f2 |
//                  |_____|
//

static float add_and_compute_mean(mean_ctx * ctx, float val)
{
	int i;
	float result;

	ctx->mean_h[ctx->mean_wr_idx++] = val;

	if( ctx->mean_wr_idx >= ctx->mean_max_idx)
		ctx->mean_wr_idx = 0;

	result = 0;
	for(i=0;i<ctx->mean_max_idx;i++)
	{
		result += ctx->mean_h[i];
	}

	result /= ctx->mean_max_idx;

	ctx->mean = result;

	return result;
}

float freq2step(unsigned int sample_rate, int freq )
{
	return (2*PI*((float)freq)) /(float)sample_rate;
}

//
// Main demodulator function
//
void mdm_demodulate(modem_ctx *mdm, modem_demodulator_ctx *mdm_dmt, short * wavebuf,int samplescnt)
{
	int i,j;
	int bit;
	float mul_freq;
	float f1_sincos,result;

	for(i=0;i<samplescnt;i++)
	{
		// f1 & f2 frequencies generation + quadrature sinus / cosinus multiply operation.
		for(j=0;j<4;j++)
		{
			// Generate sinus / cosinus signals
			f1_sincos = sinf( mdm_dmt->phase[j>>1] + ((PI/2.0)*(j&1)) );

			// Multiply the input signal with the generated signal
			mul_freq = f1_sincos * (float)(wavebuf[i]/(float)32765);

			// Basic integrator with leak
			mdm_dmt->old_integrator_res[j] = (((float)mdm_dmt->old_integrator_res[j]) * 0.92) + mul_freq;

			// Power computation
			mdm_dmt->power[j] = mdm_dmt->old_integrator_res[j] * mdm_dmt->old_integrator_res[j];

			// Windowed mean calculation
			add_and_compute_mean(&mdm_dmt->mean[j], mdm_dmt->power[j]);
		}

		// Next phase calculation for both oscillator.
		for(j=0;j<2;j++)
		{
			mdm_dmt->phase[j] += mdm_dmt->mod_step[j];

			// Prevent overflow/performance issue.
			if( mdm_dmt->phase[j] > 2 * PI )
				mdm_dmt->phase[j] -= 2 * PI;
		}

		// Add "I" and "Q" power for both frequencies.
		for(j=0;j<2;j++)
		{
			mdm_dmt->add[j] = mdm_dmt->mean[j*2].mean + mdm_dmt->mean[(j*2)+1].mean;
		}

		// And finally to the difference between the f1 and f2 power.
		result = mdm_dmt->add[1] - mdm_dmt->add[0];

		// If < 0 -> '0' logic state. If >= 0 -> '1' logic state
		bit = 0;
		if(result>=0)
			bit = 1;

		// Carrier detect
		if( mdm_dmt->add[0] + mdm_dmt->add[1] > 0.15 )
		{
			if( mdm_dmt->carrier_detect < (mdm_dmt->sample_rate / 8) )
				mdm_dmt->carrier_detect++;
		}
		else
		{
			mdm_dmt->carrier_detect = (mdm_dmt->carrier_detect * 7) / 8;
		}

		// Gate the UART input if carrier detect level is under the threshold
		if( mdm_dmt->carrier_detect < ( ( (mdm_dmt->sample_rate / 8) * 3 ) / 4 ) )
			bit = 1;

		//if(mdm_dmt->uart_rx_idx == 1)
		//	printf(">%d\n",mdm_dmt->carrier_detect);

		// Feed the UART RX input
		mdm_serial_rx(mdm,&mdm->serial_rx[mdm_dmt->uart_rx_idx],bit);

#if 0
		if(mdm_dmt->uart_rx_idx == 1)
			printf("%f;%f;%f;%f;%f;%f;%f;%d\n",mdm_dmt->power[0],mdm_dmt->power[1],mdm_dmt->power[2],mdm_dmt->power[3],mdm_dmt->add[0],mdm_dmt->add[1],result,bit);
#endif
	}

	mdm_dmt->wave_in_pages_cnt++;
}

int  mdm_is_carrier_present(modem_ctx *mdm, modem_demodulator_ctx *mdm_dmt)
{
	if( mdm_dmt->carrier_detect < ( ( (mdm_dmt->sample_rate / 8) * 3 ) / 4 ) )
		return 0;
	else
		return 1;
}

void mdm_cfg_demod(modem_ctx *mdm, modem_demodulator_ctx * demod, int baud_rate, int sample_rate, int zero_freq, int one_freq, int rxuart_idx)
{
	int i;

	demod->sample_rate = sample_rate;
	demod->bit_time = (demod->sample_rate / baud_rate);

	demod->uart_rx_idx = rxuart_idx;

	demod->freqs[0] = zero_freq;
	demod->freqs[1] = one_freq;

	for(i=0;i<2;i++)
	{
		demod->mod_step[i] = freq2step(demod->sample_rate, demod->freqs[i] );
		demod->phase[i] = 0;
	}

	for(i=0;i<4;i++)
	{
		demod->mean[i].mean_max_idx = (int)((float)((demod->sample_rate / one_freq))*0.80);
	}

	mdm->serial_rx[demod->uart_rx_idx].bit_time = ((float)demod->sample_rate / (float)baud_rate);
	mdm->serial_rx[demod->uart_rx_idx].fifo_idx = rxuart_idx;
}

void mdm_init(modem_ctx *mdm)
{
	int i;

	if(mdm)
	{
		memset(mdm,0,sizeof(modem_ctx));
		mdm->serial_msb_first = 0;
		mdm->serial_nbstart = 1;
		mdm->serial_nbits = 7;
		mdm->serial_parity = 1;
		mdm->serial_nbstop = 1;
		mdm->serial_pre_idle = 0;
		mdm->serial_post_idle = 0;

		mdm->freqs[0] = 2100;// Zero
		mdm->freqs[1] = 1300;// One
		mdm->freqs[2] = 1300;// Idle

		mdm->sample_rate = DEFAULT_SAMPLERATE;

		for(i=0;i<3;i++)
		{
			mdm->mod_step[i] = freq2step(mdm->sample_rate, mdm->freqs[i] );
		}

		mdm->phase = 0;

		// Modulator settings
		mdm->baud_rate = 1200;
		mdm->bit_time =  ((float)mdm->sample_rate / (float)mdm->baud_rate);
		mdm->Amplitude = (int)(32767 * (float)((float)70/100));

		// Feedback demodulator settings
		mdm_cfg_demod(mdm, &mdm->demodulators[0], mdm->baud_rate, DEFAULT_SAMPLERATE, 2100, 1300, 0);

		// Downlink demodulator settings (link coming from the minitel)
		mdm_cfg_demod(mdm, &mdm->demodulators[1], 75, DEFAULT_SAMPLERATE, 450, 390, 1);

		mdm->wave_size = DEFAULT_SOUND_BUFFER_SIZE;
		mdm->wave_buf = malloc(mdm->wave_size * sizeof(short));
		if( mdm->wave_buf )
		{
			memset(mdm->wave_buf,0, mdm->wave_size * sizeof(short));
		}
		else
		{
			mdm->wave_size = 0;
		}
	}
}
