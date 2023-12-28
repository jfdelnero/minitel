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

#define SERIAL_FIFO_SIZE 4096

typedef struct serial_fifo_
{
	int in_ptr;
	int out_ptr;

	unsigned char fifo[SERIAL_FIFO_SIZE];
}serial_fifo;

int is_fifo_empty(serial_fifo *fifo);
int is_fifo_full(serial_fifo *fifo);
int push_to_fifo(serial_fifo *fifo, unsigned char c);
int pop_from_fifo(serial_fifo *fifo, unsigned char * c);
int purge_fifo(serial_fifo *fifo);