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
// File : env.h
// Contains: Internal variables support.
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#ifdef SCRIPT_64BITS_SUPPORT
#define env_var_value uint64_t
#define signed_env_var_value int64_t
#define STRTOVALUE strtoull
#else
#define env_var_value uint32_t
#define signed_env_var_value int32_t
#define STRTOVALUE strtoul
#endif

//#define STATIC_ENV_BUFFER 1
#define ENV_PAGE_SIZE (16*1024)
#define ENV_MAX_TOTAL_BUFFER_SIZE (1024 * 1024) // 1MB
#define ENV_MAX_STRING_SIZE 512

typedef struct envvar_entry_
{
#ifdef STATIC_ENV_BUFFER
	unsigned char buf[ENV_PAGE_SIZE];
#else
	unsigned char * buf;
#endif
	unsigned int  bufsize;
}envvar_entry;

envvar_entry * setEnvVar( envvar_entry * env, char * varname, char * vardata);
char * getEnvVar( envvar_entry * env, char * varname, char * vardata);
env_var_value getEnvVarValue( envvar_entry * env, char * varname);
envvar_entry * setEnvVarValue( envvar_entry * env, char * varname, env_var_value value);
char * getEnvVarIndex( envvar_entry * env, int index, char * vardata);
envvar_entry * duplicate_env_vars(envvar_entry * src);
void free_env_vars(envvar_entry * src);
