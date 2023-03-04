///////////////////////////////////////////////////////////////////////////////////
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-----------H----H--X----X-----CCCCC----22222----0000-----0000------11----------//
//----------H----H----X-X-----C--------------2---0----0---0----0--1--1-----------//
//---------HHHHHH-----X------C----------22222---0----0---0----0-----1------------//
//--------H----H----X--X----C----------2-------0----0---0----0-----1-------------//
//-------H----H---X-----X---CCCCC-----222222----0000-----0000----1111------------//
//-------------------------------------------------------------------------------//
//----------------------------------------------------- http://hxc2001.free.fr --//
///////////////////////////////////////////////////////////////////////////////////
// File : bmp_file.c
// Contains: bmp image file loader / writer
//
// This file is part of VDT2BMP.
//
// Written by: Jean-François DEL NERO
//
// Copyright (C) 2006-2023 Jean-François DEL NERO
//
// You are free to do what you want with this code.
// A credit is always appreciated if you use it into your product :)
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdint.h>

#include "bmp_file.h"

int bmp_load(char * file,bitmap_data * bdata)
{
	FILE * f;
	int i,j;
	int xsize,ysize;
	int bitperpixel;
	int palettesize;
	unsigned int    bitmapdataoffset,bitmapdatalinesize;
	unsigned char * linebuffer;
	BITMAPFILEHEADER bitmap_header;
	BITMAPINFOHEADER bitmap_info_header;
	RGBQUAD * palette;

	palette = NULL;
	linebuffer = NULL;

	f = fopen(file,"rb");
	if(f)
	{
		if( fread(&bitmap_header,sizeof(BITMAPFILEHEADER),1,f) != 1 )
			goto error;

		if( bitmap_header.bfType == 19778 )
		{
			if( fread(&bitmap_info_header,sizeof(BITMAPINFOHEADER),1,f) != 1 )
				goto error;

			xsize=bitmap_info_header.biWidth;
			ysize=bitmap_info_header.biHeight;
			bitperpixel=bitmap_info_header.biBitCount;

			if(!bitmap_info_header.biCompression || (bitmap_info_header.biCompression == BI_BITFIELDS) )
			{
				// TODO : Read and use the BI_BITFIELDS RGB masks.

				// read palette
				switch(bitperpixel)
				{
					case 1:
						palettesize = 2;
					break;
					case 4:
						palettesize = 16;
					break;
					case 8:
						palettesize = 256;
					break;
					case 24:
						palettesize = 0;
					break;
					case 32:
						palettesize = 0;
					break;
					default:
						//printf("bmp_load : bitperpixel not supported : %d\n",bitperpixel);
						palettesize = 0;
						//non supported
					break;
				}

				palette=0;
				if(palettesize)
				{
					palette=(RGBQUAD *) malloc(palettesize*sizeof(RGBQUAD));
					if(!palette)
						goto error;

					memset(palette,0,palettesize*sizeof(RGBQUAD));

					fseek(f,sizeof(BITMAPFILEHEADER)+ bitmap_info_header.biSize ,SEEK_SET);
					if( fread(palette,palettesize*sizeof(RGBQUAD),1,f) != 1 )
						goto error;
				}

				bitmapdataoffset = sizeof(BITMAPFILEHEADER)+ bitmap_info_header.biSize + (palettesize*sizeof(RGBQUAD));
				fseek(f,bitmapdataoffset ,SEEK_SET);

				bdata->xsize = xsize;
				bdata->ysize = ysize;
				bdata->data = (uint32_t *)malloc(xsize*ysize*sizeof(uint32_t));
				if(!bdata->data)
					goto error;

				memset(bdata->data,0,xsize*ysize*sizeof(uint32_t));

				switch(bitperpixel)
				{
					case 1:
						if((xsize/8 + (xsize&7))&0x3)
							bitmapdatalinesize = (((xsize/8 + (xsize&7)) & (~0x3)) + 0x4);
						else
							bitmapdatalinesize = (xsize/8 + (xsize&7));

						linebuffer = (unsigned char *)malloc(bitmapdatalinesize);
						if(!linebuffer)
							goto error;

						for(i = 0; i < ysize; i++ )
						{
							fseek(f,bitmapdataoffset + (bitmapdatalinesize*i) ,SEEK_SET);
							if( fread(linebuffer,bitmapdatalinesize,1,f) != 1 )
								goto error;

							for(j=0;j<xsize;j++)
							{
								bdata->data[(xsize*((ysize-1)-i))+j]=palette[(linebuffer[(j/8)]>>(1*((7-(j))&7)))&0x1].rgbRed |
																	 palette[(linebuffer[(j/8)]>>(1*((7-(j))&7)))&0x1].rgbGreen<<8 |
																	 palette[(linebuffer[(j/8)]>>(1*((7-(j))&7)))&0x1].rgbBlue<<16;
							}
						}

						free(linebuffer);
						linebuffer = NULL;
						free(palette);
						palette = NULL;
					break;

					case 4:
						if((xsize/2 + (xsize&1))&0x3)
							bitmapdatalinesize = (((xsize/2 + (xsize&1)) & (~0x3)) + 0x4);
						else
							bitmapdatalinesize = (xsize/2 + (xsize&1));

						linebuffer=(unsigned char *) malloc(bitmapdatalinesize);
						if(!linebuffer)
							goto error;

						for(i=0;i<ysize;i++)
						{
							fseek(f,bitmapdataoffset + (bitmapdatalinesize*i) ,SEEK_SET);
							if( fread(linebuffer,bitmapdatalinesize,1,f) != 1 )
								goto error;

							for(j=0;j<xsize;j++)
							{
								bdata->data[(xsize*((ysize-1)-i))+j]=palette[(linebuffer[(j/2)]>>(4*((~j)&1)))&0xF].rgbRed |
																	 palette[(linebuffer[(j/2)]>>(4*((~j)&1)))&0xF].rgbGreen<<8 |
																	 palette[(linebuffer[(j/2)]>>(4*((~j)&1)))&0xF].rgbBlue<<16;
							}
						}

						free(linebuffer);
						linebuffer = NULL;
						free(palette);
						palette = NULL;
					break;

					case 8:
						if(xsize&0x3)
							bitmapdatalinesize = ((xsize & (~0x3)) + 0x4);
						else
							bitmapdatalinesize = xsize;

						linebuffer = (unsigned char *)malloc(bitmapdatalinesize);
						if(!linebuffer)
							goto error;

						for(i=0;i<ysize;i++)
						{
							fseek(f,bitmapdataoffset + (bitmapdatalinesize*i) ,SEEK_SET);
							if( fread(linebuffer,bitmapdatalinesize,1,f) != 1 )
								goto error;

							for(j=0;j<xsize;j++)
							{
								bdata->data[(xsize*((ysize-1)-i))+j]=palette[linebuffer[j]].rgbRed |
														 palette[linebuffer[j]].rgbGreen<<8 |
														 palette[linebuffer[j]].rgbBlue<<16;
							}
						}

						free(linebuffer);
						linebuffer = NULL;
						free(palette);
						palette = NULL;
					break;

					case 24:
						if((xsize*3)&0x3)
							bitmapdatalinesize=(((xsize*3) & (~0x3)) + 0x4);
						else
							bitmapdatalinesize=(xsize*3);

						linebuffer=(unsigned char *)malloc(bitmapdatalinesize);
						if(!linebuffer)
							goto error;

						for(i=0;i<ysize;i++)
						{
							fseek(f,bitmapdataoffset + (bitmapdatalinesize*i) ,SEEK_SET);
							if( fread(linebuffer,bitmapdatalinesize,1,f) != 1 )
								goto error;

							for(j=0;j<xsize;j++)
							{
								bdata->data[(xsize*((ysize-1)-i))+j]=linebuffer[j*3+2] |
																	 linebuffer[j*3+1]<<8 |
																	 linebuffer[j*3+0]<<16;
							}
						}

						free(linebuffer);
						linebuffer = NULL;
					break;

					case 32:
						if((xsize*4)&0x3)
							bitmapdatalinesize=(((xsize*4) & (~0x3)) + 0x4);
						else
							bitmapdatalinesize=(xsize*4);

						linebuffer=(unsigned char *)malloc(bitmapdatalinesize);
						if(!linebuffer)
							goto error;

						for(i=0;i<ysize;i++)
						{
							fseek(f,bitmapdataoffset + (bitmapdatalinesize*i) ,SEEK_SET);
							if( fread(linebuffer,bitmapdatalinesize,1,f) != 1 )
								goto error;

							for(j=0;j<xsize;j++)
							{
								bdata->data[(xsize*((ysize-1)-i))+j]=linebuffer[j*4+2] |
																	 linebuffer[j*4+1]<<8 |
																	 linebuffer[j*4+0]<<16;
							}
						}

						free(linebuffer);
						linebuffer = NULL;
					break;

					default:
						//non supported
					break;
				}
			}
			else
			{
				// non supported
				//printf("bmp_load : Packed bmp not supported : %d\n",bitmap_info_header.biCompression);
				fclose(f);
				return -1;
			}
		}
		else
		{
			// non bitmap file
			fclose(f);
			return -2;
		}

		fclose(f);
		return 0;  // ok
	}

	//file error
	return -3;

error:
	if(f)
		fclose(f);

	if(palette)
		free(palette);

	if(linebuffer)
		free(linebuffer);

	return -3;
}

