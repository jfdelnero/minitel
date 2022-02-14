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
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include <stdint.h>

#include "videotex.h"

// Minitel resolution :
// 40x25 characters
// 8x10 character size
// 320*250 screen resolution
//#define DEBUG 1

#ifdef DEBUG
#define LOG(...) printf(__VA_ARGS__)
#else
#define LOG(...)
#endif

#define ERROR(...) printf(__VA_ARGS__)

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

	{ 0x00, 0x00, 0x00, 0x00 }
};

int find_special_char(videotex_ctx * ctx,unsigned char * code, int size,unsigned char * charcode)
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

unsigned int set_mask(unsigned int reg, int shift, unsigned int mask, unsigned int val)
{
	return ( ( reg & ~(mask << shift) ) | ((val & mask) << shift) );
}

unsigned int get_mask(unsigned int reg, int shift, unsigned int mask)
{
	return ( (reg >> shift) & mask);
}

void resetstate(videotex_ctx * ctx)
{
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FOREGROUND_C0LOR_SHIFT, ATTRIBUTS_FOREGROUND_C0LOR_MASK, 7);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_BACKGROUND_C0LOR_SHIFT, ATTRIBUTS_BACKGROUND_C0LOR_MASK, 0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_BLINK_SHIFT, 0x1, 0x0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
	ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_MASK_SHIFT, 0x1, 0x0);
}

void select_palette(videotex_ctx * ctx, int pal_id)
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

videotex_ctx * init_videotex()
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

		resetstate(ctx);

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

		memset(ctx->bmp_buffer,0x29,ctx->bmp_res_x * ctx->bmp_res_y * sizeof(uint32_t) );

		select_palette(ctx, 1);

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

		free(ctx);
	}

	return NULL;
}

int load_charset(videotex_ctx * ctx, char * file)
{
	FILE * f;
	int size,ret;

	ret = 0;

	if(ctx)
	{
		f = fopen(file,"rb");
		if(f)
		{
			fseek(f,0,SEEK_END);

			size = ftell(f);

			fseek(f,0,SEEK_SET);

			if(size)
			{
				if(ctx->charset)
					free(ctx->charset);

				ctx->charset = malloc(size);
				if(ctx->charset)
				{
					ret = fread(ctx->charset,size,1,f);
				}
			}

			fclose(f);
		}
	}

	return ret;
}

static unsigned char char_mask[][10]=
{
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, // Normal
	{0x88,0x88,0xFF,0x88,0x88,0x88,0xFF,0x88,0x88,0xFF}, // Mosaic separation
	{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF}  // Underline
};


