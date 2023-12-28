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
 FIFO

 (C) 2022-2O23 Jean-François DEL NERO
*/

#include "fifo.h"

int is_fifo_empty(serial_fifo *fifo)
{
	if( ( fifo->in_ptr & (SERIAL_FIFO_SIZE-1) ) == ( fifo->out_ptr & (SERIAL_FIFO_SIZE-1) ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int is_fifo_full(serial_fifo *fifo)
{
	if( ( (fifo->in_ptr + 1) & (SERIAL_FIFO_SIZE-1) ) == ( fifo->out_ptr & (SERIAL_FIFO_SIZE-1) ) )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

int push_to_fifo(serial_fifo *fifo, unsigned char c)
{
	if ( is_fifo_full(fifo) )
		return 0;

	fifo->fifo[(fifo->in_ptr & (SERIAL_FIFO_SIZE-1))] = c;

	fifo->in_ptr = (fifo->in_ptr + 1) & (SERIAL_FIFO_SIZE-1);

	return 1;
}

int pop_from_fifo(serial_fifo *fifo, unsigned char * c)
{
	int ret;

	ret = 0;

	if( !is_fifo_empty(fifo) )
	{
		*c = fifo->fifo[fifo->out_ptr & (SERIAL_FIFO_SIZE-1)];
		fifo->out_ptr = (fifo->out_ptr + 1) & (SERIAL_FIFO_SIZE-1);
		ret = 1;
	}

	return ret;
}

int purge_fifo(serial_fifo *fifo)
{
	fifo->in_ptr = 0;
	fifo->out_ptr = 0;

	return 0;
}
