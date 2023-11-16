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
// File : env.c
// Contains: Internal variables support.
//
// Written by: Jean-François DEL NERO
//
// Change History (most recent first):
///////////////////////////////////////////////////////////////////////////////////

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

#include "env.h"

/*
|L|H|str varname\0|L|H|str vardata\0|L|H|str varname\0|L|H|str vardata\0|0|0|

(H*256)+L = str size (\0 included)
if L & H == 0 -> end of buffer
*/

#ifdef STATIC_ENV_BUFFER
static envvar_entry static_envvar;
#endif

static uint16_t getEnvStrSize(unsigned char * buf)
{
	uint16_t size;

	size = *buf++;
	size += (((uint16_t)*buf)<<8);

	return size;
}

static void setEnvStrSize(unsigned char * buf, uint16_t size)
{
	*buf++ = size & 0xFF;
	*buf = (size >> 8) & 0xFF;

	return;
}

static int getEnvEmptyOffset(envvar_entry * env)
{
	int off;
	unsigned short varname_size, vardata_size;

	off = 0;

	varname_size = getEnvStrSize(&env->buf[off]); // var name string size
	vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size

	while( varname_size )
	{
		off += (2 + varname_size + 2 + vardata_size);

		varname_size = getEnvStrSize(&env->buf[off]); // var name string size
		vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size
	}

	return off;
}

static int getEnvBufOff(envvar_entry * env, char * varname)
{
	unsigned short curstr_size;
	int str_index,i;

	i = 0;

	str_index = 0;
	curstr_size = getEnvStrSize(&env->buf[i]);
	i += 2;

	while( curstr_size )
	{
		if( !(str_index & 1) ) // variable name ?
		{
			if( !strcmp((char*)&env->buf[i],varname ) )
			{
				// this is the variable we are looking for.
				return (i - 2);
			}
			else
			{
				// not the right variable - skip this string.
				i += curstr_size;
			}
		}
		else
		{
			//variable data - skip this string.
			i += curstr_size;
		}

		if( i < env->bufsize - 2 )
		{
			curstr_size = getEnvStrSize(&env->buf[i]);
			i += 2;
		}
		else
		{
			curstr_size = 0;
		}

		str_index++;
	}

	return -1; // Not found.
}

static int pushStr(envvar_entry * env, int offset, char * str)
{
	int size;

	if(!str || offset < 0)
		return -1;

	size = strlen(str) + 1;

	if( ( offset + 2 + size ) <  env->bufsize )
	{
		setEnvStrSize(&env->buf[offset], size);
		offset += 2;
		strncpy((char*)&env->buf[offset],str,size);
		offset += size;
		env->buf[offset - 1] = '\0';

		return offset;
	}

	return -1;
}

static int pushEnvEntry(envvar_entry * env, char * varname, char * varvalue)
{
	int total_size;
	int offset;

	total_size = 2 + (strlen(varname) + 1) + 2 + (strlen(varvalue) + 1);

	offset = getEnvEmptyOffset(env);
	if( offset < 0 )
		return -1;

	if( (total_size + offset) <  env->bufsize )
	{

		offset = pushStr(env, offset, varname);
		offset = pushStr(env, offset, varvalue);

		return offset;
	}

	return -1;
}

envvar_entry * setEnvVar( envvar_entry * env, char * varname, char * varvalue )
{
	int i,off,ret;
	unsigned short varname_size, vardata_size;
	int oldentrysize;

	i = 0;

	if(!env)
	{
#ifdef STATIC_ENV_BUFFER
		memset(&static_envvar,0,ENV_PAGE_SIZE);
		static_envvar.bufsize = ENV_PAGE_SIZE;

		env = &static_envvar;
#else
		env = malloc( sizeof(envvar_entry) );
		if(!env)
			return NULL;

		memset( env,0,sizeof(envvar_entry));

		env->bufsize = ENV_PAGE_SIZE;
		env->buf = malloc(env->bufsize);
		if(!env->buf)
		{
			free(env);
			return NULL;
		}

		memset(env->buf,0,env->bufsize);
#endif
	}

	off = getEnvBufOff( env, varname );
	if( off >= 0 )
	{
		varname_size = getEnvStrSize(&env->buf[off]); // var name string size
		vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size

		oldentrysize = 2 + varname_size + 2 + vardata_size;

		if(varvalue)
		{
			if( strlen(varvalue) + 1 > vardata_size )
			{
				// add new entry, and pack the strings
				ret = pushEnvEntry(env, varname, varvalue);
				if( ret > 0 )
				{
					for(i=0;i<env->bufsize - (off + oldentrysize);i++)
					{
						env->buf[off + i] = env->buf[off + i + oldentrysize];
					}

					for(int j=0;j<4;j++)
					{
						if(off + i < env->bufsize)
						{
							env->buf[off + i] = '\0';
							i++;
						}
					}
				}
			}
			else
			{
				vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size

				strcpy( (char*)&env->buf[off + 2 + varname_size + 2], varvalue );
			}
		}
		else
		{
			// unset variable
			for(i=0;i<env->bufsize - (off + oldentrysize);i++)
			{
				env->buf[off + i] = env->buf[off + i + oldentrysize];
			}

			for(int j=0;j<4;j++)
			{
				if(off + i < env->bufsize)
				{
					env->buf[off + i] = '\0';
					i++;
				}
			}
		}
	}
	else
	{
		if(varvalue)
		{
			// New variable
			ret = pushEnvEntry(env, varname, varvalue);
		}
	}

	return env;
}

