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
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <stdint.h>

#include "videotex.h"

#include "fonts.h"

// Minitel resolution :
// 40x25 characters
// 8x10 character size
// 320*250 screen resolution

//#define DEBUG 1

#ifdef DEBUG
#define LOG(...) fprintf(stderr,__VA_ARGS__)
#else
#define LOG(...)
#endif

#define ERROR(...) fprintf(stderr,__VA_ARGS__)

const unsigned char special_char[][4]=
{
	{ 0x02,   23, 0x41, 0x61 }, // à
	{ 0x02,   25, 0x41, 0x65 }, // è
	{ 0x02,    8, 0x41, 0x75 }, // ù
	{ 0x02,   18, 0x42, 0x65 }, // é
	{ 0x02,    1, 0x43, 0x61 }, // â
	{ 0x02,   27, 0x43, 0x65 }, // ê
	{ 0x01,   26, 0x6A, 0x00 }, // Œ
	{ 0x02,   13, 0x43, 0x69 }, // î
	{ 0x01,   12, 0x2C, 0x00 }, // ←
	{ 0x01,    3, 0x23, 0x00 }, // £
	{ 0x01,   14, 0x2E, 0x00 }, // →
	{ 0x01,   15, 0x2F, 0x00 }, // ↓
	{ 0x01,   16, 0x30, 0x00 }, // °
	{ 0x01,   17, 0x31, 0x00 }, // ±
	{ 0x02,   31, 0x43, 0x6F }, // ô
	{ 0x02,   22, 0x43, 0x75 }, // û
	{ 0x02,   19, 0x48, 0x65 }, // ë
	{ 0x02,   20, 0x48, 0x69 }, // ï
	{ 0x02,   21, 0x4B, 0x63 }, // ç
	{ 0x01,   26, 0x7A, 0x00 }, // œ
	{ 0x01,   28, 0x3C, 0x00 }, // ¼
	{ 0x01,   29, 0x3D, 0x00 }, // ½
	{ 0x01,   30, 0x3E, 0x00 }, // ¾
	{ 0x01,   94, 0x2D, 0x00 }, // ↑

	{ 0x02,   97, 0x42, 0x61 }, //  á
	{ 0x02,   97, 0x48, 0x61 }, //  ä
	{ 0x02,  105, 0x41, 0x69 }, //  ì
	{ 0x02,  105, 0x42, 0x69 }, //  í
	{ 0x02,  111, 0x41, 0x6F }, //  ò
	{ 0x02,  111, 0x42, 0x6F }, //  ó
	{ 0x02,  111, 0x48, 0x6F }, //  ö
	{ 0x02,  117, 0x42, 0x75 }, //  ú
	{ 0x02,  117, 0x48, 0x75 }, //  ü

	{ 0x01,   31, 0x7B, 0x00 }, //  ß
	{ 0x01,   31, 0x7B, 0x00 }, //  β

	{ 0x01,   '$', '$', 0x00 }, //  $

	{ 0x00, 0x00, 0x00, 0x00 }
};

/*
const unsigned char special_char[][4]=
{
	{ 0x02,    0, 0x41, 0x61 }, // à
	{ 0x02,    1, 0x41, 0x65 }, // è
	{ 0x02,    2, 0x41, 0x75 }, // ù
	{ 0x02,    5, 0x42, 0x65 }, // é
	{ 0x02,    7, 0x43, 0x61 }, // â
	{ 0x02,    8, 0x43, 0x65 }, // ê
	{ 0x01,   10, 0x6A, 0x00 }, // Œ
	{ 0x02,   11, 0x43, 0x69 }, // î
	{ 0x01,   12, 0x2C, 0x00 }, // ←
	{ 0x01,   13, 0x23, 0x00 }, // £
	{ 0x01,   14, 0x2E, 0x00 }, // →
	{ 0x01,   15, 0x2F, 0x00 }, // ↓
	{ 0x01,   16, 0x30, 0x00 }, // °
	{ 0x01,   17, 0x31, 0x00 }, // ±
	{ 0x02,   18, 0x43, 0x6F }, // ô
	{ 0x02,   19, 0x43, 0x75 }, // û
	{ 0x02,   22, 0x48, 0x65 }, // ë
	{ 0x02,   23, 0x48, 0x69 }, // ï
	{ 0x02,   25, 0x4B, 0x63 }, // ç
	{ 0x01,   26, 0x7A, 0x00 }, // œ
	{ 0x01,   28, 0x3C, 0x00 }, // ¼
	{ 0x01,   29, 0x3D, 0x00 }, // ½
	{ 0x01,   30, 0x3E, 0x00 }, // ¾
	{ 0x01,   94, 0x2D, 0x00 }, // ↑

	{ 0x02,   97, 0x42, 0x61 }, //  á
	{ 0x02,   97, 0x48, 0x61 }, //  ä
	{ 0x02,  105, 0x41, 0x69 }, //  ì
	{ 0x02,  105, 0x42, 0x69 }, //  í
	{ 0x02,  111, 0x41, 0x6F }, //  ò
	{ 0x02,  111, 0x42, 0x6F }, //  ó
	{ 0x02,  111, 0x48, 0x6F }, //  ö
	{ 0x02,  117, 0x42, 0x75 }, //  ú
	{ 0x02,  117, 0x48, 0x75 }, //  ü

	{ 0x01,   31, 0x7B, 0x00 }, //  ß
	{ 0x01,   31, 0x7B, 0x00 }, //  β

	{ 0x01,   '$', '$', 0x00 }, //  $

	{ 0x00, 0x00, 0x00, 0x00 }
};
*/

