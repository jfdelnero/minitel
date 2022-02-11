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
Video Test file (*.vdt) to bmp file converter
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

videotex_ctx * init_videotex()
{
	int i;
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

		ctx->foreground_color = 0x7;
		ctx->background_color = 0x0;

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

void draw_char(videotex_ctx * ctx,int x, int y, unsigned char c,unsigned int attributs)
{
	int i,j;
	int rom_base,rom_bit_offset;
	int font;

	font = (attributs >> 8)&7;

	rom_base=0;

	switch(font)
	{
		case 0:
			if(c >= '@')
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*0;
			else
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*1;
		break;
		case 1:
			if(c >= 0x1F && c <= 0x5F)
				c += 0x20;

			//c -= 0x40;

			if(c >= '@')
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;
			else
				rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;

			//rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;

//           rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*2;
		break;
		case 2:
  //         rom_base = ((ctx->char_res_x_size * /*ctx->char_res_y_size*/ 32) * c) + 80*3;

		break;


	}

	for(j=0;j<ctx->char_res_y_size;j++)
	{
		for(i=0;i<ctx->char_res_x_size;i++)
		{
			rom_bit_offset = rom_base + ((j*ctx->char_res_x_size) + i);
			rom_bit_offset &= ((2048*8)-1);
			if( ctx->charset[ (rom_bit_offset >> 3) ] & (0x80 >> (rom_bit_offset&7)) )
			{
				if( ((x+i)<ctx->bmp_res_x) && ((y+j)<ctx->bmp_res_y) )
				{
					ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x+i)] = ctx->palette[attributs&7];
				}
			}
			else
			{
				if( ((x+i)<ctx->bmp_res_x) && ((y+j)<ctx->bmp_res_y) )
				{
					ctx->bmp_buffer[((y+j)*ctx->bmp_res_x)+(x+i)] = ctx->palette[(attributs>>4)&7];
				}
			}
		}

	}


}

void render_videotex(videotex_ctx * ctx)
{
	int i,j;

	for(j=0;j<ctx->char_res_y;j++)
	{
		for(i=0;i<ctx->char_res_x;i++)
		{
			draw_char(ctx,i*ctx->char_res_x_size,j*ctx->char_res_y_size,ctx->char_buffer[(j*ctx->char_res_x) + i],ctx->attribut_buffer[(j*ctx->char_res_x) + i]);
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
		//printf("%c",c);
		ctx->char_buffer[charindex] = c;
		ctx->attribut_buffer[charindex] = (ctx->foreground_color) | \
										  (ctx->background_color<<4) |
										  (((ctx->font&7)<<8));
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

unsigned char convert_pos(unsigned char x,unsigned char y,int ret)
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

void resetstate(videotex_ctx * ctx)
{
	ctx->foreground_color = 0x7;
	ctx->background_color = 0x0;
	ctx->font = 0;
}

void push_char(videotex_ctx * ctx, unsigned char c)
{
	unsigned int i,tmp;

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
					case 0x05:
						printf("demande d'identification\n");
					break;
					case 0x07:
						printf("Dring\n");
					break;
					case 0x08:
						printf("Left\n");
						move_cursor(ctx,-1,0);
					break;
					case 0x09:
						printf("Right\n");
						move_cursor(ctx,1,0);
					break;
					case 0x0A:
						printf("Down\n");
						move_cursor(ctx,0,1);
					break;
					case 0x0B:
						printf("Up\n");
						move_cursor(ctx,0,-1);
					break;
					case 0x0C:
						printf("Page Clear\n");
						clear_page(ctx);
						resetstate(ctx);
					break;
					case 0x0D:
						printf("Retour chariot\n");
						set_cursor(ctx,1,0,0);
					break;
					case 0x0E:
						printf("G1\n");
						ctx->font = 1;
					break;
					case 0x0F:
						printf("G0\n");
						ctx->font = 0;
					break;
					case 0x11:
						printf("Cursor ON\n");
					break;
					case 0x12:
						printf("Repeat\n");
						ctx->decoder_state = 0x12;
					break;
					case 0x13:
						printf("Special command\n");
						ctx->decoder_state = 0x13;
					break;
					case 0x14:
						printf("Cursor OFF\n");
					break;
					case 0x18:
						tmp = ctx->font;
						ctx->font = 0;
						printf("Clear end off line\n");
						fill_line(ctx, ' ');
						ctx->font = tmp;
					break;
					case 0x19:
						printf("G2\n");
						ctx->font = 2;
					break;
					case 0x1A:
						printf("Error\n");
					break;
					case 0x1B:
						printf("Escape\n");
						ctx->decoder_state = 0x1B;
					break;
					case 0x1E:
						printf("Home\n");
						set_cursor(ctx,3,0,1);
						resetstate(ctx);
					break;
					case 0x1F:
						printf("positionnement curseur\n");
						ctx->decoder_state = 0x1F;
					break;
					default:
						printf("UNSUPPORTED : 0x%.2X\n",c);
					break;
				}
			}

		break;

		case 0x12:
			 i = 0;
			 printf("Repeat %d times\n",(c - 0x40));
			 while(i<(unsigned char)(c - 0x40))
			 {
				print_char(ctx,ctx->last_char);
				i++;
			 }
			 ctx->decoder_state = 0x00;
		break;

		case 0x13:
			printf("Special command 0x%.2X\n",c);
			ctx->decoder_state = 0x00;
		break;

		case 0x1B:
			printf("Escape %.2X\n",c);

			switch(c)
			{
				case 0x48:
					printf("clignotement\n");
				break;

				case 0x49:
					printf("fixe\n");
				break;

				case 0x4A:
					printf("fin incrustation\n");
				break;

				case 0x4B:
					printf("début incrustation\n");
				break;

				case 0x4C:
					printf("taille normale\n");
				break;

				case 0x4D:
					printf("double hauteur\n");
				break;

				case 0x4E:
					printf("double largeur\n");
				break;

				case 0x4F:
					printf("double hauteur et double largeur\n");
				break;

				case 0x58:
					printf("masquage\n");
				break;

				case 0x59:
					printf("fin de lignage\n");
				break;

				case 0x5A:
					printf("début de lignage\n");
				break;

				case 0x5C:
					printf("fond normal\n");
				break;

				case 0x5D:
					printf("inversion de fond\n");
				break;

				case 0x5E:
					printf("fond transparent\n");
				break;

				case 0x5F:
					printf("démasquage\n");
				break;

				case 0x61:
					printf("demande de position\n");
				break;

				default:
					if(c >= 0x40 && c <= 0x47)
					{
						printf("Set char color (%x)\n",c);
						ctx->decoder_state = 0x00;
						ctx->foreground_color = c&7;

					}
					else
					{
						if(c >= 0x50 && c <= 0x57)
						{
							printf("Set background color (%x)\n",c);
							ctx->decoder_state = 0x00;

							ctx->background_color = c&7;
						}
						else
						{
							printf("NOT SUPPORTED ! : 0x%.2X\n",c);
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
				set_cursor(ctx,0x3,convert_pos(ctx->decoder_buffer[2],ctx->decoder_buffer[1],0),convert_pos(ctx->decoder_buffer[2],ctx->decoder_buffer[1],1));

				ctx->decoder_state = 0x00;
				resetstate(ctx);
			}
		break;
		default:
			printf("UNSUPPORTED STATE: 0x%.2X\n",c);
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

