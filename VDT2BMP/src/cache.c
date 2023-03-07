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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "cache.h"

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