int draw_char(videotex_ctx * ctx,int x, int y, unsigned char c,unsigned int attributs)
{
	int i,j,pix,pix_mask,pix_underline,invert;
	int rom_base,rom_bit_offset,mask_bit_offset;
	int xfactor,yfactor;
	int xoff;
	rom_base=0;
	unsigned char * mask;
	unsigned char * under_line_mask;

	mask = (unsigned char*)&char_mask[0];
	under_line_mask = (unsigned char*)&char_mask[0];

	switch(get_mask(attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK))
	{
		case 0:
			if(c >= '@')
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*0;
			else
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*1;

			//if(get_mask(attributs, ATTRIBUTS_UNDERLINE_SHIFT, 1))
			//	under_line_mask = (unsigned char*)&char_mask[2];

		break;
		case 1:
			if(c >= 0x1F && c <= 0x5F)
				c += 0x20;

			//c -= 0x40;

			if(c >= '@')
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;
			else
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;

			if(get_mask(attributs, ATTRIBUTS_UNDERLINE_SHIFT, 1))
				mask = (unsigned char*)&char_mask[1];

			//rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;

//           rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;
		break;
		case 2:
			rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*1;
		break;


	}

	xfactor = get_mask(attributs, ATTRIBUTS_XZOOM_SHIFT, 1) + 1;
	yfactor = get_mask(attributs, ATTRIBUTS_YZOOM_SHIFT, 1) + 1;
	invert = get_mask(attributs, ATTRIBUTS_INVERT_SHIFT, 1);

	if(yfactor == 2)
		y = y - ctx->char_res_y_size;

	for(j=0;j<ctx->char_res_y_size*yfactor;j++)
	{
		for(i=0;i<ctx->char_res_x_size;i++)
		{
			rom_bit_offset = rom_base + (((j/yfactor)*ctx->char_res_x_size) + i);
			rom_bit_offset &= ((2048*8)-1);

			mask_bit_offset = (((j/yfactor)*ctx->char_res_x_size) + i);

			xoff = ((x+(i*xfactor)+(xfactor-1)));

			if( ctx->charset[ (rom_bit_offset >> 3) ] & (0x80 >> (rom_bit_offset&7)) )
				pix = 1;
			else
				pix = 0;

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
					ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x+(i*xfactor))] = ctx->palette[(attributs>>ATTRIBUTS_FOREGROUND_C0LOR_SHIFT) & ATTRIBUTS_FOREGROUND_C0LOR_MASK];
					if(xfactor==2)
						ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x + (i*xfactor) + 1)] = ctx->palette[(attributs>>ATTRIBUTS_FOREGROUND_C0LOR_SHIFT) & ATTRIBUTS_FOREGROUND_C0LOR_MASK];
				}
			}
			else
			{
				if( (xoff < ctx->bmp_res_x) && (xoff >= 0) && ((y+j)<ctx->bmp_res_y) && ((y+j)>=0) )
				{
					ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x+(i*xfactor))] = ctx->palette[(attributs>>ATTRIBUTS_BACKGROUND_C0LOR_SHIFT) & ATTRIBUTS_BACKGROUND_C0LOR_MASK];
					if(xfactor==2)
						ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x + (i*xfactor) + 1)] = ctx->palette[(attributs>>ATTRIBUTS_BACKGROUND_C0LOR_SHIFT) & ATTRIBUTS_BACKGROUND_C0LOR_MASK];
				}
			}
		}

	}

	return  (ctx->char_res_x_size * xfactor);
}

void render_videotex(videotex_ctx * ctx)
{
	int i,j;
	int xpos;

	for(j=0;j<ctx->char_res_y;j++)
	{
		xpos = 0;
		for(i=0;i<ctx->char_res_x;i++)
		{
			xpos += draw_char(ctx,xpos,j*ctx->char_res_y_size,ctx->char_buffer[(j*ctx->char_res_x) + i],ctx->attribut_buffer[(j*ctx->char_res_x) + i]);
		}
	}
}

int move_cursor(videotex_ctx * ctx,int x,int y)
{
	ctx->cursor_x_pos += x;
	if(ctx->cursor_x_pos < 0)
	{
		ctx->cursor_x_pos = ctx->char_res_x + ctx->cursor_x_pos;
		ctx->cursor_y_pos--;
		if(ctx->cursor_y_pos<0)
			ctx->cursor_y_pos = 0;
	}

	if(ctx->cursor_x_pos >= ctx->char_res_x)
	{
		ctx->cursor_x_pos %= ctx->char_res_x;
		ctx->cursor_y_pos++;
		if(ctx->cursor_y_pos >= ctx->char_res_y)
			ctx->cursor_y_pos = ctx->char_res_y - 1;
	}

	ctx->cursor_y_pos += y;
	if(ctx->cursor_x_pos < 0)
	{
	   ctx->cursor_y_pos = 0;
	}

	if(ctx->cursor_x_pos >= ctx->char_res_y)
	{
	   ctx->cursor_y_pos = ctx->char_res_y - 1;
	}

	return 0;
}

int set_cursor(videotex_ctx * ctx,unsigned mask,int x,int y)
{
	if(mask&1)
		ctx->cursor_x_pos = x;

	if(mask&2)
		ctx->cursor_y_pos = y;

	return 0;
}

