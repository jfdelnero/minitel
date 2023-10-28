/*
//
// Copyright (C) 2022-2023 Jean-Fran�ois DEL NERO
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
(C) 2022-2023 Jean-Fran�ois DEL NERO

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
#include <unistd.h>

#include <math.h>

#include <errno.h>
#include <sys/time.h>
#include <pthread.h>

#ifdef WIN32
#include <windows.h>
#endif

#ifdef SDL_SUPPORT
#include <SDL2/SDL.h>
#include <SDL2/SDL_audio.h>
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/threading.h>
#include <atomic_arch.h>
#endif

#include "env.h"

#include "cache.h"
#include "videotex.h"
#include "bmp_file.h"
#include "modem.h"

#include "FIR/band_pass_rx_Filter.h"
#include "FIR/low_pass_tx_Filter.h"

#include "data/vdt_tests.h"

#include "vdt2bmp.h"

#include "script_exec.h"

#include "version.h"

#define DEFAULT_SOUND_FILE "out_audio.wav"

#ifdef SDL_SUPPORT

#ifdef DBG_INJECT_SND
FILE * indbgfile;
FILE * outdbgfile;
#endif

EVENT_HANDLE * createevent()
{
	EVENT_HANDLE* theevent;
	theevent=(EVENT_HANDLE*)malloc(sizeof(EVENT_HANDLE));
#ifdef EMSCRIPTEN_SUPPORT
	emscripten_atomic_store_u32(&theevent->futex, 0);
#else
	pthread_mutex_init(&theevent->eMutex, NULL);
	pthread_cond_init(&theevent->eCondVar, NULL);
#endif
	return theevent;
}

void setevent(EVENT_HANDLE * theevent)
{
#ifdef EMSCRIPTEN_SUPPORT
	emscripten_futex_wake(&theevent->futex, 1);
#else
	pthread_cond_signal(&theevent->eCondVar);
#endif
}

void destroyevent(EVENT_HANDLE * theevent)
{
#ifdef EMSCRIPTEN_SUPPORT
	free(theevent);
#else
	pthread_cond_destroy(&theevent->eCondVar);
	free(theevent);
#endif
}

int32_t waitevent(EVENT_HANDLE* theevent,int32_t timeout)
{
#ifdef EMSCRIPTEN_SUPPORT
	emscripten_futex_wait( &theevent->futex, 1,timeout);
	return 0;
#else
	struct timeval now;
	struct timespec timeoutstr;
	int32_t retcode;

	pthread_mutex_lock(&theevent->eMutex);
	gettimeofday(&now,0);
	timeoutstr.tv_sec = now.tv_sec + (timeout/1000);
	timeoutstr.tv_nsec = (now.tv_usec * 1000);
	retcode = 0;

	retcode = pthread_cond_timedwait(&theevent->eCondVar, &theevent->eMutex, &timeoutstr);
	if (retcode == ETIMEDOUT)
	{
		pthread_mutex_unlock(&theevent->eMutex);
		return 1;
	}
	else
	{
		pthread_mutex_unlock(&theevent->eMutex);
		return 0;
	}
#endif
}

//////////////////////////////////////////////////////////////////
// Audio callbacks
//////////////////////////////////////////////////////////////////

int push_audio_page(app_ctx * ctx, short * buf)
{
	if(
		( ((ctx->sound_fifo.idx_in+1) & 0xF ) != ( ctx->sound_fifo.idx_out & 0xF ) )
	)
	{
		memcpy((void*)&ctx->sound_fifo.snd_buf[(ctx->sound_fifo.idx_in&0xF) * ctx->sound_fifo.page_size], buf, ctx->sound_fifo.page_size * sizeof(short));
		ctx->sound_fifo.idx_in = (ctx->sound_fifo.idx_in + 1) & 0xF;
		return 1;
	}

	return 0;
}

int pop_audio_page(app_ctx * ctx, short * buf)
{
	if(
		( ((ctx->sound_fifo.idx_in) & 0xF ) != ( ctx->sound_fifo.idx_out & 0xF ) )
	)
	{
		memcpy((void*)buf,(void*)&ctx->sound_fifo.snd_buf[(ctx->sound_fifo.idx_out&0xF) * ctx->sound_fifo.page_size], ctx->sound_fifo.page_size * sizeof(short) );
		ctx->sound_fifo.idx_out = (ctx->sound_fifo.idx_out + 1) & 0xF;
		return 1;
	}

	return 0;
}

void audio_out(void *ctx, Uint8 *stream, int len)
{
	modem_ctx *mdm;

	mdm = ((app_ctx *)ctx)->mdm;

	mdm_genWave(mdm, (short*)stream, len/2);

#ifdef DBG_INJECT_SND
	//if(indbgfile)
	//	fread(stream,len,1,indbgfile);
#endif

#if 0
	int i;
	short * buf;

	buf = (short *)stream;
	for(i=0;i<len/2;i++)
	{
		band_pass_rx_Filter_put(&((app_ctx *)ctx)->rx_fir, (float)buf[i]);
		buf[i] = (short)band_pass_rx_Filter_get(&((app_ctx *)ctx)->rx_fir);
	}

#ifdef DBG_INJECT_SND
	if(outdbgfile)
		fwrite(buf,len,1,outdbgfile);
#endif

#endif

	mdm_demodulate(mdm, &mdm->demodulators[0],(short *)stream, len/2);
	//memset(stream,0,len);
}

void audio_in(void *ctx, Uint8 *stream, int len)
{
	setevent(((app_ctx *)ctx)->snd_event);

#ifdef DBG_INJECT_SND
	if(indbgfile)
		fread(stream,len,1,indbgfile);
#endif

	push_audio_page((app_ctx *)ctx, (short *)stream);

}

void * AudioInThreadProc( void *lpParameter )
{
	app_ctx * ctx;
	short * buf;
	modem_ctx *mdm;

	ctx = (app_ctx *)lpParameter;
	mdm = ctx->mdm;

	buf = malloc(ctx->sound_fifo.page_size * sizeof(short) );

	while(1)
	{
		waitevent(ctx->snd_event,1000);

		if(pop_audio_page(ctx, buf))
		{
			if(ctx->uplink)
			{
				int i;

				for(i=0;i<ctx->sound_fifo.page_size;i++)
				{
					low_pass_tx_Filter_put(&((app_ctx *)ctx)->tx_fir, (float)buf[i]);
					buf[i] = (short)low_pass_tx_Filter_get(&((app_ctx *)ctx)->tx_fir);
				}

#ifdef DBG_INJECT_SND
				//if(outdbgfile)
				//	fwrite(buf,ctx->sound_fifo.page_size*2,1,outdbgfile);
#endif

				mdm_demodulate(mdm, &mdm->demodulators[1],(short *)buf, ctx->sound_fifo.page_size);
			}
			else
			{
				mdm_demodulate(mdm, &mdm->demodulators[0],(short *)buf, ctx->sound_fifo.page_size);
			}
		}
#ifdef EMSCRIPTEN_SUPPORT
		else
		{
			emscripten_atomic_store_u32(&ctx->snd_event->futex, 0);
		}
#endif
	}

	return 0;
}

int32_t create_audioin_thread(app_ctx * ctx)
{
	uint32_t sit;
	pthread_t threadid;
	pthread_attr_t threadattrib;
	int ret;

	sit = 0;

	//pthread_attr_create(&threadattrib);
	pthread_attr_init(&threadattrib);
	pthread_attr_setinheritsched(&threadattrib,PTHREAD_EXPLICIT_SCHED);

	pthread_attr_setschedpolicy(&threadattrib,SCHED_FIFO);
	/* set the new scheduling param */
	//pthread_attr_setschedparam (&threadattrib, &param);

	ret = pthread_create(&threadid,0,AudioInThreadProc, ctx);
	if(ret)
	{
	#ifdef DEBUG
		printf("create_audioin_thread : pthread_create failed -> %d",ret);
	#endif
	}
	return sit;
}

