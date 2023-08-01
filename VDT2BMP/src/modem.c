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
 Minitel Modem
(C) 2022-2O23 Jean-François DEL NERO
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "cache.h"
#include "wave.h"
#include "modem.h"

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
			case 1: // odd
				tx_buffer[j++] = one_bits & 1;
			break;

			case 2: // even
				tx_buffer[j++] = (one_bits & 1)^1;
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

int mdm_is_fifo_empty(serial_fifo *fifo)
{
	if( ( fifo->in_ptr & (SERIAL_FIFO_SIZE-1) ) == ( fifo->out_ptr & (SERIAL_FIFO_SIZE-1) ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int mdm_is_fifo_full(serial_fifo *fifo)
{
	if( ( (fifo->in_ptr + 1) & (SERIAL_FIFO_SIZE-1) ) == ( fifo->out_ptr & (SERIAL_FIFO_SIZE-1) ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int mdm_push_to_fifo(serial_fifo *fifo, unsigned char c)
{
	if ( mdm_is_fifo_full(fifo) )
		return 0;

	fifo->fifo[(fifo->in_ptr & (SERIAL_FIFO_SIZE-1))] = c;

	fifo->in_ptr = (fifo->in_ptr + 1) & (SERIAL_FIFO_SIZE-1);

	return 1;
}

int mdm_pop_from_fifo(serial_fifo *fifo, unsigned char * c)
{
	int ret;

	ret = 0;

	if( !mdm_is_fifo_empty(fifo) )
	{
		*c = fifo->fifo[fifo->out_ptr & (SERIAL_FIFO_SIZE-1)];
		fifo->out_ptr = (fifo->out_ptr + 1) & (SERIAL_FIFO_SIZE-1);
		ret = 1;
	}

	return ret;
}

int mdm_serial_rx(modem_ctx *mdm, int state)
{
	unsigned char mask;

	if(mdm->serial_rx_delay)
	{
		mdm->serial_rx_delay--;
		mdm->serial_rx_prev_state = state;
		return 0;
	}

	switch(mdm->serial_rx_state)
	{
		case 0: // Wait start bit
			if(!state && mdm->serial_rx_prev_state)
			{
				mdm->serial_rx_delay = mdm->bit_time + (mdm->bit_time/2);
				mdm->serial_rx_shiftreg = 0x0000;
				mdm->serial_rx_parbitcnt = 0;
				mdm->serial_rx_cnt = 0;

				mdm->serial_rx_state = 1;
			}
		break;
		case 1: // Rx Shift
			mask = 0x01;

			if(state)
			{
				mdm->serial_rx_shiftreg |= (mask << mdm->serial_rx_cnt);
				mdm->serial_rx_parbitcnt++;
			}

			mdm->serial_rx_cnt++;
			if(mdm->serial_rx_cnt == mdm->serial_nbits)
			{
				mdm->serial_rx_state = 2;
			}

			mdm->serial_rx_delay = mdm->bit_time;
		break;

		case 2: // Parity
			switch(mdm->serial_parity)
			{
				case 1: // odd
					if( (mdm->serial_rx_parbitcnt & 1)  != (state&1) )
					{
						// bad parity
#if 0
						printf("Parity Error !\n");
#endif
					}
				break;

				case 2: // even
					if( (mdm->serial_rx_parbitcnt & 1)  != ((state^1)&1) )
					{
						// bad parity
#if 0
						printf("Parity Error !\n");
#endif
					}
				break;
			}

			mdm->serial_rx_state = 3;
			mdm->serial_rx_delay = mdm->bit_time;
		break;

		case 3: // Stop bit
			mdm->serial_rx_state = 0;
			mdm->serial_rx_delay = 0;

			if( !mdm_push_to_fifo(&mdm->rx_fifo, mdm->serial_rx_shiftreg) )
			{
				printf("RX Fifo full !\n");
			}

#if 0
			printf("RX : 0x%.2X\n",mdm->serial_rx_shiftreg);
#endif
		break;
	}

	mdm->serial_rx_prev_state = state;

	return 0;

}

// cur_state_ticks
//
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
				if( mdm_pop_from_fifo(&mdm->tx_fifo, &c) )
				{
					mdm->tx_buffer_size = mdm_prepare_next_word( mdm, (int*)mdm->tx_buffer, c );
					mdm->tx_buffer_offset = 0;

					mdm->last_bitstate = mdm->tx_buffer[mdm->tx_buffer_offset];

					mdm->tx_buffer_offset++;

					mdm->tx_bittime_cnt = (int)mdm->bit_time;

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
//           |                 |                                       |  ADD |---------|
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
//           |                 |                                       |  ADD |---------|
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

void mdm_demodulate(modem_ctx *mdm, modem_demodulator_ctx *mdm_dmt, short * wavebuf,int samplescnt)
{
	int i,j;
	int bit;
	float mul_freq;
	float f1_sincos,result;

	for(i=0;i<samplescnt;i++)
	{
		for(j=0;j<4;j++)
		{
			f1_sincos = sinf( mdm_dmt->phase[j>>1] + ((PI/2.0)*(j&1)) );

			mul_freq = f1_sincos * (float)(wavebuf[i]/(float)32765);

			mdm_dmt->mul[j] = f1_sincos;

			mdm_dmt->old_integrator_res[j] =  (((float)mdm_dmt->old_integrator_res[j]) * 0.9) + mul_freq;

			mdm_dmt->power[j] = mdm_dmt->old_integrator_res[j] * mdm_dmt->old_integrator_res[j];

			add_and_compute_mean(&mdm_dmt->mean[j], mdm_dmt->power[j]);
		}

		for(j=0;j<2;j++)
		{
			mdm_dmt->phase[j] += mdm_dmt->mod_step[j];

			if( mdm_dmt->phase[j] > 2 * PI )
				mdm_dmt->phase[j] -= 2 * PI;
		}

		for(j=0;j<2;j++)
		{
			mdm_dmt->add[j] = mdm_dmt->mean[j*2].mean + mdm_dmt->mean[(j*2)+1].mean;
		}

		result = mdm_dmt->add[1] - mdm_dmt->add[0];

		bit = 0;
		if(result>=0)
			bit = 1;

		mdm_serial_rx(mdm, bit);

#if 0
		printf("%f;%f;%f;%f;%f;%f;%f;%d\n",mdm_dmt->power[0],mdm_dmt->power[1],mdm_dmt->power[2],mdm_dmt->power[3],mdm_dmt->add[0],mdm_dmt->add[1],result,bit);
#endif

		mdm_dmt->oldbit = bit;
	}
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

		mdm->baud_rate = 1200;
		mdm->bit_time =  ((float)mdm->sample_rate / (float)mdm->baud_rate);
		mdm->Amplitude = (int)(32767 * (float)((float)80/100));

		mdm->demodulators[0].sample_rate = DEFAULT_SAMPLERATE;
		mdm->demodulators[0].bit_time = (mdm->sample_rate / mdm->baud_rate);

		mdm->demodulators[0].freqs[0] = 2100;
		mdm->demodulators[0].freqs[1] = 1300;
		for(i=0;i<2;i++)
		{
			mdm->demodulators[0].mod_step[i] = freq2step(mdm->demodulators[0].sample_rate, mdm->demodulators[0].freqs[i] );
			mdm->demodulators[0].phase[i] = 0;
		}

		for(i=0;i<4;i++)
		{
			mdm->demodulators[0].mean[i].mean_max_idx = (int)((float)((mdm->sample_rate / mdm->baud_rate)) * 0.75);
		}

		mdm->wave_buf = malloc(1024 * sizeof(short));
		if(mdm->wave_buf )
		{
			mdm->wave_size = 1024;
			memset(mdm->wave_buf,0, mdm->wave_size * sizeof(short));
		}
	}
}