int push_cursor(videotex_ctx * ctx,int cnt)
{
	 ctx->cursor_x_pos += cnt;

	 if(ctx->cursor_x_pos >= ctx->char_res_x)
	 {
		ctx->cursor_x_pos %= ctx->char_res_x;
		ctx->cursor_y_pos++;

		if(ctx->cursor_y_pos>=ctx->char_res_y)
		{
			ctx->cursor_y_pos = ctx->char_res_y - 1;
			return 1;
		}
	 }

	return 0;
}

void print_char(videotex_ctx * ctx,unsigned char c)
{
	int charindex = (ctx->cursor_y_pos*ctx->char_res_x) + ctx->cursor_x_pos;

	ctx->last_char = c;

	if(charindex < (ctx->char_res_x*ctx->char_res_y) )
	{
		ctx->char_buffer[charindex] = c;

		ctx->attribut_buffer[charindex] = ctx->current_attributs;
	}

	push_cursor(ctx,1);
}

int fill_line(videotex_ctx * ctx, unsigned char c)
{
	while( ctx->cursor_x_pos < ctx->char_res_x - 1 )
	{
		print_char(ctx,c);
	};

	print_char(ctx,c);

	return 0;
}

void clear_page(videotex_ctx * ctx)
{
	int i;

	set_cursor(ctx,3,0,0);
	for(i=0;i<(ctx->char_res_x*ctx->char_res_y);i++)
	{
		print_char(ctx,' ');
	}

	set_cursor(ctx,3,0,0);
}

unsigned char convert_pos(videotex_ctx * ctx,unsigned char x,unsigned char y,int ret)
{
	unsigned char x_pos,y_pos;
	int charindex,i,cnt_dbl;

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

	cnt_dbl = 0;
	if(y_pos < ctx->char_res_y)
	{
		charindex = (y_pos*ctx->char_res_x);

		i = 0;
		while(i<x_pos && i <ctx->char_res_x)
		{
			if( get_mask(ctx->attribut_buffer[charindex+i], ATTRIBUTS_XZOOM_SHIFT, 1))
			{
				cnt_dbl++;
			}
			i++;
		}
	}

	if(ret)
		return y_pos;
	else
		return x_pos - cnt_dbl;
}