static int vdt_find_special_char(videotex_ctx * ctx,unsigned char * code, int size,unsigned char * charcode)
{
	int i,j;
	unsigned char * ptr;

	i = 0;
	while(special_char[i][0])
	{
		ptr = (unsigned char *)&special_char[i];

		if( ptr[0] == size)
		{
			j = 0;
			while(j < size && j < 2)
			{
				if( ptr[2 + j] != code[j] )
				{
					break;
				}

				j++;
			}

			if(j==size)
			{
				*charcode = ptr[1];
				return 1;
			}
		}

		i++;
	}

	return 0;
}

static unsigned int vdt_set_mask(unsigned int reg, int shift, unsigned int mask, unsigned int val)
{
	return ( ( reg & ~(mask << shift) ) | ((val & mask) << shift) );
}

static unsigned int vdt_get_mask(unsigned int reg, int shift, unsigned int mask)
{
	return ( (reg >> shift) & mask);
}

static void vdt_resetstate(videotex_ctx * ctx)
{
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FOREGROUND_C0LOR_SHIFT, ATTRIBUTS_FOREGROUND_C0LOR_MASK, 7);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_BACKGROUND_C0LOR_SHIFT, ATTRIBUTS_BACKGROUND_C0LOR_MASK, 0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_BLINK_SHIFT, 0x1, 0x0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_MASK_SHIFT, 0x1, 0x0);
	ctx->underline_latch = 0;
}

void vdt_select_palette(videotex_ctx * ctx, int pal_id)
{
	int i;

	switch(pal_id)
	{
		case 0: // Black and white
			i = 0;
			ctx->palette[i++] = 0x000000; // 0%
			ctx->palette[i++] = 0x7F7F7F; // 50%
			ctx->palette[i++] = 0xB2B2B2; // 70%
			ctx->palette[i++] = 0xE5E5E5; // 90%
			ctx->palette[i++] = 0x666666; // 40%
			ctx->palette[i++] = 0x999999; // 60%
			ctx->palette[i++] = 0xCCCCCC; // 80%
			ctx->palette[i++] = 0xFFFFFF; // 100%
		break;
		case 1: // Color
			for(i=0;i<8;i++)
			{
				ctx->palette[i] = 0x000000;
				if(i&1)
					ctx->palette[i] |= 0x000000FF;
				if(i&2)
					ctx->palette[i] |= 0x0000FF00;
				if(i&4)
					ctx->palette[i] |= 0x00FF0000;
			}
		break;
	}
}

static int vdt_set_cursor(videotex_ctx * ctx,unsigned mask,int x,int y)
{
	if(mask&1)
		ctx->cursor_x_pos = x;

	if(mask&2)
		ctx->cursor_y_pos = y;

	return 0;
}

videotex_ctx * vdt_init()
{
	videotex_ctx * ctx;

	ctx = NULL;

	ctx = malloc(sizeof(videotex_ctx));
	if(ctx)
	{
		memset(ctx,0,sizeof(videotex_ctx));

		ctx->char_res_x = 40;
		ctx->char_res_y = 25;

		ctx->char_res_x_size = 8;
		ctx->char_res_y_size = 10;

		ctx->bmp_res_x = ctx->char_res_x * ctx->char_res_x_size;
		ctx->bmp_res_y = ctx->char_res_y * ctx->char_res_y_size;

		ctx->char_buffer = malloc(ctx->char_res_x * ctx->char_res_y);
		if(!ctx->char_buffer)
			goto error;

		memset(ctx->char_buffer,' ',ctx->char_res_x * ctx->char_res_y);

		ctx->attribut_buffer = malloc(ctx->char_res_x * ctx->char_res_y * sizeof(unsigned int));
		if(!ctx->attribut_buffer)
			goto error;

		memset(ctx->attribut_buffer,0,ctx->char_res_x * ctx->char_res_y * sizeof(unsigned int) );

		ctx->bmp_buffer = malloc(ctx->bmp_res_x * ctx->bmp_res_y * sizeof(uint32_t));
		if(!ctx->bmp_buffer)
			goto error;

		ctx->prev_raw_attributs = malloc(ctx->char_res_x * sizeof(unsigned int));
		if(!ctx->prev_raw_attributs)
			goto error;

		memset(ctx->bmp_buffer,0x29,ctx->bmp_res_x * ctx->bmp_res_y * sizeof(uint32_t) );

		vdt_select_palette(ctx, 1);

		vdt_resetstate(ctx);

		vdt_set_cursor(ctx,3,38,0);
		ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x1);
		vdt_push_char(ctx, 'C');
		ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x0);
		vdt_set_cursor(ctx,3,0,1);

		ctx->last_char = ' ';
	}

	return ctx;