int bmp24b_write(char * file,bitmap_data * bdata)
{
	FILE * f;
	int i,j;
	BITMAPFILEHEADER bitmap_header;
	BITMAPINFOHEADER bitmap_info_header;
	uint32_t bitmapdatalinesize;
	unsigned char * linebuffer;

	f = fopen(file,"wb");
	if(f)
	{
		memset(&bitmap_header,0,sizeof(BITMAPFILEHEADER));

		bitmap_header.bfType = 19778;
		bitmap_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		bitmapdatalinesize = bdata->xsize*3;

		if(bitmapdatalinesize&0x3)
			bitmapdatalinesize = ((bitmapdatalinesize & (~0x3)) + 0x4);

		bitmap_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bitmapdatalinesize*bdata->ysize);
		fwrite(&bitmap_header,1,sizeof(BITMAPFILEHEADER),f);

		memset(&bitmap_info_header,0,sizeof(BITMAPINFOHEADER));
		bitmap_info_header.biSize = sizeof(BITMAPINFOHEADER);
		bitmap_info_header.biBitCount = 24;
		bitmap_info_header.biHeight = bdata->ysize;
		bitmap_info_header.biWidth = bdata->xsize;
		bitmap_info_header.biPlanes = 1;
		bitmap_info_header.biSizeImage = bitmapdatalinesize*bdata->ysize;

		fwrite(&bitmap_info_header,1,sizeof(BITMAPINFOHEADER),f);

		linebuffer = (unsigned char *)malloc(bitmapdatalinesize);
		memset(linebuffer,0,bitmapdatalinesize);

		for(i=0;i<bdata->ysize;i++)
		{
			for(j=0;j<bdata->xsize;j++)
			{
				linebuffer[(j*3)+2] = (unsigned char)( (bdata->data[((((bdata->ysize-1)-i))*bdata->xsize) + j]) & 0xFF);
				linebuffer[(j*3)+1] = (unsigned char)( (bdata->data[((((bdata->ysize-1)-i))*bdata->xsize) + j]>>8) & 0xFF);
				linebuffer[(j*3)+0] = (unsigned char)( (bdata->data[((((bdata->ysize-1)-i))*bdata->xsize) + j]>>16) & 0xFF);
			}

			fwrite(linebuffer,1,bitmapdatalinesize,f);
		}

		free(linebuffer);
		fclose(f);
	}

	return 0;
}