void push_char(videotex_ctx * ctx, unsigned char c)
{
	unsigned int i,tmp;
	unsigned char tmp_char;

	switch(ctx->decoder_state)
	{
		case 0x00:

			if( (c>=0x20) && (c<=0x7F) )
			{
				print_char(ctx,c);
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
						LOG("demande d'identification\n");
					break;
					case 0x07:
						LOG("Dring\n");
					break;
					case 0x08:
						LOG("Left\n");
						move_cursor(ctx,-1,0);
					break;
					case 0x09:
						LOG("Right\n");
						move_cursor(ctx,1,0);
					break;
					case 0x0A:
						LOG("Down\n");
						move_cursor(ctx,0,1);
					break;
					case 0x0B:
						LOG("Up\n");
						move_cursor(ctx,0,-1);
					break;
					case 0x0C:
						LOG("Page Clear\n");
						clear_page(ctx);
						resetstate(ctx);
					break;
					case 0x0D:
						LOG("Retour chariot\n");
						set_cursor(ctx,1,0,0);
					break;
					case 0x0E:
						LOG("G1\n");
						ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x1);
					break;
					case 0x0F:
						LOG("G0\n");
						ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x0);
					break;
					case 0x11:
						LOG("Cursor ON\n");
					break;
					case 0x12:
						LOG("Repeat\n");
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
						ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x0);
						LOG("Clear end off line\n");
						fill_line(ctx, ' ');
						ctx->current_attributs = tmp;
					break;
					case 0x19:
						LOG("G2\n");
						ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x2);
					break;
					case 0x1A:
						LOG("Error\n");
					break;
					case 0x1B:
						LOG("Escape\n");
						ctx->decoder_state = 0x1B;
					break;
					case 0x1E:
						LOG("Home\n");
						set_cursor(ctx,3,0,1);
						resetstate(ctx);
					break;
					case 0x1F:
						LOG("positionnement curseur\n");
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
			 LOG("Repeat %d times\n",(c - 0x40));
			 while(i<(unsigned char)(c - 0x40))
			 {
				print_char(ctx,ctx->last_char);
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

			if(find_special_char(ctx,(unsigned char*)&ctx->decoder_buffer[1], ctx->decoder_step,&tmp_char))
			{
				tmp = ctx->current_attributs;
				ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FONT_SHIFT, ATTRIBUTS_FONT_MASK, 0x2);
				print_char(ctx,tmp_char);
				ctx->current_attributs = tmp;

				ctx->decoder_state = 0x00;
			}

			if(ctx->decoder_step >= 2)
			{
				ctx->decoder_state = 0x00;
			}

		break;

		case 0x1B:
			LOG("Escape %.2X\n",c);

			switch(c)
			{
				case 0x48:
					LOG("clignotement\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_BLINK_SHIFT, 0x1, 0x1);
				break;

				case 0x49:
					LOG("fixe\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_BLINK_SHIFT, 0x1, 0x0);
				break;

				case 0x4A:
					LOG("fin incrustation\n");
				break;

				case 0x4B:
					LOG("début incrustation\n");
				break;

				case 0x4C:
					LOG("taille normale\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x0);
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x0);
				break;

				case 0x4D:
					LOG("double hauteur\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x0);
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x1);
				break;

				case 0x4E:
					LOG("double largeur\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x1);
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x0);
				break;

				case 0x4F:
					LOG("double hauteur et double largeur\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_XZOOM_SHIFT, 0x1, 0x1);
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_YZOOM_SHIFT, 0x1, 0x1);
				break;

				case 0x58:
					LOG("masquage\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_MASK_SHIFT, 0x1, 0x1);
				break;

				case 0x59:
					LOG("fin de lignage\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x0);
				break;

				case 0x5A:
					LOG("début de lignage\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_UNDERLINE_SHIFT, 0x1, 0x1);
				break;

				case 0x5C:
					LOG("fond normal\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x0);
				break;

				case 0x5D:
					LOG("inversion de fond\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_INVERT_SHIFT, 0x1, 0x1);
				break;

				case 0x5E:
					LOG("fond transparent\n");
				break;

				case 0x5F:
					LOG("démasquage\n");
					ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_MASK_SHIFT, 0x1, 0x0);
				break;

				case 0x61:
					LOG("demande de position\n");
				break;

				default:
					if(c >= 0x40 && c <= 0x47)
					{
						LOG("Set char color (%x)\n",c);
						ctx->decoder_state = 0x00;
						ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_FOREGROUND_C0LOR_SHIFT, ATTRIBUTS_FOREGROUND_C0LOR_MASK, c);
					}
					else
					{
						if(c >= 0x50 && c <= 0x57)
						{
							LOG("Set background color (%x)\n",c);
							ctx->decoder_state = 0x00;
							ctx->current_attributs = set_mask(ctx->current_attributs, ATTRIBUTS_BACKGROUND_C0LOR_SHIFT, ATTRIBUTS_BACKGROUND_C0LOR_MASK, c);
						}
						else
						{
							ERROR("NOT SUPPORTED ! : 0x%.2X\n",c);
						}
					}
				break;
			}
			ctx->decoder_state = 0x00;
		break;
		case 0x1F:
			ctx->decoder_step++;
			ctx->decoder_buffer[ctx->decoder_step] = c;
			if(ctx->decoder_step >= 2)
			{
				set_cursor(ctx,0x3,convert_pos(ctx,ctx->decoder_buffer[2],ctx->decoder_buffer[1],0),convert_pos(ctx,ctx->decoder_buffer[2],ctx->decoder_buffer[1],1));

				ctx->decoder_state = 0x00;
				resetstate(ctx);
			}
		break;
		default:
			ERROR("UNSUPPORTED STATE: 0x%.2X\n",c);
		break;

	}
}

void deinit_videotex(videotex_ctx * ctx)
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