error:
	if(ctx)
	{
		if(ctx->char_buffer)
			free(ctx->char_buffer);

		if(ctx->attribut_buffer)
			free(ctx->attribut_buffer);

		if(ctx->bmp_buffer)
			free(ctx->bmp_buffer);

		if(ctx->prev_raw_attributs)
			free(ctx->prev_raw_attributs);

		free(ctx);
	}

	return NULL;
}

static int vdt_getpixstate(unsigned char * buf, int xpos, int ypos, int xsize, int ysize,int msbfirst)
{
	int biti;

	if( (xpos < xsize) && (ypos < ysize) )
	{
		biti = ((ypos * xsize) + xpos);

		if(msbfirst)
		{
			if( buf[biti >> 3] & (0x80 >> (biti&7)) )
				return 1;
			else
				return 0;
		}
		else
		{
			if( buf[biti >> 3] & (0x01 << (biti&7)) )
				return 1;
			else
				return 0;
		}
	}

	return 0;
}

static void vdt_set_char_pix(unsigned char * buf,int x,int y,int xsize,int ysize, int state)
{
	int bitoffset;

	if( (x < xsize) && (y < ysize) )
	{
		bitoffset = (xsize * y) + x;

		if(state)
			buf[bitoffset >> 3] |= (0x80 >> (bitoffset&7));
		else
			buf[bitoffset >> 3] &= ~(0x80 >> (bitoffset&7));
	}
}

static void vdt_copy_char(unsigned char *dest_buf,unsigned char *src_buf,unsigned int src_x_size, unsigned int src_y_size, unsigned int src_x_pos, unsigned int src_y_pos, unsigned int char_x_size, unsigned int char_y_size, int msbfirst)
{
	unsigned int x, y;

	for(y=0;y<char_y_size;y++)
	{
		for(x=0;x<char_x_size;x++)
		{
			if(vdt_getpixstate(src_buf, src_x_pos + x, src_y_pos + y, src_x_size, src_y_size,msbfirst))
			{
				vdt_set_char_pix(dest_buf, x, y, char_x_size, char_y_size, 1);
			}
			else
			{
				vdt_set_char_pix(dest_buf, x, y, char_x_size, char_y_size, 0);
			}
		}
	}
}


