/*
//
// Copyright (C) 2022 Jean-François DEL NERO
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
// along with vdt2wav; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

/*
Videotex file (*.vdt) to bmp file converter
(C) 2022 Jean-François DEL NERO

videotex decoder.
*/

typedef struct videotex_ctx_
{
	int char_res_x;
	int char_res_y;

	int char_res_x_size;
	int char_res_y_size;

	unsigned char * char_buffer;
	unsigned int  * attribut_buffer;

	uint32_t * bmp_buffer;
	int bmp_res_x;
	int bmp_res_y;

	unsigned char * charset;

	int cursor_x_pos;
	int cursor_y_pos;

	uint32_t decoder_state;
	uint32_t decoder_step;
	unsigned char decoder_buffer[8];


	unsigned char last_char;

	unsigned char foreground_color;
	unsigned char background_color;

	unsigned int palette[8];

	int font;


}videotex_ctx;

videotex_ctx * init_videotex();
int load_charset(videotex_ctx * ctx, char * file);
void render_videotex(videotex_ctx * ctx);
void push_char(videotex_ctx * ctx, unsigned char c);

void deinit_videotex(videotex_ctx * ctx);
