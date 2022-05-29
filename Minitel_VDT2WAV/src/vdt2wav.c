g/*
//
// Copyright (C) 2022 Jean-François DEL NERO
//
// This file is part of vdt2wav.
//
// vdt2wav may be used and distributed without restriction provided
// that this copyright statement is not removed from the file and that any
// derivative work contains the original copyright notice and the associated
// disclaimer.
//
// vdt2wav is free software; you can redistribute it
// and/or modify  it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// vdt2wav is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//   See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with vdt2wav; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

/*
VidéoTex file (*.vdt) to wav file converter
(C) 2022 Jean-François DEL NERO

Usage example : vdt2wav -vdt:a_vdt_file.vdt -wave:OUT.WAV

The default configuration target the V23 1200 Bauds French Minitel Terminal

Available command line options :
    -vdt:file                 : vdt file
    -wave:file                : wave file

    -samplerate:[RATE]        : wave sample rate (default 11025Hz)
    -volume:[volume]          : wave volume (0-100 default 80)

    -bauds:[bauds]            : Baud rate (default 1200 Bauds)
    -zero_freq:[Hz]           : '0' bits frequency (default 2100 Hz)
    -one_freq:[Hz]            : '1' bits frequency (default 1300 Hz)
    -idle_freq:[Hz]           : Idle/carrier frequency (default 1300 Hz)

    -ser_msbfirst:[0/1]       : MSB first (default 0)
    -ser_nbstart:[nb bits]    : Start bit duration (bits) (default 1)
    -ser_nbits:[nb bits]      : Number of bits (default 7)
    -ser_parity:[0/1/2]       : Parity (0:None, 1: Even, 2: Odd) (default : 1 - Even)
    -ser_nbstop:[nb bits]     : Stop bit duration (bits) (default 1)
    -ser_preidle:[nb bits]    : Start pause (bits) (default 0)
    -ser_postidle:[nb bits]   : Stop pause (bits) (default 0)

    -initial_start_delay:[ms] : Initial start pause (ms) (default 4000)
    -page_start_delay:[ms]    : Page start pause (ms) (default 1000)
    -page_stop_delay:[ms]     : Page stop pause (ms) (default 1000)
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "wave.h"

#define DEFAULT_NUMBER_OF_CHANNELS 1
#define DEFAULT_SAMPLERATE 11025

#if defined(M_PI)
#define PI M_PI
#else
#define PI 3.1415926535897932384626433832795
#endif

int verbose;

#define FILE_CACHE_SIZE (1024)

typedef struct file_cache_
{
	FILE * f;
	int  current_offset;
	int  cur_page_size;
	int  file_size;
	unsigned char cache_buffer[FILE_CACHE_SIZE];
	int dirty;
}file_cache;

typedef struct wavegen_
{
	double sinoffset;
	double Frequency,OldFrequency;

	int sample_rate;
	int Amplitude;

	int baud_rate;

	int bit_time;
	int cur_bit_time;
	int cur_bit;

	double one_freq;
	double zero_freq;
	double idle_freq;

	int tx_buffer[64];
	int tx_buffer_size;
	int tx_buffer_offset;

	int data_buffer_offset;

	int initial_start_delay;
	int start_delay;
	int start_delay_cnt;

	int stop_delay;
	int stop_delay_cnt;

	int all_data_sent;

	int serial_pre_idle;
	int serial_nbstart;
	int serial_nbits;
	int serial_nbstop;
	int serial_parity;
	int serial_post_idle;
	int serial_msb_first;
}wavegen;

int open_file(file_cache * fc, char* path,unsigned char fill)
{
	memset(fc,0,sizeof(file_cache));

	// Read mode
	fc->f = fopen(path,"rb");
	if(fc->f)
	{
		fseek(fc->f,0,SEEK_END);
		fc->file_size = ftell(fc->f);
		fseek(fc->f,fc->current_offset,SEEK_SET);

		if(fc->current_offset + FILE_CACHE_SIZE > fc->file_size)
			fc->cur_page_size = ( fc->file_size - fc->current_offset);
		else
			fc->cur_page_size = FILE_CACHE_SIZE;

		memset(&fc->cache_buffer,fill,FILE_CACHE_SIZE);

		if(!fc->file_size)
			goto error;

		if( fread(&fc->cache_buffer,fc->cur_page_size,1,fc->f) != 1 )
			goto error;

		return 0;
	}



	return -1;

error:
	if(fc->f)
		fclose(fc->f);

	fc->f = 0;

	return -1;
}

unsigned char get_byte(file_cache * fc,int offset, int * success)
{
	unsigned char byte;
	int ret;

	byte = 0xFF;
	ret = 1;

	if(fc)
	{
		if(offset < fc->file_size)
		{
			if( ( offset >= fc->current_offset ) &&
				( offset <  (fc->current_offset + FILE_CACHE_SIZE) ) )
			{
				byte = fc->cache_buffer[ offset - fc->current_offset ];
			}
			else
			{
				fc->current_offset = (offset & ~(FILE_CACHE_SIZE-1));
				fseek(fc->f, fc->current_offset,SEEK_SET);

				memset(&fc->cache_buffer,0xFF,FILE_CACHE_SIZE);

				if(fc->current_offset + FILE_CACHE_SIZE < fc->file_size)
					ret = fread(&fc->cache_buffer,FILE_CACHE_SIZE,1,fc->f);
				else
					ret = fread(&fc->cache_buffer,fc->file_size - fc->current_offset,1,fc->f);

				byte = fc->cache_buffer[ offset - fc->current_offset ];
			}
		}
	}

	if(success)
	{
		*success = ret;
	}

	return byte;
}

void close_file(file_cache * fc)
{
	if(fc)
	{
		if(fc->f)
		{
			fclose(fc->f);

			fc->f = NULL;
		}
	}

	return;
}

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,512);
	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,512);

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':')
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] )
							{
								argtoparam[j]=argv[param][i];
								i++;
								j++;
							}
							argtoparam[j]=0;
							return 1;
						}
						else
						{
							return -1;
						}
					}
					else
					{
						return 1;
					}
				}
			}
		}
		param++;
	}

	return 0;
}

void printhelp(char* argv[])
{
	printf("Options:\n");
	printf("  -vdt:file \t\t\t: vdt file\n");
	printf("  -wave:file \t\t\t: wave file\n");
	printf("\nWave File:\n");
	printf("  -samplerate:[RATE]\t\t: wave sample rate (default 11025Hz)\n");
	printf("  -volume:[volume]\t\t: wave volume (0-100 default 80)\n");
	printf("\nAnalog link:\n");
	printf("  -bauds:[bauds]\t\t: Baud rate (default 1200 Bauds)\n");
	printf("  -zero_freq:[Hz]\t\t: '0' bits frequency (default 2100 Hz)\n");
	printf("  -one_freq:[Hz]\t\t: '1' bits frequency (default 1300 Hz)\n");
	printf("  -idle_freq:[Hz]\t\t: Idle/carrier frequency (default 1300 Hz)\n");
	printf("\nSerial encoding:\n");
	printf("  -ser_msbfirst:[0/1]\t\t: MSB first (default 0)\n");
	printf("  -ser_nbstart:[nb bits]\t: Start bit duration (bits) (default 1)\n");
	printf("  -ser_nbits:[nb bits]\t\t: Number of bits (default 7)\n");
	printf("  -ser_parity:[0/1/2]\t\t: Parity (0:None, 1: Even, 2: Odd) (default : 1 - Even)\n");
	printf("  -ser_nbstop:[nb bits]\t\t: Stop bit duration (bits) (default 1)\n");
	printf("  -ser_preidle:[nb bits]\t: Start pause (bits) (default 0)\n");
	printf("  -ser_postidle:[nb bits]\t: Stop pause (bits) (default 0)\n");
	printf("\n");
	printf("  -initial_start_delay:[ms]\t: Initial start pause (ms) (default 4000)\n");
	printf("  -page_start_delay:[ms]\t: Page start pause (ms) (default 1000)\n");
	printf("  -page_stop_delay:[ms]\t\t: Page stop pause (ms) (default 1000)\n");
	printf("\n");
	printf("  -help \t\t\t: This help\n");
	printf("\n");
}

int get_wave_file_last_samples(char* filename, short * samples_buf, int size)
{
	FILE * f;
	wav_hdr wavhdr;

	memset(&wavhdr,0,sizeof(wavhdr));

	f = fopen(filename,"rb");
	if(f)
	{
		if(fread(&wavhdr,sizeof(wav_hdr),1,f) != 1)
		{
			fclose(f);
			return -1;
		}

		fseek(f,-(size*(int)sizeof(short)),SEEK_END);

		if(fread(samples_buf,size*sizeof(short),1,f) != 1)
		{
			fclose(f);
			return -1;
		}

		fclose(f);
	}
	else
	{
		return -1;
	}

	return 0;
}

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

int prepare_next_word(int * tx_buffer,unsigned char byte, int msb_first, int pre_idle, int nb_bits, int start_bits, int parity, int stop_bits, int post_idle)
{
	int i,j;
	unsigned char mask;
	int one_bits;
	int tx_buffer_size;

	tx_buffer_size = pre_idle + start_bits +  nb_bits + stop_bits + post_idle;
	if(parity)
		tx_buffer_size++;

	if(tx_buffer_size > 64)
	{
		tx_buffer_size = 0;
		return tx_buffer_size;
	}

	j = 0;
	for(i=0;i<pre_idle;i++)
	{
		tx_buffer[j++] = 2;
	}

	for(i=0;i<start_bits;i++)
	{
		tx_buffer[j++] = 0;
	}

	one_bits = 0;
	if(msb_first)
	{
		mask = 0x01 << (nb_bits - 1);

		for(i=0;i<nb_bits;i++)
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

		for(i=0;i<nb_bits;i++)
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

	if(parity)
	{
		switch(parity)
		{
			case 1: // odd
				tx_buffer[j++] = one_bits & 1;
			break;

			case 2: // even
				tx_buffer[j++] = (one_bits & 1)^1;
			break;
		}
	}

	for(i=0;i<stop_bits;i++)
	{
		tx_buffer[j++] = 1;
	}

	for(i=0;i<post_idle;i++)
	{
		tx_buffer[j++] = 2;
	}

	tx_buffer_size = j;

	return tx_buffer_size;
}

int find_zero_phase_index(short * buf, int size)
{
	int i;
	short cur_val, old_val;

	if(!size)
		return 0;

	i = size - 1;

	old_val = buf[i];

	i--;
	while( i >= 0)
	{
		cur_val = buf[i];

		if( (old_val > cur_val) && ( (old_val >= 0) && (cur_val <= 0) ) )
		{
			return i;
		}

		old_val = cur_val;
		i--;
	}

	return 0;
}

double find_phase(wavegen * wg, double sinoffset,short samplevalue,short * buf, int size)
{
	double phase;

//	double phasedecal = ((double)((double)2*(double)PI*((double)wg->Frequency)*sinoffset)/(double)wg->sample_rate);

	phase = asin( (double)samplevalue / (double)wg->Amplitude );

//	if(phasedecal > (double)PI)

	if(buf[size-1] < buf[size-2])
		phase += PI;

	sinoffset = ( ((double)wg->sample_rate * phase) / ((double)2*(double)PI*((double)wg->Frequency)) ) + 1;

	return sinoffset;
}

int FillWaveBuffer(wavegen * wg,file_cache * fc,short * wave_buf, int Size)
{
	int i;

	if(wg->Frequency!=0 && (wg->Frequency!=wg->OldFrequency))
	{   // Sync the frequency change...
		wg->sinoffset = (wg->OldFrequency*wg->sinoffset) / wg->Frequency;
		wg->OldFrequency = wg->Frequency;
	}

	for(i=0;i<Size;i++)
	{
		*(wave_buf+i) = (int)(sin((double)((double)2*(double)PI*((double)wg->Frequency)*(double)wg->sinoffset)/(double)wg->sample_rate)*(double)wg->Amplitude);

		wg->sinoffset++;

		if( (wg->start_delay_cnt > wg->start_delay) && !wg->all_data_sent )
		{
			wg->cur_bit_time++;

			if( wg->cur_bit_time >= wg->bit_time )
			{
				if(wg->tx_buffer_offset >= wg->tx_buffer_size)
				{
					if(wg->data_buffer_offset < fc->file_size)
					{
						wg->tx_buffer_size = prepare_next_word((int*)&wg->tx_buffer,get_byte(fc,wg->data_buffer_offset, NULL),
															   wg->serial_msb_first,
															   wg->serial_pre_idle,
															   wg->serial_nbits,
															   wg->serial_nbstart,
															   wg->serial_parity,
															   wg->serial_nbstop,
															   wg->serial_post_idle);
						wg->tx_buffer_offset = 0;
						wg->data_buffer_offset++;
					}
					else
					{
						wg->all_data_sent = 1;
					}
				}

				wg->cur_bit_time = 0;

				if(wg->tx_buffer_size)
				{
					switch(wg->tx_buffer[wg->tx_buffer_offset])
					{
						case 0:
							wg->Frequency = wg->zero_freq;
						break;
						case 1:
							wg->Frequency = wg->one_freq;
						break;
						default:
							wg->Frequency = wg->idle_freq;
						break;
					}

					wg->tx_buffer_offset++;
				}

				if(wg->Frequency!=0 && wg->Frequency!=wg->OldFrequency)
				{   // Sync the frequency change...
					wg->sinoffset = (wg->OldFrequency*wg->sinoffset)/wg->Frequency;
					wg->OldFrequency = wg->Frequency;
				}
			}

		}
		else
		{
			if(wg->start_delay_cnt <= wg->start_delay)
			{
				wg->Frequency = wg->idle_freq;
				if(wg->Frequency!=0 && (wg->Frequency != wg->OldFrequency))
				{   // Sync the frequency change...
					wg->sinoffset = (wg->OldFrequency*wg->sinoffset) / wg->Frequency;
					wg->OldFrequency = wg->Frequency;
				}
				wg->start_delay_cnt++;
			}
			else
			{
				if(wg->stop_delay_cnt <= wg->stop_delay)
				{
					wg->Frequency = wg->idle_freq;
					if(wg->Frequency!=0 && (wg->Frequency != wg->OldFrequency))
					{   // Sync the frequency change...
						wg->sinoffset = (wg->OldFrequency*wg->sinoffset) / wg->Frequency;
						wg->OldFrequency = wg->Frequency;
					}
					wg->stop_delay_cnt++;
				}
				else
				{
					return i;
				}
			}
		}
	}

	return i;
}

int main(int argc, char* argv[])
{
	char filename[512];
	char ofilename[512];
	char tmp[512];

	short wavbuf[8*1024];
	short initval;

	file_cache fc;
	wavegen wg;

	unsigned int size;
	int vol;

	verbose = 0;

	printf("Minitel VDT to WAV converter v1.0.0.0\n");
	printf("Copyright (C) 2022 Jean-Francois DEL NERO\n");
	printf("This program comes with ABSOLUTELY NO WARRANTY\n");
	printf("This is free software, and you are welcome to redistribute it\n");
	printf("under certain conditions;\n\n");

	// Verbose option...
	if(isOption(argc,argv,"verbose",0)>0)
	{
		printf("verbose mode\n");
		verbose=1;
	}

	memset(&wg,0,sizeof(wg));

	wg.serial_msb_first = 0;
	if(isOption(argc,argv,"ser_msbfirst",(char*)&tmp)>0)
	{
		wg.serial_msb_first = atoi(tmp);
	}

	wg.serial_nbstart = 1;
	if(isOption(argc,argv,"ser_nbstart",(char*)&tmp)>0)
	{
		wg.serial_nbstart = atoi(tmp);
	}

	wg.serial_nbits = 7;
	if(isOption(argc,argv,"ser_nbits",(char*)&tmp)>0)
	{
		wg.serial_nbits = atoi(tmp);
	}

	wg.serial_parity = 1;
	if(isOption(argc,argv,"ser_parity",(char*)&tmp)>0)
	{
		wg.serial_parity = atoi(tmp);
	}

	wg.serial_nbstop = 1;
	if(isOption(argc,argv,"ser_nbstop",(char*)&tmp)>0)
	{
		wg.serial_nbstop = atoi(tmp);
	}

	wg.serial_pre_idle = 0;
	if(isOption(argc,argv,"ser_preidle",(char*)&tmp)>0)
	{
		wg.serial_pre_idle = atoi(tmp);
	}

	wg.serial_post_idle = 0;
	if(isOption(argc,argv,"ser_postidle",(char*)&tmp)>0)
	{
		wg.serial_post_idle = atoi(tmp);
	}

	wg.zero_freq = 2100;
	if(isOption(argc,argv,"zero_freq",(char*)&tmp)>0)
	{
		wg.zero_freq = atoi(tmp);
	}

	wg.one_freq = 1300;
	if(isOption(argc,argv,"one_freq",(char*)&tmp)>0)
	{
		wg.one_freq = atoi(tmp);
	}

	wg.idle_freq = 1300;
	if(isOption(argc,argv,"idle_freq",(char*)&tmp)>0)
	{
		wg.idle_freq = atoi(tmp);
	}

	wg.sample_rate = DEFAULT_SAMPLERATE;
	if(isOption(argc,argv,"samplerate",(char*)&tmp)>0)
	{
		wg.sample_rate = atoi(tmp);
	}

	wg.baud_rate = 1200;
	if(isOption(argc,argv,"bauds",(char*)&tmp)>0)
	{
		wg.baud_rate = atoi(tmp);
	}

	wg.bit_time = (int)((float)wg.sample_rate / (float)wg.baud_rate);

	wg.Amplitude = (int)(32767 * (float)((float)80/100));
	if(isOption(argc,argv,"volume",(char*)&tmp)>0)
	{
		vol = atoi(tmp);
		if(vol > 100)
			vol = 100;

		wg.Amplitude = (int)(32767 * (float)((float)vol/100));
	}

	wg.initial_start_delay = wg.sample_rate * 4;
	if(isOption(argc,argv,"initial_start_delay",(char*)&tmp)>0)
	{
		wg.initial_start_delay = (int)((double)atoi(tmp) * ((double)wg.sample_rate/(double)1000));
	}

	wg.start_delay = wg.sample_rate * 1;
	if(isOption(argc,argv,"page_start_delay",(char*)&tmp)>0)
	{
		wg.start_delay = (int)((double)atoi(tmp) * ((double)wg.sample_rate/(double)1000));
	}

	wg.stop_delay = wg.sample_rate * 1;
	if(isOption(argc,argv,"page_stop_delay",(char*)&tmp)>0)
	{
		wg.stop_delay = (int)((double)atoi(tmp) * ((double)wg.sample_rate/(double)1000));
	}

	wg.Frequency = wg.idle_freq;
	wg.OldFrequency = wg.idle_freq;

	// help option...
	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}

	memset(filename,0,sizeof(filename));

	// Output file name option
	strcpy(ofilename,"OUT.WAV");
	isOption(argc,argv,"foutput",(char*)&ofilename);

	if(isOption(argc,argv,"vdt",(char*)&filename)>0)
	{
	}

	if(isOption(argc,argv,"wave",(char*)&ofilename)>0)
	{
		printf("Write wave file : %s from %s\n",ofilename,filename);

		initval = 0;

		if(!get_wave_file_last_samples(ofilename, (short*)&wavbuf, 256))
		{
			double phase;
			int zero_phase_index;
			int diff_index;

			zero_phase_index = find_zero_phase_index(wavbuf, 256);
			diff_index = 256 - zero_phase_index;

			phase = (double)((double)2*(double)PI*((double)wg.Frequency)*(diff_index-0))/(double)wg.sample_rate;

			initval = wavbuf[256-1];

			wg.sinoffset = find_phase(&wg, phase,initval,(short*)&wavbuf, 256);
		}
		else
		{
			// No previously created wave file ... Use the initial delay.
			wg.start_delay = wg.initial_start_delay;
		}

		if(open_file(&fc, filename,0xFF)>=0)
		{
			do
			{
				size = FillWaveBuffer(&wg,&fc,(short*)&wavbuf, sizeof(wavbuf)/2);
				write_wave_file(ofilename,(short*)&wavbuf,size,wg.sample_rate);
			}while(size == sizeof(wavbuf)/2);

			close_file(&fc);
		}
		else
		{
			printf("ERROR : Can't open %s !\n",filename);
		}
	}

	if( (isOption(argc,argv,"help",0)<=0) &&
		(isOption(argc,argv,"vdt",0)<=0)  &&
		(isOption(argc,argv,"wave",0)<=0) )
	{
		printhelp(argv);
	}

	return 0;
}