int vdt_load_charset(videotex_ctx * ctx, char * file)
{
	FILE * f;
	unsigned char * charsettmp;
	int size,ret;
	int i,j,l;
	ret = 0;
	size = 0;

	if(ctx)
	{
		ctx->charset_size = 192 * 16; // 16 Bytes per char
		size = ctx->charset_size;

		ctx->charset = malloc(ctx->charset_size);
		if(!ctx->charset)
			return -1;

		memset(ctx->charset, 0,ctx->charset_size);

		if( file )
		{
			f = fopen(file,"rb");
			if(f)
			{
				fseek(f,0,SEEK_END);

				size = ftell(f);

				fseek(f,0,SEEK_SET);

				if(size)
				{
					charsettmp = malloc(size);

					if(charsettmp)
					{
						ret = fread(charsettmp,size,1,f);
					}
				}

				fclose(f);
			}
		}
		else
		{
			if(0)
			{
				// The Teletel have 192 characters
				// 64 group of 3 Characters
				l = 0;

				j = 0;
				for(i=0;i<64;i++)
				{
					vdt_copy_char( &ctx->charset[((8*16)*(i+'@'))/8] ,(unsigned char*)font_teletel,8, 2048, 0, (i * 32) + (j*10), 8, 10, 1 );
					l++;
				}

				j = 1;
				for(i=0;i<64;i++)
				{
					vdt_copy_char( &ctx->charset[((8*16)*(i))/8] ,(unsigned char*)font_teletel,8, 2048, 0, (i * 32) + (j*10), 8, 10, 1 );
					l++;
				}

				j = 2;
				for(i=0;i<64;i++)
				{
					vdt_copy_char( &ctx->charset[((8*16)*(i+128))/8] ,(unsigned char*)font_teletel,8, 2048, 0, (i * 32) + (j*10), 8, 10, 1 );
					l++;
				}
			}
			else
			{
				// EF9345 font

				l = 0;

				for(i=0;i<64;i++)
				{
					l = 192 + i;
					vdt_copy_char( &ctx->charset[((8*16)*(i+'@'))/8] ,(unsigned char*)font_ef9345,32, 2048, (l&3) * 8, (l >> 2) * 16, 8, 10, 0 );
				}

				for(i=0;i<64;i++)
				{
					l = i;
					vdt_copy_char( &ctx->charset[((8*16)*(i))/8] ,    (unsigned char*)font_ef9345,32, 2048, (l&3) * 8, (l >> 2) * 16, 8, 10, 0 );
				}

				for(i=0;i<64;i++)
				{
					l = 256 + i;
					vdt_copy_char( &ctx->charset[((8*16)*(i+128))/8] ,(unsigned char*)font_ef9345,32, 2048, (l&3) * 8, (l >> 2) * 16, 8, 10, 0 );
				}

				for(i=0;i<64;i++)
				{
					l = 320 + i;
					vdt_copy_char( &ctx->charset[((8*16)*(i+128))/8] ,(unsigned char*)font_ef9345,32, 2048, (l&3) * 8, (l >> 2) * 16, 8, 10, 0 );
				}
			}

			ret = 1;
		}

#if 0
		if(ctx->charset && size)
		{
			int i,j;

			f = fopen("expanded_font.raw","wb");
			if(f)
			{
				for(i=0;i<size;i++)
				{
					for(j=0;j<8;j++)
					{
						if( ctx->charset[i] & (0x80>>j) )
						{
							fputc(0xFF,f);
						}
						else
						{
							fputc(0x00,f);
						}
					}
				}
				fclose(f);
			}
		}
#endif

	}

	return ret;
}

static unsigned char char_mask[][10]=
{
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Normal
	{0x88,0x88,0xFF,0x88,0x88,0x88,0xFF,0x88,0x88,0xFF}, // Mosaic separation
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}  // Underline
};

// Double height / weight rules :
// Can't have a double height character in the first line
// Can't have a double width character in the last row.

