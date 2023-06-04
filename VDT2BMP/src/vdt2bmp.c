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

#include "cache.h"
#include "videotex.h"
#include "bmp_file.h"
#include "modem.h"

#define DEFAULT_SOUND_FILE "out_audio.wav"

int verbose;

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	memset(option,0,sizeof(option));

	while(param<=argc)
	{
		if(argv[param])
		{
			if(argv[param][0]=='-')
			{
				memset(option,0,sizeof(option));

				j=0;
				i=1;
				while( argv[param][i] && argv[param][i]!=':' && ( j < (sizeof(option) - 1)) )
				{
					option[j]=argv[param][i];
					i++;
					j++;
				}

				if( !strcmp(option,paramtosearch) )
				{
					if(argtoparam)
					{
						argtoparam[0] = 0;

						if(argv[param][i]==':')
						{
							i++;
							j=0;
							while( argv[param][i] && j < (512 - 1) )
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
	fprintf(stderr,"  animation: vdt2bmp -ani -fps:30 -stdout /path/*.vdt | ffmpeg -y -f rawvideo -pix_fmt argb -s 320x250 -r 30 -i - -an out_video.mkv\n");
	fprintf(stderr,"  video + audio merging : ffmpeg -i out_video.mkv -i out_audio.wav -c copy output.mkv\n");	
	fprintf(stderr,"  vdt to bmp convert : vdt2bmp -bmp /path/*.vdt\n");
	fprintf(stderr,"  vdt to bmp convert : vdt2bmp -bmp:out.bmp /path/videotex.vdt\n");
	fprintf(stderr,"\n");
}

int animate(videotex_ctx * vdt_ctx, modem_ctx *mdm, char * vdtfile,float framerate, int nb_pause_frames, int stdout_mode)
{
	char ofilename[512];
	int offset,i,totalsndsmp,next_pic_sndsmp,totalneededsamples;
	bitmap_data bmp;
	int image_nb,pause_frames_cnt;
	file_cache fc;
	unsigned char * tmp_buf;
	image_nb = 0;

	pause_frames_cnt = 0;
	next_pic_sndsmp = 0;

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

		vdt_ctx->pages_cnt++;

		mdm->next_bitlimit = mdm->bit_time;
		mdm->sample_offset = 0;

		totalsndsmp = 0;
		offset = 0;
		while(offset<fc.file_size || (pause_frames_cnt <= nb_pause_frames) )
		{
			if(offset<fc.file_size)
			{
				unsigned char c;

				c = get_byte( &fc, offset, NULL );

				mdm->tx_buffer_size = prepare_next_word( mdm, (int*)mdm->tx_buffer, c );
				BitStreamToWave( mdm );
				push_char( vdt_ctx, c );
				write_wave_file(DEFAULT_SOUND_FILE,mdm->wave_buf,mdm->wave_size,mdm->sample_rate);
			}
			else
			{
				for(i=0;i<64;i++)
				{
					mdm->tx_buffer[i] = 1;
				}
				BitStreamToWave( mdm );
				write_wave_file(DEFAULT_SOUND_FILE,mdm->wave_buf,mdm->wave_size,mdm->sample_rate);
			}

			totalsndsmp += mdm->sample_offset;

			if( totalsndsmp >= next_pic_sndsmp )
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

				next_pic_sndsmp = (int)(((double)mdm->sample_rate / (double)framerate) * image_nb);
			}
			offset++;
		}

		// Pad sound buffer...
		totalneededsamples = (int)( ( (float)image_nb / (float)framerate) * (float)mdm->sample_rate);
		while( totalsndsmp < totalneededsamples )
		{
			if( (totalneededsamples - totalsndsmp) > 64 )
			{
				mdm->wave_size = FillWaveBuff(mdm, 64,0);
				totalsndsmp += 64;
			}
			else
			{
				mdm->wave_size = FillWaveBuff(mdm, (totalneededsamples - totalsndsmp),0);
				totalsndsmp += (totalneededsamples - totalsndsmp);
			}
			write_wave_file(DEFAULT_SOUND_FILE,mdm->wave_buf,mdm->wave_size,mdm->sample_rate);
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

		load_charset(vdt_ctx,  NULL);

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
	char strtmp[512];
	modem_ctx mdm_ctx;
	int pal,stdoutmode;
	int i;
	float framerate;
	videotex_ctx * vdt_ctx;

	verbose = 0;
	stdoutmode = 0;
	framerate = 30.0;

	fprintf(stderr,"Minitel VDT to BMP converter v1.2.0.0\n");
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

	if(isOption(argc,argv,"fps",(char*)&strtmp))
	{
		framerate = atof(strtmp);
	}

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
		init_modem(&mdm_ctx);

		vdt_ctx = init_videotex();
		if(vdt_ctx)
		{
			vdt_ctx->framerate = framerate;

			if(isOption(argc,argv,"greyscale",NULL)>0)
			{
				select_palette(vdt_ctx,0);
			}

			load_charset(vdt_ctx, NULL);

			remove(DEFAULT_SOUND_FILE);

			i = 1;
			while( i < argc)
			{
				if(argv[i][0] != '-')
				{
					animate(vdt_ctx,&mdm_ctx,argv[i],framerate,(int)(framerate*4),stdoutmode);
				}
				i++;
			}

			fprintf(stderr,"Videotex pages count : %d \n",vdt_ctx->pages_cnt);
			fprintf(stderr,"Videotex input bytes count : %d \n",vdt_ctx->input_bytes_cnt);
			fprintf(stderr,"Rendered images : %d \n",vdt_ctx->rendered_images_cnt);
			fprintf(stderr,"Frame rate : %.2f fps \n",framerate);
			fprintf(stderr,"Duration : %f ms \n",(float)((float)(vdt_ctx->rendered_images_cnt*1000)/(float)framerate));
			fprintf(stderr,"Sound samples count : %d \n",mdm_ctx.samples_generated_cnt);
			fprintf(stderr,"Sound duration : %f ms \n",((float)mdm_ctx.samples_generated_cnt/(float)mdm_ctx.sample_rate)*1000);
			fprintf(stderr,"Video duration - Sound duration diff : %f ms \n",((float)((float)(vdt_ctx->rendered_images_cnt*1000)/(float)framerate)) - (((float)mdm_ctx.samples_generated_cnt/(float)mdm_ctx.sample_rate)*1000));

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