Uint32 video_tick(Uint32 interval, void *param)
{
	uint8_t * pixels;
	bitmap_data bmp;
	videotex_ctx * vdt_ctx;
	modem_ctx *mdm;
	app_ctx *app;
	SDL_Surface *sdl_scr;
	int i;

	app = ((app_ctx *)param);
	mdm = app->mdm;
	vdt_ctx = app->vdt_ctx;
	sdl_scr = app->screen;

	update_frame(vdt_ctx,mdm, &bmp);

	if (SDL_MUSTLOCK(sdl_scr))
		SDL_LockSurface(sdl_scr);

	pixels = sdl_scr->pixels;

#ifdef EMSCRIPTEN_SUPPORT
	for(i=0;i<(bmp.xsize * bmp.ysize);i++)
	{
		pixels[(i*4)+0] = bmp.data[i] & 0xFF;
		pixels[(i*4)+1] = (bmp.data[i]>>8) & 0xFF;
		pixels[(i*4)+2] = (bmp.data[i]>>16) & 0xFF;
	}
#else
	for(i=0;i<(bmp.xsize * bmp.ysize);i++)
	{
		pixels[(i*4)+2] = bmp.data[i] & 0xFF;
		pixels[(i*4)+1] = (bmp.data[i]>>8) & 0xFF;
		pixels[(i*4)+0] = (bmp.data[i]>>16) & 0xFF;
	}
#endif

	if (SDL_MUSTLOCK(sdl_scr))
		SDL_UnlockSurface(sdl_scr);

	SDL_UpdateWindowSurface(app->window);

#ifdef EMSCRIPTEN_SUPPORT
	// Test page loop
	app->imgcnt++;

	if(app->imgcnt > vdt_ctx->framerate * 2 )
	{

		if(app->indexbuf >= vdt_test_pages_size[app->pageindex] )
		{
			if( mdm_is_fifo_empty(&mdm->tx_fifo) )
			{
				app->indexbuf = 0;
				app->imgcnt = 0;

				app->pageindex++;
				if(vdt_test_pages[app->pageindex] == NULL)
					app->pageindex = 0;

			}
		}
		else
		{
			unsigned char * ptr;

			ptr = vdt_test_pages[app->pageindex];
			while( app->indexbuf < vdt_test_pages_size[app->pageindex] )
			{
				if( !mdm_push_to_fifo( &mdm->tx_fifo, ptr[app->indexbuf] ) )
				{
					break;
				}

				app->indexbuf++;
			}
		}
	}
#endif

	return interval;
}

