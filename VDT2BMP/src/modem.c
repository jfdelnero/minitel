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

int prepare_next_word(modem_ctx * mdm, int * tx_buffer,unsigned char byte)
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

int FillWaveBuff(modem_ctx *mdm, int size,int offset)
{
	int i;

	i = 0;
	while( i < size )
	{
		*(mdm->wave_buf+offset) = (int)(sin((double)((double)2*(double)PI*((double)mdm->Frequency)*(double)mdm->sinoffset)/(double)mdm->sample_rate)*(double)mdm->Amplitude);
		mdm->sinoffset++;
		mdm->sample_offset++;
		mdm->samples_generated_cnt++;
		i++;
		offset++;
	}

	return offset;
}

int BitStreamToWave(modem_ctx *mdm)
{
	int i,j;
	int cnt = 0;

	mdm->next_bitlimit -= mdm->sample_offset;
	mdm->sample_offset = 0;

	j = 0;
	for( i = 0;i < mdm->tx_buffer_size;i++ )
	{
		switch(mdm->tx_buffer[i])
		{
			case 0:
				mdm->Frequency = mdm->zero_freq;
			break;
			case 1:
				mdm->Frequency = mdm->one_freq;
			break;
			default:
				mdm->Frequency = mdm->idle_freq;
			break;
		}

		if(mdm->Frequency!=0 && (mdm->Frequency!=mdm->OldFrequency))
		{   // Sync the frequency change...
			mdm->sinoffset = (mdm->OldFrequency*mdm->sinoffset) / mdm->Frequency;
			mdm->OldFrequency = mdm->Frequency;
		}

		j = FillWaveBuff(mdm, ((int)mdm->next_bitlimit - mdm->sample_offset), j);

		mdm->next_bitlimit += mdm->bit_time;
	}

	mdm->wave_size = j;

	return cnt;
}

void init_modem(modem_ctx *mdm)
{
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
		mdm->zero_freq = 2100;
		mdm->one_freq = 1300;
		mdm->idle_freq = 1300;
		mdm->sample_rate = DEFAULT_SAMPLERATE;
		mdm->baud_rate = 1200;
		mdm->bit_time =  ((float)mdm->sample_rate / (float)mdm->baud_rate);
		mdm->Amplitude = (int)(32767 * (float)((float)80/100));

		mdm->wave_buf = malloc(256 * 1024 * sizeof(short));
		if(mdm->wave_buf )
			memset(mdm->wave_buf,0,256 * 1024 * sizeof(short));
	}
}
