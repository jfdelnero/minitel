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
// along with vdt2wav; if not, write to the Free Software
// Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
//
*/

/*
Videotex file (*.vdt) to bmp file converter
(C) 2022-2023 Jean-François DEL NERO

Usage example : vdt2bmp -bmp:OUT.BMP a_vdt_file.vdt

Available command line options :
	a_vdt_file.vdt            : vdt file(s)
	-bmp[:file]               : Generate bmp(s) file(s)
	-ani                      : Simulate Minitel page loading.
	-stdout                   : stdout mode (for piped ffmpeg compression)

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
	fprintf(stderr,"Options:\n");
	fprintf(stderr,"  -bmp[:out_file.bmp] \t\t: generate bmp file(s)\n");
	fprintf(stderr,"  -ani\t\t\t\t: generate animation\n");
	fprintf(stderr,"  -stdout \t\t\t: stdout mode\n");
	fprintf(stderr,"  -help \t\t\t: This help\n");
	fprintf(stderr,"  \nExamples :\n");
	fprintf(stderr,"  animation: vdt2bmp -ani -stdout /path/*.vdt | ffmpeg -y -f rawvideo -pix_fmt argb -s 320x250 -r 50 -i - -an out_vid.mkv\n");
	fprintf(stderr,"  vdt to bmp convert : vdt2bmp -bmp /path/*.vdt\n");
	fprintf(stderr,"  vdt to bmp convert : vdt2bmp -bmp:out.bmp /path/videotex.vdt\n");
	fprintf(stderr,"\n");
}

int animate(videotex_ctx * vdt_ctx, char * vdtfile,int frameperiod_us, int nb_pause_frames, int stdout_mode)
{
	char ofilename[512];
	int offset,i;
	bitmap_data bmp;
	int image_nb,pause_frames_cnt;
	uint32_t ani_time_us;
	uint32_t next_pic_timeout_us;
	file_cache fc;
	unsigned char * tmp_buf;
	image_nb = 0;
	next_pic_timeout_us = 0;
	ani_time_us = 0;
	pause_frames_cnt = 0;

	fprintf(stderr,"Generate animation from %s...\n",vdtfile);

	memset(&fc,0,sizeof(file_cache));
	tmp_buf = NULL;

	if(open_file(&fc, vdtfile,0xFF)>=0)
	{
		if(stdout_mode)
		{
			tmp_buf = malloc(vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
			if(!tmp_buf)
				goto alloc_error;

			memset(tmp_buf,0,vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
		}

		offset = 0;
		while(offset<fc.file_size || (pause_frames_cnt <= nb_pause_frames) )
		{
			if(offset<fc.file_size)
			{
				push_char(vdt_ctx, get_byte(&fc,offset, NULL) );
			}

			ani_time_us += 8333; // 8.333 ms (1 / (1200 Baud / 10))

			if( ani_time_us >= next_pic_timeout_us )
			{
				if(!stdout_mode)
				{
					sprintf(ofilename,"I%.6d.bmp",image_nb);
					fprintf(stderr,"Write bmp file : %s...\n",ofilename);
				}

				//write image...
				render_videotex(vdt_ctx);
				bmp.xsize = vdt_ctx->bmp_res_x;
				bmp.ysize = vdt_ctx->bmp_res_y;
				bmp.data = vdt_ctx->bmp_buffer;

				if(!stdout_mode)
					bmp24b_write(ofilename,&bmp);
				else
				{
					for(i=0;i<(bmp.xsize * bmp.ysize);i++)
					{
						tmp_buf[(i*4)+1] = bmp.data[i] & 0xFF;
						tmp_buf[(i*4)+2] = (bmp.data[i]>>8) & 0xFF;
						tmp_buf[(i*4)+3] = (bmp.data[i]>>16) & 0xFF;
					}
					fwrite(tmp_buf, bmp.xsize * bmp.ysize * 4, 1, stdout);
				}

				image_nb++;

				if(!(offset<fc.file_size))
					pause_frames_cnt++;

				next_pic_timeout_us += frameperiod_us;
			}
			offset++;
		}

		if(tmp_buf)
			free(tmp_buf);

		close_file(&fc);
	}
	else
	{
		fprintf(stderr,"ERROR : Can't open %s !\n",vdtfile);
	}


	return 0;

alloc_error:
	if(vdt_ctx)
		deinit_videotex(vdt_ctx);

	if(tmp_buf)
		free(tmp_buf);

	close_file(&fc);

	return -3;
}

int write_bmp(char * vdt_file, char * bmp_file, int pal, int stdout_mode)
{
	char ofilename[512];
	int offset;
	int i;
	videotex_ctx * vdt_ctx;
	unsigned char * tmp_buf;
	bitmap_data bmp;
	file_cache fc;
	int ret;

	memset(&fc,0,sizeof(file_cache));
	ret = 0;
	tmp_buf = NULL;

	vdt_ctx = init_videotex();
	if(vdt_ctx)
	{
		if(stdout_mode)
		{
			tmp_buf = malloc(vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
			if(!tmp_buf)
				goto alloc_error;

			memset(tmp_buf,0,vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
		}

		select_palette(vdt_ctx,pal);

		load_charset(vdt_ctx, "font.bin");

		if(!stdout_mode)
		{
			if(strlen(bmp_file))
			{
				strcpy(ofilename,bmp_file);
			}
			else
			{
				strcpy(ofilename,vdt_file);
				i = 0;
				while(ofilename[i])
				{
					if(ofilename[i] == '.')
						ofilename[i] = '_';

					i++;
				}
				strcat(ofilename,".bmp");
			}

			fprintf(stderr,"Write bmp file : %s from %s\n",ofilename,vdt_file);
		}
		else
		{
			fprintf(stderr,"Write raw data to stdout from %s\n", vdt_file);
		}

		if(open_file(&fc, vdt_file,0xFF)>=0)
		{
			offset = 0;
			while(offset<fc.file_size)
			{
				push_char(vdt_ctx, get_byte(&fc,offset, NULL) );
				offset++;
			}

			close_file(&fc);

			render_videotex(vdt_ctx);

			bmp.xsize = vdt_ctx->bmp_res_x;
			bmp.ysize = vdt_ctx->bmp_res_y;
			bmp.data = vdt_ctx->bmp_buffer;

			if(!stdout_mode)
				bmp24b_write(ofilename,&bmp);
			else
			{
				for(i=0;i<(bmp.xsize * bmp.ysize);i++)
				{
					tmp_buf[(i*4)+1] = bmp.data[i] & 0xFF;
					tmp_buf[(i*4)+2] = (bmp.data[i]>>8) & 0xFF;
					tmp_buf[(i*4)+3] = (bmp.data[i]>>16) & 0xFF;
				}
				fwrite(tmp_buf, bmp.xsize * bmp.ysize * 4, 1, stdout);
			}
		}
		else
		{
			fprintf(stderr,"ERROR : Can't open %s !\n",vdt_file);
			ret = -1;
		}

		if(tmp_buf)
			free(tmp_buf);

		deinit_videotex(vdt_ctx);
	}
	else
	{
			ret = -2;
	}

	return ret;

alloc_error:
	if(vdt_ctx)
		deinit_videotex(vdt_ctx);

	if(tmp_buf)
		free(tmp_buf);

	close_file(&fc);

	return -3;
}

int main(int argc, char* argv[])
{
	char filename[512];
	char ofilename[512];

	int pal,stdoutmode;
	int i;
	videotex_ctx * vdt_ctx;

	verbose = 0;
	stdoutmode = 0;

	fprintf(stderr,"Minitel VDT to BMP converter v1.1.0.0\n");
	fprintf(stderr,"Copyright (C) 2022-2023 Jean-Francois DEL NERO\n");
	fprintf(stderr,"This program comes with ABSOLUTELY NO WARRANTY\n");
	fprintf(stderr,"This is free software, and you are welcome to redistribute it\n");
	fprintf(stderr,"under certain conditions;\n\n");

	// Verbose option...
	if(isOption(argc,argv,"verbose",0)>0)
	{
		fprintf(stderr,"verbose mode\n");
		verbose=1;
	}

	// Verbose option...
	if(isOption(argc,argv,"stdout",0)>0)
	{
		fprintf(stderr,"sdtout mode\n");
		stdoutmode = 1;
	}

	// help option...
	if(isOption(argc,argv,"help",0)>0)
	{
		printhelp(argv);
	}

	memset(filename,0,sizeof(filename));

	// Output file name option
	strcpy(ofilename,"");

	if(isOption(argc,argv,"bmp",(char*)&ofilename))
	{
		pal = 1;
		if(isOption(argc,argv,"greyscale",NULL)>0)
		{
			pal = 0;
		}

		i = 1;
		while( i < argc)
		{
			if(argv[i][0] != '-')
			{
				write_bmp(argv[i], ofilename, pal,stdoutmode);
			}

			i++;
		}
	}

	// Animation generator
	if(isOption(argc,argv,"ani",NULL)>0)
	{
		vdt_ctx = init_videotex();
		if(vdt_ctx)
		{
			if(isOption(argc,argv,"greyscale",NULL)>0)
			{
				select_palette(vdt_ctx,0);
			}

			load_charset(vdt_ctx, "font.bin");

			i = 1;
			while( i < argc)
			{
				if(argv[i][0] != '-')
				{
					animate(vdt_ctx,argv[i],20000,50*1,stdoutmode);
				}
				i++;
			}
			deinit_videotex(vdt_ctx);
		}
	}

	if( (isOption(argc,argv,"help",0)<=0) &&
		(isOption(argc,argv,"vdt",0)<=0)  &&
		(isOption(argc,argv,"ani",0)<=0)  &&
		(isOption(argc,argv,"bmp",0)<=0) )
	{
		printhelp(argv);
	}

	return 0;
}
