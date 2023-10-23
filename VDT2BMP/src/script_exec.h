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
 Minitel Server script engine

 (C) 2022-2O23 Jean-François DEL NERO
*/

#define _script_ctx_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef _script_printf_func_
typedef int (* SCRIPT_PRINTF_FUNC)(void * ctx, int MSGTYPE, char * string, ... );
#define _script_printf_func_
#endif

#ifdef SCRIPT_64BITS_SUPPORT
#define env_var_value uint64_t
#define STRTOVALUE strtoull
#define LONGHEXSTR "%llX"
#else
#define env_var_value uint32_t
#define STRTOVALUE strtoul
#define LONGHEXSTR "%.8X"
#endif

#define DEFAULT_BUFLEN 1024 
#define MAX_LABEL_SIZE 64
#define MAX_LABEL 256

typedef struct _script_label
{
	char label_name[MAX_LABEL_SIZE];
	unsigned int offset;
} script_label;

typedef struct _script_ctx
{
	SCRIPT_PRINTF_FUNC script_printf;
	void * app_ctx;

	void * env;

	void * cmdlist;

	FILE * script_file;
	char script_file_path[1024];

	int cur_label_index;
	script_label labels[MAX_LABEL];

	int cur_script_offset;

	int dry_run;

	int last_error_code;
	env_var_value last_data_value;
	int last_flags;

	char pre_command[1024 + 32];

	uint32_t rand_seed;

} script_ctx;

script_ctx * init_script(void * app_ctx, unsigned int flags, void * env);
int  execute_file_script( script_ctx * ctx, char * filename );
int  execute_line_script( script_ctx * ctx, char * line );
int  execute_ram_script( script_ctx * ctx, unsigned char * script_buffer, int buffersize );
void setOutputFunc_script( script_ctx * ctx, SCRIPT_PRINTF_FUNC ext_printf );
script_ctx * deinit_script(script_ctx * ctx);

// Output Message level
#define MSG_INFO_0                       0
#define MSG_INFO_1                       1
#define MSG_WARNING                      2
#define MSG_ERROR                        3
#define MSG_DEBUG                        4
#define MSG_NONE                         5

#ifdef __cplusplus
}
#endif