int bmp16b_write(char * file,bitmap_data * bdata)
{
	FILE * f;
	int i,j;
	unsigned short color;
	BITMAPFILEHEADER bitmap_header;
	BITMAPINFOHEADER bitmap_info_header;
	uint32_t bitmapdatalinesize;
	unsigned char * linebuffer;

	f = fopen(file,"wb");
	if(f)
	{
		memset(&bitmap_header,0,sizeof(BITMAPFILEHEADER));

		bitmap_header.bfType = 19778;
		bitmap_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

		bitmapdatalinesize = bdata->xsize*2;

		if(bitmapdatalinesize&0x3)
			bitmapdatalinesize = ((bitmapdatalinesize & (~0x3)) + 0x4);

		bitmap_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (bitmapdatalinesize*bdata->ysize);
		fwrite(&bitmap_header,1,sizeof(BITMAPFILEHEADER),f);

		memset(&bitmap_info_header,0,sizeof(BITMAPINFOHEADER));
		bitmap_info_header.biSize = sizeof(BITMAPINFOHEADER);
		bitmap_info_header.biBitCount = 16;
		bitmap_info_header.biHeight = bdata->ysize;
		bitmap_info_header.biWidth = bdata->xsize;
		bitmap_info_header.biPlanes = 1;
		bitmap_info_header.biSizeImage = bitmapdatalinesize*bdata->ysize;

		fwrite(&bitmap_info_header,1,sizeof(BITMAPINFOHEADER),f);

		linebuffer = (unsigned char *)malloc(bitmapdatalinesize);
		memset(linebuffer,0,bitmapdatalinesize);

		for(i=0;i<bdata->ysize;i++)
		{
			for(j=0;j<bdata->xsize;j++)
			{

				color = (unsigned short)( (bdata->data[((((bdata->ysize-1)-i))*bdata->xsize) + j]) & 0xFF)>>3;
				color = (color<<5) | (unsigned short)( (bdata->data[((((bdata->ysize-1)-i))*bdata->xsize) + j]>>8) & 0xFF)>>3;
				color = (color<<5) | (unsigned short)( (bdata->data[((((bdata->ysize-1)-i))*bdata->xsize) + j]>>16) & 0xFF)>>3;

				linebuffer[(j*2)+0] =  color & 0xFF;
				linebuffer[(j*2)+1] =  (color>>8) & 0xFF;
			}

			fwrite(linebuffer,1,bitmapdatalinesize,f);
		}

		free(linebuffer);
		fclose(f);
	}

	return 0;
}