char * getEnvVar( envvar_entry * env, char * varname, char * varvalue)
{
	int off;
	unsigned short varname_size, vardata_size;

	envvar_entry * tmp_envvars;

	tmp_envvars = (envvar_entry *)env;
	if(!tmp_envvars)
		return NULL;

	off = getEnvBufOff( env, varname );
	if( off >= 0 )
	{
		varname_size = getEnvStrSize(&env->buf[off]); // var name string size
		vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size

		if( varname_size>0 && vardata_size>0)
		{
			if(varvalue)
				strncpy(varvalue,(char*)&env->buf[off + 2 + varname_size + 2], vardata_size);

			return (char*)&env->buf[off + 2 + varname_size + 2];
		}
	}

	return NULL;
}

env_var_value getEnvVarValue( envvar_entry * env, char * varname)
{
	env_var_value value;
	char * str_return;

	value = 0;

	if(!varname)
		return 0;

	str_return = getEnvVar( env, varname, NULL);

	if(str_return)
	{
		if( strlen(str_return) > 2 )
		{
			if( str_return[0]=='0' && ( str_return[1]=='x' || str_return[1]=='X'))
			{
				value = (env_var_value)STRTOVALUE(str_return, NULL, 0);
			}
			else
			{
				value = atoi(str_return);
			}
		}
		else
		{
			value = atoi(str_return);
		}
	}

	return value;
}

envvar_entry * setEnvVarValue( envvar_entry * env, char * varname, env_var_value value)
{
	char tmp_str[128];

	tmp_str[128 - 1] = 0;

	snprintf(tmp_str,sizeof(tmp_str) - 1, "%d",value);

	return setEnvVar( env, varname, tmp_str );
}

char * getEnvVarIndex( envvar_entry * env, int index, char * varvalue)
{
	int str_index, off;
	unsigned short varname_size, vardata_size;

	off = 0;

	str_index = 0;

	varname_size = getEnvStrSize(&env->buf[off]); // var name string size
	vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size

	while( varname_size )
	{
		if( str_index == index )
		{
			if( varname_size>0 && vardata_size>0)
			{
				if(varvalue)
					strncpy(varvalue,(char*)&env->buf[off + 2 + varname_size + 2], vardata_size);

				return (char*)&env->buf[off + 2 + varname_size + 2];
			}
		}

		off += (2 + varname_size + 2 + vardata_size);

		varname_size = getEnvStrSize(&env->buf[off]); // var name string size
		vardata_size = getEnvStrSize(&env->buf[off + 2 + varname_size]); // var data string size

		str_index++;
	}

	return NULL; // Not found.
}

envvar_entry * duplicate_env_vars(envvar_entry * src)
{
#ifndef STATIC_ENV_BUFFER
	envvar_entry * tmp_envvars;
#endif

	if(!src)
		return NULL;

	if(!src->buf)
		return NULL;

#ifndef STATIC_ENV_BUFFER
	tmp_envvars = malloc(sizeof(envvar_entry));
	if(tmp_envvars)
	{
		memset(tmp_envvars,0,sizeof(envvar_entry));
		tmp_envvars->buf = malloc(src->bufsize);
		if(!tmp_envvars->buf)
		{
			free(tmp_envvars);
			return NULL;
		}

		memcpy(tmp_envvars->buf,src->buf,src->bufsize);

		return tmp_envvars;
	}
#endif
	return NULL;
}

void free_env_vars(envvar_entry * src)
{
#ifndef STATIC_ENV_BUFFER
	if(!src)
		return;

	if(src->buf)
		free(src->buf);

	free(src);
#endif
	return;
}