#ifdef EMSCRIPTEN_SUPPORT
void emscripten_vid_callback(void* param)
{
	SDL_Event event;

	while(SDL_PollEvent(&event))
	{
		if( event.type == SDL_QUIT )
		{
			exit(0);
		}
		else
		{
			if( event.type == SDL_MOUSEBUTTONDOWN )
			{
			}
		}
	}

	video_tick(16, param);
}
#endif

#endif

int isOption(int argc, char* argv[],char * paramtosearch,char * argtoparam)
{
	int param=1;
	int i,j;

	char option[512];

	if(!argv)
		return -1;

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
	fprintf(stderr,"  -sdl\t\t\t\t: SDL mode\n");
	fprintf(stderr,"  -mic\t\t\t\t: Use the Microphone/Input line instead of files\n");
	fprintf(stderr,"  -stdout \t\t\t: stdout mode\n");
	fprintf(stderr,"  -help \t\t\t: This help\n");
	fprintf(stderr,"  \nExamples :\n");
	fprintf(stderr,"  animation: vdt2bmp -ani -fps:30 -stdout /path/*.vdt | ffmpeg -y -f rawvideo -pix_fmt argb -s 320x250 -r 30 -i - -an out_video.mkv\n");
	fprintf(stderr,"  video + audio merging : ffmpeg -i out_video.mkv -i out_audio.wav -c copy output.mkv\n");
	fprintf(stderr,"  vdt to bmp convert : vdt2bmp -bmp /path/*.vdt\n");
	fprintf(stderr,"  vdt to bmp convert : vdt2bmp -bmp:out.bmp /path/videotex.vdt\n");
	fprintf(stderr,"\n");
}

int update_frame(videotex_ctx * vdt_ctx,modem_ctx *mdm, bitmap_data * bmp)
{
	unsigned char byte;

	while(mdm_pop_from_fifo(&mdm->rx_fifo[0], &byte))
	{
		vdt_push_char( vdt_ctx, byte );
	}

	bmp->xsize = vdt_ctx->bmp_res_x;
	bmp->ysize = vdt_ctx->bmp_res_y;
	bmp->data = vdt_ctx->bmp_buffer;

	vdt_render(vdt_ctx);

	return 0;
}

