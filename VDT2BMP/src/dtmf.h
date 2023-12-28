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

#define DEFAULT_MARK_DURATION_MS 250
#define DEFAULT_SPACE_DURATION_MS 250
#define DEFAULT_MAX_LEVEL 14000

typedef struct dtmf_ctx_
{
	int sample_rate;
	float freqsstep[8];

	float phase[2];
	float mod_step[2];

	int level;
	int max_level_cfg;

	serial_fifo tx_fifo;

	int mark_time_cnt;
	int mark_time_cfg;

	int space_time_cnt;
	int space_time_cfg;
}dtmf_ctx;

void dtmf_init(dtmf_ctx *dtmf, int sample_rate);
int  dtmf_genWave(dtmf_ctx *dtmf, short * buf, int size);
int  dtmf_gen_code(dtmf_ctx *dtmf, unsigned char c);
