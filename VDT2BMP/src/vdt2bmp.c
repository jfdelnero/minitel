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

Usage example : vdt2bmp -vdt:a_vdt_file.vdt -bmp:OUT.BMP

Available command line options :
	-vdt:file                 : vdt file
	-bmp:file                 : bmp file

*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include <math.h>

#include "videotex.h"
#include "bmp_file.h"

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
	printf("  -bmp:file \t\t\t: bmp file\n");
	printf("  -help \t\t\t: This help\n");
	printf("\n");
}

int main(int argc, char* argv[])
{
	char filename[512];
	char ofilename[512];
	int offset;
	videotex_ctx * vdt_ctx;
	bitmap_data bmp;

	file_cache fc;

	verbose = 0;

	printf("Minitel VDT to BMP converter v1.0.0.0\n");
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

	// help option...
	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}

	memset(filename,0,sizeof(filename));

	// Output file name option
	strcpy(ofilename,"OUT.BMP");
	isOption(argc,argv,"foutput",(char*)&ofilename);

	if(isOption(argc,argv,"vdt",(char*)&filename)>0)
	{
	}

	if(isOption(argc,argv,"bmp",(char*)&ofilename)>0)
	{
		vdt_ctx = init_videotex();
		if(vdt_ctx)
		{
			load_charset(vdt_ctx, "font.bin");

			printf("Write bmp file : %s from %s\n",ofilename,filename);

			if(open_file(&fc, filename,0xFF)>=0)
			{
				offset = 0;
				while(offset<fc.file_size)
				{
					push_char(vdt_ctx, get_byte(&fc,offset, NULL) );
					offset++;
				}

				close_file(&fc);
/*
				c = 0;
				for(j=0;j<vdt_ctx->char_res_y;j++)
				{
					for(i=0;i<vdt_ctx->char_res_x;i++)
					{
						draw_char(vdt_ctx, i*vdt_ctx->char_res_x_size, j*vdt_ctx->char_res_y_size, (c & 0xFF));
						c++;
					}
				}
*/
				render_videotex(vdt_ctx);

				bmp.xsize = vdt_ctx->bmp_res_x;
				bmp.ysize = vdt_ctx->bmp_res_y;
				bmp.data = vdt_ctx->bmp_buffer;
				bmp24b_write(ofilename,&bmp);
			}
			else
			{
				printf("ERROR : Can't open %s !\n",filename);
			}

			deinit_videotex(vdt_ctx);
		}
	}

	if( (isOption(argc,argv,"help",0)<=0) &&
		(isOption(argc,argv,"vdt",0)<=0)  &&
		(isOption(argc,argv,"bmp",0)<=0) )
	{
		printhelp(argv);
	}

	return 0;
}
