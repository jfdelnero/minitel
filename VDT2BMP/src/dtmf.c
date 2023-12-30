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
 DTMF Modulator / Demodulator

 (C) 2022-2O23 Jean-François DEL NERO
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <math.h>

#include "cache.h"
#include "wave.h"
#include "fifo.h"
#include "dtmf.h"
#include "modem.h"

#if defined(M_PI)
#define PI M_PI
#else
#define PI 3.1415926535897932384626433832795
#endif

static const int freqslist[8] =
{
	697,
	770,
	852,
	941,
	1209,
	1336,
	1477,
	1633
};

typedef struct dtmf_codes_
{
	unsigned char code;
	unsigned int freq1;
	unsigned int freq2;
}dtmf_codes;

static const dtmf_codes codes[]=
{
	{'1', 4, 0 },
	{'2', 5, 0 },
	{'3', 6, 0 },
	{'A', 7, 0 },

	{'4', 4, 1 },
	{'5', 5, 1 },
	{'6', 6, 1 },
	{'B', 7, 1 },

	{'7', 4, 2 },
	{'8', 5, 2 },
	{'9', 6, 2 },
	{'C', 7, 2 },

	{'*', 4, 3 },
	{'0', 5, 3 },
	{'#', 6, 3 },
	{'D', 7, 3 },

	{ 0x00, 0, 0 }
};

int dtmf_update_frequencies( dtmf_ctx *dtmf, unsigned char c )
{
	int i;

	i = 0;
	while( codes[i].code )
	{
		if( codes[i].code == c )
		{
			dtmf->mod_step[0] = dtmf->freqsstep[codes[i].freq1];
			dtmf->mod_step[1] = dtmf->freqsstep[codes[i].freq2];

			return 1;
		}
		i++;
	}

	return -1;
}

int  dtmf_genWave( dtmf_ctx *dtmf, short * buf, int size)
{
	int i,j;
	unsigned char c;
	short * buffer;

	buffer = buf;

	i = 0;
	do
	{
		if(dtmf->mark_time_cnt <= 0)
		{
			if( dtmf->space_time_cnt < dtmf->space_time_cfg )
			{
				dtmf->level = 0;
				dtmf->space_time_cnt++;
			}
			else
			{
				if( pop_from_fifo(&dtmf->tx_fifo, &c) )
				{
					if( dtmf_update_frequencies( dtmf, c ) > 0 )
					{
						dtmf->phase[0] = 0;
						dtmf->phase[1] = 0;
						dtmf->level = dtmf->max_level_cfg;

						dtmf->mark_time_cnt = (int)dtmf->mark_time_cfg;
					}
					else
					{
						dtmf->level = 0;
						dtmf->mark_time_cnt = 0;
					}
				}
				else
				{
					dtmf->level = 0;
				}
			}
		}
		else
		{
			dtmf->mark_time_cnt--;
		}

		*buffer++ = (int)((float)( sinf(dtmf->phase[0]) + sinf(dtmf->phase[1]) ) * (float)dtmf->level);

		for(j=0;j<2;j++)
		{
			dtmf->phase[j] += dtmf->mod_step[j];
			if( dtmf->phase[j] > 2 * PI )
				dtmf->phase[j] -= 2 * PI;
		}

		i++;

	}while(i<size);

	return 0;
}

// goertzelPower function
//         _____
//Buf[i]  |     |
//  ----->|  +  |----------------------X--------------------------->
//        |_____|                    __|__ Q0
//          ^  ^    _____           | REG |
//          |   \__|*coef|          |_____|
//          |      |_____|<---\        |
//          |                  \-------|Q1
//        __|__                      __|__
//       |* -1 |____________________| REG |
//       |_____|                Q2  |_____|
//
//
// m = (ftone /  (sample_rate / buffer_size ) )
// coef = 2 * COS ( 2 * PI * m / buffer_sizes)
// coef = 2 * COS ( 2 * PI * (ftone /  (sample_rate / buffer_size ) ) / buffer_sizes)
// coef = 2 * COS ( 2 * PI * (ftone *  (buffer_size / sample_rate ) ) / buffer_sizes)
// coef = 2 * COS ( 2 * PI * ( (ftone / buffer_sizes) * (buffer_size / sample_rate ) ) )
// coef = 2 * COS ( 2 * PI * (ftone / sample_rate ) )
// Q0 = Buf[i] + (Q1 * coef) + (Q2 * -1)
//

void init_goertzel(goertzet_stat * ctx, int ftone, int sample_rate, int window_ms)
{
	ctx->freq = ftone;
	ctx->W = (2.0 * M_PI * ((double)ftone) / (double)sample_rate);

	ctx->wnd_smp_cnt = (int)(((double)sample_rate * window_ms) / (double)1000);

	ctx->coef = 2 * cos(ctx->W);

	ctx->Q0 = 0;
	ctx->Q1 = 0;
	ctx->Q2 = 0;
	ctx->smp_cnt = 0;
}

static void poke_goertzel(goertzet_stat * ctx, short sample)
{
	ctx->Q0 = (double)sample + (ctx->coef * ctx->Q1) - ctx->Q2;
	ctx->Q2 = ctx->Q1;
	ctx->Q1 = ctx->Q0;
	ctx->smp_cnt++;
}

double get_goertzelPower(goertzet_stat * ctx)
{
	ctx->power = sqrt((ctx->Q1*ctx->Q1) + (ctx->Q2*ctx->Q2) - (ctx->coef * ctx->Q1 * ctx->Q2));
	return ctx->power;
}

int  dtmf_decodWave( dtmf_ctx *dtmf, short * buf, int size)
{
	int i, j;

	i = 0;
	for(i=0;i<size;i++)
	{
		for(j=0;j<8;j++)
		{
			poke_goertzel(&dtmf->goertzet_detect[j], buf[i]);
		}

		if( dtmf->goertzet_detect[0].smp_cnt >= dtmf->goertzet_detect[0].wnd_smp_cnt)
		{
			for(j=0;j<8;j++)
			{
				dtmf->goertzet_detect[j].power = get_goertzelPower( &dtmf->goertzet_detect[j] );
				printf("[%d]=%f  ",j, dtmf->goertzet_detect[j].power);
				dtmf->goertzet_detect[j].Q0 = 0;
				dtmf->goertzet_detect[j].Q1 = 0;
				dtmf->goertzet_detect[j].Q2 = 0;
				dtmf->goertzet_detect[j].smp_cnt = 0;
			}
			printf("\n");
		}
	}

	return 0;
}

void dtmf_init(dtmf_ctx *dtmf, int sample_rate)
{
	int i;

	if( dtmf )
	{
		memset(dtmf,0,sizeof(dtmf_ctx));

		dtmf->sample_rate = sample_rate;

		for(i=0;i<8;i++)
		{
			dtmf->freqsstep[i] = freq2step(sample_rate, freqslist[i] );
			init_goertzel(&dtmf->goertzet_detect[i] , freqslist[i], dtmf->sample_rate, 10);
		}

		dtmf->mark_time_cfg = (dtmf->sample_rate * DEFAULT_MARK_DURATION_MS) / 1000;
		dtmf->space_time_cfg = (dtmf->sample_rate * DEFAULT_SPACE_DURATION_MS) / 1000;
		dtmf->max_level_cfg = DEFAULT_MAX_LEVEL;
	}
}

int dtmf_gen_code(dtmf_ctx *dtmf, unsigned char c)
{
	if( !is_fifo_full(&dtmf->tx_fifo) )
	{
		push_to_fifo(&dtmf->tx_fifo, c);

		return 1;
	}

	return -1;
}
