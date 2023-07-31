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

typedef struct app_ctx_
{
	videotex_ctx * vdt_ctx;
	modem_ctx *mdm;
	int verbose;
	int quit;
	int fir_en;
	FIR_2100_1300_22050_Filter fir;
#ifdef SDL_SUPPORT
	SDL_Surface *screen;
	SDL_TimerID timer;
	SDL_Window* window;
	const unsigned char * keystate;
	int audio_id;
#endif

	int indexbuf;
	int imgcnt;
	int pageindex;

}app_ctx;

enum {
	OUTPUT_MODE_FILE = 0,
	OUTPUT_MODE_STDOUT = 1,
	OUTPUT_MODE_SDL = 2
};

int update_frame(videotex_ctx * vdt_ctx,modem_ctx *mdm, bitmap_data * bmp);