static uint32_t vdt_draw_char(videotex_ctx * ctx,int x, int y, unsigned char c,uint32_t attributs,uint32_t prevrow_attributs,uint32_t prev_attributs)
{
	int i,j,pix,pix_mask,pix_underline,invert;
	int rom_base,rom_bit_offset,mask_bit_offset;
	uint32_t xfactor,yfactor;
	int xoff;
	int blink;

	unsigned char * mask;
	unsigned char * under_line_mask;
	uint32_t row_left,row_offset;
	uint32_t col_left,col_offset;

	rom_base=0;

	mask = (unsigned char*)&char_mask[0];
	under_line_mask = (unsigned char*)&char_mask[0];

	switch(vdt_get_mask(attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK))
	{
		case 0:
			rom_base = ((ctx->char_res_x_size * 16) * (int)c);

			if(vdt_get_mask(attributs, ATTRIBUTS_UNDERLINE_SHIFT, 1))
				under_line_mask = (unsigned char*)&char_mask[2];

		break;

		case 1:
			if(c >= 0x1F && c <= 0x5F)
				c += 0x20;

			c -= 0x40;
			c |= 0x80;

			rom_base = ((ctx->char_res_x_size * 16) * (int)c);

			if(vdt_get_mask(attributs, ATTRIBUTS_UNDERLINE_SHIFT, 1))
				mask = (unsigned char*)&char_mask[1];

		break;

		case 2:
			rom_base = ((ctx->char_res_x_size * 16) * (int)c);
		break;
	}

	xfactor = vdt_get_mask(attributs, ATTRIBUTS_XZOOM_SHIFT, 1) + 1;
	yfactor = vdt_get_mask(attributs, ATTRIBUTS_YZOOM_SHIFT, 1) + 1;
	invert = vdt_get_mask(attributs, ATTRIBUTS_INVERT_SHIFT, 1);
	blink = vdt_get_mask(attributs, ATTRIBUTS_BLINK_SHIFT, 1);

	row_left = ctx->char_res_y_size;
	row_offset = 0;
	if(yfactor == 2)
	{
		if( (prevrow_attributs >> 24) & 0xF )
		{
			row_offset = ctx->char_res_y_size / 2;
			row_left = ( prevrow_attributs >> 24 ) & 0xF;
		}
		else
		{
			row_left = ctx->char_res_y_size * 2;
		}
	}

	col_left = ctx->char_res_x_size;
	col_offset = 0;
	if(xfactor == 2)
	{
		if( (prev_attributs >> 28) & 0xF )
		{
			col_offset = ctx->char_res_x_size / 2;
			col_left = (prev_attributs >> 28) & 0xF;
		}
		else
		{
			col_left = ctx->char_res_x_size * 2;
		}
	}

	for(j=0;j<ctx->char_res_y_size;j++)
	{
		for(i=0;i<ctx->char_res_x_size;i++)
		{
			rom_bit_offset = rom_base + ((((j/yfactor)+row_offset)*ctx->char_res_x_size) + (col_offset+(i/xfactor)));
			rom_bit_offset %= (192 * 8 * 16);

			mask_bit_offset = (((j/yfactor)*ctx->char_res_x_size) + (col_offset+(i/xfactor)));

			xoff = ( x + i );

			if( ctx->charset[ (rom_bit_offset >> 3) ] & (0x80 >> (rom_bit_offset&7)) )
				pix = 1;
			else
				pix = 0;

			if(blink)
			{
				if(pix)
				{
					pix ^= (ctx->blink_state ^ invert);
				}
			}

			if( mask[ (mask_bit_offset >> 3) ] & (0x80 >> (mask_bit_offset&7)) )
				pix_mask = 0;
			else
				pix_mask = 1;

			if( under_line_mask[ (mask_bit_offset >> 3) ] & (0x80 >> (mask_bit_offset&7)) )
				pix_underline = 1;
			else
				pix_underline = 0;

			if( (((pix | pix_underline) ^ invert) && pix_mask) )
			{
				if( (xoff < ctx->bmp_res_x) && (xoff >= 0) && ((y+j)<ctx->bmp_res_y) && ((y+j)>=0) )
				{
					ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x+i)] = ctx->palette[(attributs>>ATTRIBUTS_FOREGROUND_C0LOR_SHIFT) & ATTRIBUTS_FOREGROUND_C0LOR_MASK];
				}
			}
			else
			{
				if( (xoff < ctx->bmp_res_x) && (xoff >= 0) && ((y+j)<ctx->bmp_res_y) && ((y+j)>=0) )
				{
					ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x+i)] = ctx->palette[(attributs>>ATTRIBUTS_BACKGROUND_C0LOR_SHIFT) & ATTRIBUTS_BACKGROUND_C0LOR_MASK];
				}
			}
		}

		row_left--;
	}

	col_left -= ctx->char_res_x_size;

	return (attributs & 0xFFFFFF) | ((col_left&0xF) << 28) | ((row_left&0xF) << 24);
}

void vdt_render(videotex_ctx * ctx)
{
	int x,y,charindex;
	uint32_t prev_attributs;

	memset(ctx->prev_raw_attributs,0, ctx->char_res_x * sizeof(uint32_t));

	for(y=0;y<ctx->char_res_y;y++)
	{
		x = 0;
		prev_attributs = 0x0000000;
		while(x<ctx->char_res_x)
		{
			charindex = (y*ctx->char_res_x) + x;

			prev_attributs = vdt_draw_char(ctx, x*ctx->char_res_x_size, y*ctx->char_res_y_size, ctx->char_buffer[charindex], ctx->attribut_buffer[charindex], ctx->prev_raw_attributs[x],prev_attributs);

			ctx->prev_raw_attributs[x] = prev_attributs;

			x++;
		}
	}

	ctx->rendered_images_cnt++;

	ctx->framecnt_blink++;
	if( ctx->framecnt_blink >= ctx->framerate )
	{
		ctx->framecnt_blink = 0;
		ctx->blink_state ^= 0x01;
	}
}

static int vdt_move_cursor(videotex_ctx * ctx,int x,int y)
{
	ctx->cursor_x_pos += x;
	if(ctx->cursor_x_pos < 0)
	{
		ctx->cursor_x_pos = ctx->char_res_x + ctx->cursor_x_pos;
		ctx->cursor_y_pos--;
		if( ctx->cursor_y_pos < 1 )
			ctx->cursor_y_pos = ctx->char_res_y - 1;
	}

	if(ctx->cursor_x_pos >= ctx->char_res_x)
	{
		ctx->cursor_x_pos %= ctx->char_res_x;
		ctx->cursor_y_pos++;
		if(ctx->cursor_y_pos >= ctx->char_res_y)
			ctx->cursor_y_pos = 1;
	}

	ctx->cursor_y_pos += y;
	if(ctx->cursor_y_pos < 1)
	{
		ctx->cursor_y_pos = ctx->char_res_y - 1;
	}

	if(ctx->cursor_y_pos >= ctx->char_res_y)
	{
		ctx->cursor_y_pos = 1;
	}

	return 0;
}