int packlineRLE(unsigned char * src,int size,unsigned char * dst)
{
	int cnt,i,j,lastvalue;


	i = 0;
	j = 0;
	lastvalue = src[0];
	cnt = 0;
	while(i < size)
	{
		if( (src[i] == lastvalue) && (cnt<254))
		{
			cnt++;
		}
		else
		{
			dst[j++] = cnt;
			dst[j++] = lastvalue;

			lastvalue = src[i];
			cnt = 1;
		}

		i++;
	}

	if(cnt)
	{
		dst[j++] = cnt;
		dst[j++] = lastvalue;
	}

	dst[j++] = 0;
	dst[j++] = 0;

	return j;
}

int bmpRLE8b_write(char * file,bitmap_data * bdata)
{
	FILE * f;
	int i,lnsize,datasize;
	BITMAPFILEHEADER bitmap_header;
	BITMAPINFOHEADER bitmap_info_header;
	uint32_t bitmapdatalinesize;
	unsigned char * linebuffer;
	unsigned char * src_data;
	unsigned char pal[256*4];


	f = fopen(file,"wb");
	if(f)
	{
		memset(&bitmap_header,0,sizeof(BITMAPFILEHEADER));

		bitmap_header.bfType = 19778;
		bitmap_header.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + (256*4);

		bitmapdatalinesize = bdata->xsize*1;

		if(bitmapdatalinesize&0x3)
			bitmapdatalinesize = ((bitmapdatalinesize & (~0x3)) + 0x4);

		bitmap_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(pal) + (bitmapdatalinesize*bdata->ysize);
		fwrite(&bitmap_header,1,sizeof(BITMAPFILEHEADER),f);

		memset(&bitmap_info_header,0,sizeof(BITMAPINFOHEADER));
		bitmap_info_header.biSize = sizeof(BITMAPINFOHEADER);
		bitmap_info_header.biBitCount = 8;
		bitmap_info_header.biHeight = bdata->ysize;
		bitmap_info_header.biWidth = bdata->xsize;
		bitmap_info_header.biPlanes = 1;
		bitmap_info_header.biCompression = 1;
		bitmap_info_header.biSizeImage = bitmapdatalinesize*bdata->ysize;

		fwrite(&bitmap_info_header,1,sizeof(BITMAPINFOHEADER),f);

		memset(pal,0,sizeof(pal));
		if(bdata->palette)
		{
			for(i=0;i<256;i++)
			{
				pal[(i*4) + 0] = (bdata->palette[(i*4) + 2]);
				pal[(i*4) + 1] = (bdata->palette[(i*4) + 1]);
				pal[(i*4) + 2] = (bdata->palette[(i*4) + 0]);
			}
		}
		else
		{
			for(i=0;i<256;i++)
			{
				pal[(i*4) + 0] = i;
				pal[(i*4) + 1] = i;
				pal[(i*4) + 2] = i;
			}
		}

		fwrite(pal,sizeof(pal),1,f);

		linebuffer = (unsigned char *)malloc((bitmapdatalinesize*2) + (bdata->ysize*2));
		memset(linebuffer,0,(bitmapdatalinesize*2) + (bdata->ysize*2));

		datasize = 0;
		src_data = (unsigned char*)bdata->data;
		for(i=0;i<bdata->ysize;i++)
		{
			lnsize = packlineRLE(&src_data[((bdata->ysize-1)-i)*bdata->xsize],bdata->xsize,linebuffer);

			if( i == (bdata->ysize - 1) )
			{
				if(lnsize)
				{
					linebuffer[lnsize-1] = 0x01; // End of BMP
				}
			}

			fwrite(linebuffer,1,lnsize,f);

			datasize = datasize + lnsize;
		}

		bitmap_header.bfSize = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER) + sizeof(pal) + datasize;
		fseek(f,0,SEEK_SET);
		fwrite(&bitmap_header,1,sizeof(BITMAPFILEHEADER),f);

		bitmap_info_header.biSizeImage = datasize;
		fseek(f,sizeof(BITMAPFILEHEADER),SEEK_SET);
		fwrite(&bitmap_info_header,1,sizeof(BITMAPINFOHEADER),f);

		free(linebuffer);
		fclose(f);
	}

	return 0;
}
