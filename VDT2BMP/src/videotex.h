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

	unsigned int  current_attributs;

	unsigned int palette[8];

	uint32_t input_bytes_cnt;
	uint32_t pages_cnt;
	uint32_t rendered_images_cnt;

	int framerate;
	int framecnt_blink;
	int blink_state;
}videotex_ctx;

#define ATTRIBUTS_FOREGROUND_C0LOR_MASK   0x7
#define ATTRIBUTS_FOREGROUND_C0LOR_SHIFT  0x0

#define ATTRIBUTS_BACKGROUND_C0LOR_MASK   0x7
#define ATTRIBUTS_BACKGROUND_C0LOR_SHIFT  0x4

#define ATTRIBUTS_FONT_MASK        0x3
#define ATTRIBUTS_FONT_SHIFT       0x8

#define ATTRIBUTS_INVERT_SHIFT     0xA
#define ATTRIBUTS_BLINK_SHIFT      0xB
#define ATTRIBUTS_UNDERLINE_SHIFT  0xC
#define ATTRIBUTS_MASK_SHIFT       0xD
#define ATTRIBUTS_XZOOM_SHIFT      0xE
#define ATTRIBUTS_YZOOM_SHIFT      0xF

videotex_ctx * init_videotex();
int load_charset(videotex_ctx * ctx, char * file);
void render_videotex(videotex_ctx * ctx);
void select_palette(videotex_ctx * ctx, int palid);
void push_char(videotex_ctx * ctx, unsigned char c);

void deinit_videotex(videotex_ctx * ctx);