static int vdt_push_cursor(videotex_ctx * ctx,int cnt)
{
	 ctx->cursor_x_pos += cnt;

	 if(ctx->cursor_x_pos >= ctx->char_res_x)
	 {
		ctx->cursor_x_pos %= ctx->char_res_x;
		ctx->cursor_y_pos++;

		if( vdt_get_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1) )
			ctx->cursor_y_pos++;

		ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
		ctx->underline_latch = 0;

		if(ctx->cursor_y_pos>=ctx->char_res_y)
		{
			ctx->cursor_y_pos = 1;
			return 1;
		}
	 }

	return 0;
}

static void vdt_print_char(videotex_ctx * ctx,unsigned char c)
{
	int xsize,ysize,x,y;
	int charindex;

	ctx->last_char = c;

	xsize = 1;
	ysize = 1;

	if( ( ctx->cursor_x_pos < ctx->char_res_x - 1 ) && vdt_get_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1) )
		xsize = 2;

	if( ctx->cursor_y_pos && vdt_get_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1) )
		ysize = 2;

	for(x=0;x<xsize;x++)
	{
		for(y=0;y<ysize;y++)
		{
			charindex = ( (ctx->cursor_y_pos-( (ysize-1) - y)) * ctx->char_res_x) + (ctx->cursor_x_pos);

#ifdef DEBUG
			LOG("Print char at x:%d y:%d - 0x%.2X (%c) attributs : 0x%.8X, ",ctx->cursor_x_pos,ctx->cursor_y_pos,c,c,ctx->current_attributs);

			if( ctx->current_attributs & (0x1 << ATTRIBUTS_INVERT_SHIFT) )
				LOG(" INVERT");

			if( ctx->current_attributs & (0x1 << ATTRIBUTS_BLINK_SHIFT) )
				LOG(" BLINK");

			if( ctx->current_attributs & (0x1 << ATTRIBUTS_UNDERLINE_SHIFT) )
				LOG(" UNDERLINE");

			if( ctx->current_attributs & (0x1 << ATTRIBUTS_MASK_SHIFT) )
				LOG(" MASK");

			if( ctx->current_attributs & (0x1 << ATTRIBUTS_XZOOM_SHIFT) )
				LOG(" XZOOM");

			if( ctx->current_attributs & (0x1 << ATTRIBUTS_YZOOM_SHIFT) )
				LOG(" YZOOM");

			LOG(" FONT : 0x%X", (ctx->current_attributs>>ATTRIBUTS_FONT_SHIFT) & ATTRIBUTS_FONT_MASK );
			LOG(" FCOLOR : 0x%X", (ctx->current_attributs>>ATTRIBUTS_FOREGROUND_C0LOR_SHIFT) & ATTRIBUTS_FOREGROUND_C0LOR_MASK );
			LOG(" BCOLOR : 0x%X", (ctx->current_attributs>>ATTRIBUTS_BACKGROUND_C0LOR_SHIFT) & ATTRIBUTS_BACKGROUND_C0LOR_MASK );

			LOG("\n");
#endif

			if(charindex < (ctx->char_res_x*ctx->char_res_y) )
			{
				ctx->char_buffer[charindex] = c;
				ctx->attribut_buffer[charindex] = ctx->current_attributs;
			}
		}

		vdt_push_cursor(ctx,1);
	}
}

static int vdt_fill_line(videotex_ctx * ctx, unsigned char c)
{
	int tmp_x;

	while( ctx->cursor_x_pos < ctx->char_res_x - 1 )
	{
		tmp_x = ctx->cursor_x_pos;
		vdt_print_char(ctx,c);
		if( ctx->cursor_x_pos <= tmp_x )
		{
			return 0;
		}
	};

	vdt_print_char(ctx,c);

	return 0;
}

static void vdt_clear_page(videotex_ctx * ctx)
{
	int i;
	uint32_t attributs;

	vdt_set_cursor(ctx,3,0,1);
	for(i=0;i<(ctx->char_res_x*ctx->char_res_y);i++)
	{
		vdt_print_char(ctx,' ');
	}

	vdt_set_cursor(ctx,3,38,0);

	attributs = ctx->current_attributs;
	vdt_set_cursor(ctx,3,38,0);
	ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x1);
	vdt_push_char(ctx, 'C');
	ctx->current_attributs = attributs;
	vdt_set_cursor(ctx,3,0,1);
}

static unsigned char vdt_convert_pos(videotex_ctx * ctx,unsigned char x,unsigned char y,int ret)
{
	unsigned char x_pos,y_pos;

	if( ( y >= 0x30 ) && (y <= 0x32) )
	{
		x_pos = 0;
		y_pos = ( 10 * (y - 0x30) ) + (x - 0x30);
	}
	else
	{
		x_pos = x - 0x40;
		x_pos--;

		y_pos = y - 0x40;
	}

	if(ret)
		return y_pos;
	else
		return x_pos;
}