int gen_stdout_frame(videotex_ctx * vdt_ctx, modem_ctx *mdm,unsigned char * tmp_buf)
{
	bitmap_data bmp;
	int i,size;

	update_frame(vdt_ctx,mdm, &bmp);

	for(i=0;i<(bmp.xsize * bmp.ysize);i++)
	{
		tmp_buf[(i*4)+1] = bmp.data[i] & 0xFF;
		tmp_buf[(i*4)+2] = (bmp.data[i]>>8) & 0xFF;
		tmp_buf[(i*4)+3] = (bmp.data[i]>>16) & 0xFF;
	}
	fwrite(tmp_buf, bmp.xsize * bmp.ysize * 4, 1, stdout);

	size = mdm->sample_rate / vdt_ctx->framerate;
	if(size > 0 && size < mdm->wave_size)
	{
		mdm_genWave(mdm, NULL,  size );
		mdm_demodulate(mdm, &mdm->demodulators[0],(short *)mdm->wave_buf, size);
	}

	write_wave_file(DEFAULT_SOUND_FILE,mdm->wave_buf,size,mdm->sample_rate);

	return 0;
}

int animate(videotex_ctx * vdt_ctx, modem_ctx *mdm, char * vdtfile,float framerate, int nb_pause_frames, int out_mode, int * quit)
{
	char ofilename[512];
	int offset,i,pause_frames_cnt;
	bitmap_data bmp;
	int image_nb;
	file_cache fc;
	unsigned char * tmp_buf;
	unsigned char c;
	SDL_Event e;

	image_nb = 0;
	pause_frames_cnt = 0;

	fprintf(stderr,"Generate animation from %s...\n",vdtfile);

	memset(&fc,0,sizeof(file_cache));
	tmp_buf = NULL;

	if(open_file(&fc, vdtfile,0xFF)>=0)
	{
		if(out_mode != OUTPUT_MODE_FILE)
		{
			tmp_buf = malloc(vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
			if(!tmp_buf)
				goto alloc_error;

			memset(tmp_buf,0,vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
		}

		if(out_mode != OUTPUT_MODE_FILE)
		{
			switch(out_mode)
			{
				case OUTPUT_MODE_STDOUT:
					while(!mdm_is_fifo_empty(&mdm->tx_fifo))
					{
						gen_stdout_frame(vdt_ctx, mdm, tmp_buf);
					}

					for(i=0;i<vdt_ctx->framerate * 2;i++)
					{
						gen_stdout_frame(vdt_ctx, mdm, tmp_buf);
					}
				break;

				case OUTPUT_MODE_SDL:
					// Wait tx fifo empty
					while(!mdm_is_fifo_empty(&mdm->tx_fifo))
					{
					}

					for(i=0;i<2000;i++)
					{
						while( SDL_PollEvent(&e) )
						{
						}
						SDL_Delay(1);
					}
				break;
			}
		}

		vdt_ctx->pages_cnt++;

		offset = 0;
		while( (offset<fc.file_size || !mdm_is_fifo_empty(&mdm->tx_fifo) || (pause_frames_cnt < vdt_ctx->framerate * 1)) && !(*quit) )
		{
			while(!mdm_is_fifo_full(&mdm->tx_fifo) && offset<fc.file_size)
			{
				c = get_byte( &fc, offset, NULL );
				mdm_push_to_fifo( &mdm->tx_fifo, c);
				offset++;
			}

			if(offset>=fc.file_size)
			{
				pause_frames_cnt++;
				if(out_mode == OUTPUT_MODE_SDL)
				{
					pause_frames_cnt =  vdt_ctx->framerate * 1;
					sleep(2);
				}
			}

			switch(out_mode)
			{
				case OUTPUT_MODE_FILE:
					update_frame(vdt_ctx,mdm, &bmp);
					sprintf(ofilename,"I%.6d.bmp",image_nb);
					fprintf(stderr,"Write bmp file : %s...\n",ofilename);
					bmp24b_write(ofilename,&bmp);
				break;

				case OUTPUT_MODE_STDOUT:
					gen_stdout_frame(vdt_ctx, mdm,tmp_buf);
				break;

#ifdef SDL_SUPPORT
				case OUTPUT_MODE_SDL:
					for(i=0;i<2000;i++)
					{
						while( SDL_PollEvent(&e) )
						{
						}
						SDL_Delay(1);
					}
				break;
#endif
			}

			image_nb++;
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
	if(tmp_buf)
		free(tmp_buf);

	close_file(&fc);

	return -3;
}

int write_bmp(char * vdt_file, char * bmp_file, int pal, int out_mode)
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

	vdt_ctx = vdt_init();
	if(vdt_ctx)
	{
		if(out_mode)
		{
			tmp_buf = malloc(vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
			if(!tmp_buf)
				goto alloc_error;

			memset(tmp_buf,0,vdt_ctx->bmp_res_x * vdt_ctx->bmp_res_y * 4);
		}

		vdt_select_palette(vdt_ctx,pal);

		vdt_load_charset(vdt_ctx,  NULL);

		if(!out_mode)
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
				vdt_push_char(vdt_ctx, get_byte(&fc,offset, NULL) );
				offset++;
			}

			close_file(&fc);

			vdt_render(vdt_ctx);

			bmp.xsize = vdt_ctx->bmp_res_x;
			bmp.ysize = vdt_ctx->bmp_res_y;
			bmp.data = vdt_ctx->bmp_buffer;

			if(!out_mode)
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

		vdt_deinit(vdt_ctx);
	}
	else
	{
			ret = -2;
	}

	return ret;

alloc_error:
	if(vdt_ctx)
		vdt_deinit(vdt_ctx);

	if(tmp_buf)
		free(tmp_buf);

	close_file(&fc);

	return -3;
}

int SCRIPT_CUI_affiche(void * ctx, int MSGTYPE, char * string, ... )
{
	if( MSGTYPE!=MSG_DEBUG )
	{
		va_list marker;
		va_start( marker, string );

		vprintf(string,marker);
		printf("\n");

		va_end( marker );
	}

	return 0;
}

int main(int argc, char* argv[])
{
	char filename[512];
	char ofilename[512];
	char strtmp[512];
	modem_ctx mdm_ctx;
	int pal, outmode;
	int i,mic_mode;
	float framerate;
	videotex_ctx * vdt_ctx;
	app_ctx appctx;
	script_ctx * scriptctx;

	memset(&appctx,0,sizeof(app_ctx));

	appctx.env = setEnvVar(appctx.env, "VERSION", "v"STR_FILE_VERSION2);

#ifdef SDL_SUPPORT
	SDL_AudioSpec fmt;
	SDL_AudioSpec fmt_up;
#endif

	vdt_ctx = NULL;
	appctx.quit = 0;
	outmode = OUTPUT_MODE_FILE;
	framerate = 30.0;
	mic_mode = 0;

#ifdef DBG_INJECT_SND
	indbgfile = NULL;
	outdbgfile = NULL;

	//indbgfile = fopen("minitel_test2.wav", "rb");
	//outdbgfile = fopen("minitel_test_out.raw", "wb");
#endif

	fprintf(stderr,"Minitel VDT to BMP converter v%s\n",STR_FILE_VERSION2);
	fprintf(stderr,"Copyright (C) 2022-2023 Jean-Francois DEL NERO\n");
	fprintf(stderr,"This program comes with ABSOLUTELY NO WARRANTY\n");
	fprintf(stderr,"This is free software, and you are welcome to redistribute it\n");
	fprintf(stderr,"under certain conditions;\n\n");

	// Verbose option...
	if(isOption(argc,argv,"verbose",0)>0)
	{
		fprintf(stderr,"verbose mode\n");
		appctx.verbose=1;
	}

	// stdout option...
	if(isOption(argc,argv,"stdout",0)>0)
	{
		fprintf(stderr,"sdtout mode\n");
		outmode = OUTPUT_MODE_STDOUT;
	}

	if(isOption(argc,argv,"mic",NULL)>0)
	{
		mic_mode = 1;
		band_pass_rx_Filter_init(&appctx.rx_fir);
		appctx.fir_en = 1;
	}

	if(isOption(argc,argv,"fps",(char*)&strtmp))
	{
		framerate = atof(strtmp);
		if(framerate <= 0)
			framerate = 30.0;
	}

	if(isOption(argc,argv,"server",(char*)&filename)>0)
	{
#ifdef SDL_SUPPORT
		SDL_Init(SDL_INIT_EVERYTHING);
		appctx.keystate = SDL_GetKeyboardState(NULL);

		outmode = OUTPUT_MODE_SDL;

		vdt_ctx = vdt_init();
		appctx.vdt_ctx = vdt_ctx;
		vdt_ctx->framerate = framerate;

		vdt_load_charset(vdt_ctx, NULL);

		mdm_init(&mdm_ctx);
		appctx.mdm = &mdm_ctx;

		appctx.window = SDL_CreateWindow( "Minitel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vdt_ctx->bmp_res_x, vdt_ctx->bmp_res_y, SDL_WINDOW_SHOWN );
		if(!appctx.window)
		{
			fprintf(stderr, "ERROR : SDL_CreateWindow - %s\n",SDL_GetError());
			return -1;
		}

		appctx.screen = SDL_GetWindowSurface( appctx.window );
		if(!appctx.screen)
		{
			fprintf(stderr, "ERROR : SDL_GetWindowSurface - %s\n",SDL_GetError());
			return -1;
		}
		
		if(!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
		{
			if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
			{
				fprintf(stderr, "ERROR : SDL_INIT_AUDIO not initialized !\n");
				return -1;
			}
		}

		memset(&fmt,0,sizeof(fmt));
		fmt.freq = mdm_ctx.sample_rate;
		fmt.format = AUDIO_S16;
		fmt.channels = 1;
		fmt.samples = mdm_ctx.wave_size;
		if(mic_mode)
			fmt.callback = audio_in;
		else
			fmt.callback = audio_out;

		fmt.userdata = &appctx;

		if(isOption(argc,argv,"uplink",NULL)>0)
		{
			memset(&fmt_up,0,sizeof(fmt));
			fmt_up.freq = mdm_ctx.sample_rate;
			fmt_up.format = AUDIO_S16;
			fmt_up.channels = 1;
			fmt_up.samples = mdm_ctx.wave_size;
			fmt_up.callback = audio_in;
			fmt_up.userdata = &appctx;

			appctx.snd_event = createevent();

			appctx.sound_fifo.page_size = DEFAULT_SOUND_BUFFER_SIZE;
			appctx.sound_fifo.snd_buf = malloc( appctx.sound_fifo.page_size * 16 * sizeof(short) );
			memset((void*)appctx.sound_fifo.snd_buf,0,appctx.sound_fifo.page_size * 16 * sizeof(short));

			appctx.audio_id_uplink = SDL_OpenAudioDevice(NULL, 1, &fmt_up, &fmt_up, 0);
			if ( appctx.audio_id_uplink < 0 )
			{
				fprintf(stderr, "SDL Sound Init error: %s\n", SDL_GetError());
			}
			else
			{
				SDL_PauseAudioDevice( appctx.audio_id_uplink, SDL_FALSE );
			}

			low_pass_tx_Filter_init(&appctx.tx_fir);
			appctx.uplink = 1;

			create_audioin_thread(&appctx);
		}

		appctx.audio_id = SDL_OpenAudioDevice(NULL, mic_mode, &fmt, &fmt, 0);
		if ( appctx.audio_id < 0 )
		{
			fprintf(stderr, "SDL Sound Init error: %s\n", SDL_GetError());
		}
		else
		{
			SDL_PauseAudioDevice( appctx.audio_id, SDL_FALSE );
		}

		appctx.timer = SDL_AddTimer(30, video_tick, &appctx);

		scriptctx = init_script((void *)&appctx, 0x00000000, (void*)&appctx.env);
		if( scriptctx )
		{
			setOutputFunc_script( scriptctx, SCRIPT_CUI_affiche );
			execute_file_script( scriptctx, (char*)&filename );

			deinit_script( scriptctx );
		}
#else
		fprintf(stderr, "ERROR : No built-in SDL support !\n");
#endif
	}

	if(isOption(argc,argv,"sdl",0)>0)
	{
#ifdef SDL_SUPPORT
		outmode = OUTPUT_MODE_SDL;
		vdt_ctx = vdt_init();

		if(vdt_ctx)
		{
			mdm_init(&mdm_ctx);

			SDL_Init(SDL_INIT_EVERYTHING);

			appctx.keystate = SDL_GetKeyboardState(NULL);

			appctx.window = SDL_CreateWindow( "Minitel", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, vdt_ctx->bmp_res_x, vdt_ctx->bmp_res_y, SDL_WINDOW_SHOWN );
			if(!appctx.window)
			{
				fprintf(stderr, "ERROR : SDL_CreateWindow - %s\n",SDL_GetError());
				return -1;
			}

			appctx.screen = SDL_GetWindowSurface( appctx.window );
			if(!appctx.screen)
			{
				fprintf(stderr, "ERROR : SDL_GetWindowSurface - %s\n",SDL_GetError());
				return -1;
			}

			appctx.mdm = &mdm_ctx;
			appctx.vdt_ctx = vdt_ctx;

			if(!(SDL_WasInit(SDL_INIT_AUDIO) & SDL_INIT_AUDIO))
			{
				if (SDL_InitSubSystem(SDL_INIT_AUDIO) < 0)
				{
					fprintf(stderr, "ERROR : SDL_INIT_AUDIO not initialized !\n");
					return -1;
				}
			}

			memset(&fmt,0,sizeof(fmt));
			fmt.freq = mdm_ctx.sample_rate;
			fmt.format = AUDIO_S16;
			fmt.channels = 1;
			fmt.samples = mdm_ctx.wave_size;
			if(mic_mode)
				fmt.callback = audio_in;
			else
				fmt.callback = audio_out;

			fmt.userdata = &appctx;


			if(isOption(argc,argv,"uplink",NULL)>0)
			{
				memset(&fmt_up,0,sizeof(fmt));
				fmt_up.freq = mdm_ctx.sample_rate;
				fmt_up.format = AUDIO_S16;
				fmt_up.channels = 1;
				fmt_up.samples = mdm_ctx.wave_size;
				fmt_up.callback = audio_in;
				fmt_up.userdata = &appctx;

				appctx.audio_id_uplink = SDL_OpenAudioDevice(NULL, 1, &fmt_up, &fmt_up, 0);
				if ( appctx.audio_id_uplink < 0 )
				{
					fprintf(stderr, "SDL Sound Init error: %s\n", SDL_GetError());
				}
				else
				{
					SDL_PauseAudioDevice( appctx.audio_id_uplink, SDL_FALSE );
				}

				low_pass_tx_Filter_init(&appctx.tx_fir);

			}

			appctx.audio_id = SDL_OpenAudioDevice(NULL, mic_mode, &fmt, &fmt, 0);
			if ( appctx.audio_id < 0 )
			{
				fprintf(stderr, "SDL Sound Init error: %s\n", SDL_GetError());
			}
			else
			{
				SDL_PauseAudioDevice( appctx.audio_id, SDL_FALSE );
			}

			#ifndef EMSCRIPTEN_SUPPORT
			appctx.timer = SDL_AddTimer(30, video_tick, &appctx);
			#endif
		}
#else
		fprintf(stderr, "ERROR : No built-in SDL support !\n");
#endif
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
				write_bmp(argv[i], ofilename, pal,outmode);
			}

			i++;
		}
	}

	// Animation generator
	if(isOption(argc,argv,"uplink",NULL)>0)
	{
		appctx.uplink = 1;
	}

	// Animation generator
	if(isOption(argc,argv,"ani",NULL)>0)
	{
		mdm_init(&mdm_ctx);

		if(!vdt_ctx)
			vdt_ctx = vdt_init();

		if(vdt_ctx)
		{
			vdt_ctx->framerate = framerate;

			if(isOption(argc,argv,"greyscale",NULL)>0)
			{
				vdt_select_palette(vdt_ctx,0);
			}

			vdt_load_charset(vdt_ctx, NULL);

			if( !mic_mode )
			{

				#ifndef EMSCRIPTEN_SUPPORT
				remove(DEFAULT_SOUND_FILE);
				#endif

				#ifdef SDL_SUPPORT
				if(appctx.uplink)
				{
					appctx.snd_event = createevent();

					appctx.sound_fifo.page_size = DEFAULT_SOUND_BUFFER_SIZE;
					appctx.sound_fifo.snd_buf = malloc( appctx.sound_fifo.page_size * 16 * sizeof(short) );
					memset((void*)appctx.sound_fifo.snd_buf,0,appctx.sound_fifo.page_size * 16 * sizeof(short));

					create_audioin_thread(&appctx);
				}
				#endif

				#ifdef EMSCRIPTEN_SUPPORT

				emscripten_set_main_loop_arg(emscripten_vid_callback, &appctx, 30, 1);

				while(1)
				{
					emscripten_sleep(10);

					i = 0;
					while( i < sizeof(vdt_test_001) )
					{
						if( !mdm_push_to_fifo( &mdm_ctx.tx_fifo, vdt_test_001[i] ) )
						{
							emscripten_sleep(10);
						}
						i++;
					}
				}

				#endif

				i = 1;
				while( i < argc && !appctx.quit)
				{
					if(argv[i][0] != '-')
					{
						animate(vdt_ctx,&mdm_ctx,argv[i],framerate,(int)(framerate*4),outmode,&appctx.quit);
					}
					i++;
				}
			}
			else
			{

#ifdef SDL_SUPPORT
				appctx.snd_event = createevent();

				appctx.sound_fifo.page_size = DEFAULT_SOUND_BUFFER_SIZE;
				appctx.sound_fifo.snd_buf = malloc( appctx.sound_fifo.page_size * 16 * sizeof(short) );
				memset((void*)appctx.sound_fifo.snd_buf,0,appctx.sound_fifo.page_size * 16 * sizeof(short));

				create_audioin_thread(&appctx);
#endif

				#ifdef EMSCRIPTEN_SUPPORT
				emscripten_set_main_loop_arg(emscripten_vid_callback, &appctx, 30, 1);
				#endif

				while( !appctx.quit)
				{
					SDL_Delay(1000*100);
				}
			}

			fprintf(stderr,"Videotex pages count : %d \n",vdt_ctx->pages_cnt);
			fprintf(stderr,"Videotex input bytes count : %d \n",vdt_ctx->input_bytes_cnt);
			fprintf(stderr,"Rendered images : %d \n",vdt_ctx->rendered_images_cnt);
			fprintf(stderr,"Frame rate : %.2f fps \n",framerate);
			fprintf(stderr,"Duration : %f ms \n",(float)((float)(vdt_ctx->rendered_images_cnt*1000)/(float)framerate));
			fprintf(stderr,"Sound samples count : %d \n",mdm_ctx.samples_generated_cnt);
			fprintf(stderr,"Sound duration : %f ms \n",((float)mdm_ctx.samples_generated_cnt/(float)mdm_ctx.sample_rate)*1000);
			fprintf(stderr,"Video duration - Sound duration diff : %f ms \n",((float)((float)(vdt_ctx->rendered_images_cnt*1000)/(float)framerate)) - (((float)mdm_ctx.samples_generated_cnt/(float)mdm_ctx.sample_rate)*1000));

			vdt_deinit(vdt_ctx);
		}
	}

#ifdef SDL_SUPPORT
	if(appctx.window)
	{
		SDL_DestroyWindow( appctx.window );

		SDL_RemoveTimer( appctx.timer );

		SDL_PauseAudioDevice( appctx.audio_id, SDL_TRUE );

		SDL_CloseAudioDevice( appctx.audio_id );

		if(appctx.uplink)
		{
			SDL_PauseAudioDevice( appctx.audio_id_uplink, SDL_TRUE );

			SDL_CloseAudioDevice( appctx.audio_id_uplink );
		}
	}
	SDL_Quit();
#endif

	if( (isOption(argc,argv,"help",0)<=0) &&
		(isOption(argc,argv,"vdt",0)<=0)  &&
		(isOption(argc,argv,"ani",0)<=0)  &&
		(isOption(argc,argv,"mic",0)<=0)  &&
		(isOption(argc,argv,"bmp",0)<=0) )
	{
		printhelp(argv);
	}

	return 0;
}