void vdt_push_char(videotex_ctx * ctx, unsigned char c)
{
	unsigned int i,tmp;
	unsigned char tmp_char;
	uint32_t next_decoder_state;

	ctx->input_bytes_cnt++;

	switch(ctx->decoder_state)
	{
		case 0x00:

			if( (c>=0x20) && (c<=0x7F) )
			{
				vdt_print_char(ctx,c);

				if(c == ' ' && ctx->underline_latch )
				{
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x1);
					ctx->underline_latch = 0;
				}
			}
			else
			{
				ctx->decoder_step = 0;
				ctx->decoder_buffer[ctx->decoder_step] = c;

				switch(c)
				{
					case 0x00:
						LOG("Null char...\n");
					break;
					case 0x05:
						LOG("Demande d'identification\n");
					break;
					case 0x07:
						LOG("Dring\n");
					break;
					case 0x08:
						vdt_move_cursor(ctx,-1,0);
						LOG("Left (New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);
					break;
					case 0x09:
						vdt_move_cursor(ctx,1,0);
						LOG("Right (New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);
					break;
					case 0x0A:
						vdt_move_cursor(ctx,0,1);
						LOG("Down (New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);
					break;
					case 0x0B:
						vdt_move_cursor(ctx,0,-1);
						LOG("Up (New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);
					break;
					case 0x0C:
						LOG("Page Clear\n");
						vdt_clear_page(ctx);
						vdt_resetstate(ctx);
					break;
					case 0x0D:
						vdt_set_cursor(ctx,1,0,0);
						LOG("Retour chariot (New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);
					break;
					case 0x0E:
						LOG("G1\n");
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x1);
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
						ctx->underline_latch = 0;
					break;
					case 0x0F:
						LOG("G0\n");
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x0);
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
						ctx->underline_latch = 0;
					break;
					case 0x11:
						LOG("Cursor ON\n");
					break;
					case 0x12:
						ctx->decoder_state = 0x12;
					break;
					case 0x13:
						LOG("Special command\n");
						ctx->decoder_state = 0x13;
					break;
					case 0x14:
						LOG("Cursor OFF\n");
					break;
					case 0x16:
						LOG("Tmp G2\n");
						ctx->decoder_state = 0x16;
					break;
					case 0x18:
						tmp = ctx->current_attributs;
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x0);
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
						ctx->underline_latch = 0;
						LOG("Clear end off line\n");
						vdt_fill_line(ctx, ' ');
						ctx->current_attributs = tmp;
					break;
					case 0x19:
						LOG("G2\n");
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x2);
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
						ctx->underline_latch = 0;
					break;
					case 0x1A:
						LOG("Error\n");
					break;
					case 0x1B:
						ctx->decoder_state = 0x1B;
					break;
					case 0x1E:
						vdt_set_cursor(ctx,3,0,1);
						vdt_resetstate(ctx);
						LOG("Home (New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);
					break;
					case 0x1F:
						LOG("Positionnement curseur ");
						ctx->decoder_state = 0x1F;
					break;
					default:
						ERROR("UNSUPPORTED : 0x%.2X\n",c);
					break;
				}
			}

		break;

		case 0x12:
			 i = 0;
			 LOG("Repeat %d times '%C'/0x%.2X\n",(c - 0x40),ctx->last_char,ctx->last_char);
			 while(i<(unsigned char)(c - 0x40))
			 {
				vdt_print_char(ctx,ctx->last_char);
				i++;
			 }
			 ctx->decoder_state = 0x00;
		break;

		case 0x13:
			LOG("Special command 0x%.2X\n",c);
			ctx->decoder_state = 0x00;
		break;

		case 0x16:
			// Special char
			ctx->decoder_step++;
			ctx->decoder_buffer[ctx->decoder_step] = c;

			LOG("Special char 0x%.2X\n",c);

			if( vdt_find_special_char(ctx,(unsigned char*)&ctx->decoder_buffer[1], ctx->decoder_step,&tmp_char) )
			{
				tmp = ctx->current_attributs;
				ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x2);
				vdt_print_char(ctx,tmp_char);
				ctx->current_attributs = tmp;

				ctx->decoder_state = 0x00;
			}

			if(ctx->decoder_step >= 2)
			{
				ctx->decoder_state = 0x00;
			}

		break;

		case 0x1B:

			next_decoder_state = 0x00;

			LOG("Escape %.2X : ",c);

			switch(c)
			{
				case 0x48:
					LOG("Clignotement\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_BLINK_SHIFT, 0x1, 0x1);
				break;

				case 0x49:
					LOG("Fixe\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_BLINK_SHIFT, 0x1, 0x0);
				break;

				case 0x4A:
					LOG("Fin incrustation\n");
				break;

				case 0x4B:
					LOG("Début incrustation\n");
				break;

				case 0x4C:
					LOG("Taille normale\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x0);
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x0);
				break;

				case 0x4D:
					LOG("Double hauteur\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x0);
					if(ctx->cursor_y_pos>1)
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x1);
				break;

				case 0x4E:
					LOG("Double largeur\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x1);
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x0);
				break;

				case 0x4F:
					LOG("Double hauteur et double largeur\n");
					if(ctx->cursor_y_pos>1)
					{
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x1);
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x1);
					}
				break;

				case 0x58:
					LOG("Masquage\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_MASK_SHIFT, 0x1, 0x1);
				break;

				case 0x59:
					LOG("Fin de lignage\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
					ctx->underline_latch = 0;
				break;

				case 0x5A:
					LOG("Début de lignage\n");
					if(vdt_get_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK) == 0)
					{
						ctx->underline_latch = 1;
					}
					else
					{
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x1);
						ctx->underline_latch = 0;
					}
				break;

				case 0x5C:
					LOG("Fond normal\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x0);
				break;

				case 0x5D:
					LOG("Inversion de fond\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x1);
				break;

				case 0x5E:
					LOG("Fond transparent\n");
				break;

				case 0x5F:
					LOG("Démasquage\n");
					ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_MASK_SHIFT, 0x1, 0x0);
				break;

				case 0x61:
					LOG("Demande de position\n");
				break;

				case 0x70:
					LOG("Minitel Photo JPEG image...\n");
					next_decoder_state = 0x70;
					ctx->jpg_rx_cnt = 0;
				break;

				default:
					if(c >= 0x40 && c <= 0x47)
					{
						LOG("Set char color (%x)\n",c);
						ctx->decoder_state = 0x00;
						ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_FOREGROUND_C0LOR_SHIFT, ATTRIBUTS_FOREGROUND_C0LOR_MASK, c);
					}
					else
					{
						if(c >= 0x50 && c <= 0x57)
						{
							LOG("Set background color (%x)\n",c);
							ctx->decoder_state = 0x00;
							ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_BACKGROUND_C0LOR_SHIFT, ATTRIBUTS_BACKGROUND_C0LOR_MASK, c);

							if( vdt_get_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1) && vdt_get_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK) == 0 )
							{
								ctx->current_attributs = vdt_set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
								ctx->underline_latch = 1;
							}
						}
						else
						{
							ERROR("NOT SUPPORTED ! : 0x%.2X\n",c);
						}
					}
				break;
			}

			ctx->decoder_state = next_decoder_state;
		break;
		case 0x1F:
			ctx->decoder_step++;
			ctx->decoder_buffer[ctx->decoder_step] = c;
			if(ctx->decoder_step >= 2)
			{
				vdt_set_cursor(ctx,0x3,vdt_convert_pos(ctx,ctx->decoder_buffer[2],ctx->decoder_buffer[1],0),vdt_convert_pos(ctx,ctx->decoder_buffer[2],ctx->decoder_buffer[1],1));

				LOG("(New pos : x:%d y:%d)\n",ctx->cursor_x_pos,ctx->cursor_y_pos);

				ctx->decoder_state = 0x00;
				vdt_resetstate(ctx);
			}
		break;
		case 0x70:
			// JPEG download...

			// 0x1B 0x70 PICT_MODE FORMAT_ID ?? ?? ?? ?? ?? JPEG_DATA
			switch(ctx->jpg_rx_cnt)
			{
				case 0x0: // MSB Size
					ctx->jpg_size = (uint16_t)(c)<<8;
					ctx->jpg_rx_cnt++;

				break;
				case 0x1: // LSB Size
					ctx->jpg_size |= c;

					if(!ctx->jpg_size)
						ctx->decoder_state = 0x00;

					LOG("JPEG data size : 0x%X...\n",ctx->jpg_size);
					ctx->jpg_rx_cnt++;

				break;
				default:
					// Consume the data...

					ctx->jpg_size--;
					LOG("0x%X... 0x%.2X\n",ctx->jpg_size,c);

					if(!ctx->jpg_size)
						ctx->decoder_state = 0x00;

				break;
			}
		break;
		default:
			ERROR("UNSUPPORTED STATE: 0x%.2X\n",c);
		break;
	}
}

void vdt_deinit(videotex_ctx * ctx)
{
	if(ctx)
	{
		if(ctx->char_buffer)
			free(ctx->char_buffer);

		if(ctx->attribut_buffer)
			free(ctx->attribut_buffer);

		if(ctx->bmp_buffer)
			free(ctx->bmp_buffer);

		free(ctx);
	}
}
