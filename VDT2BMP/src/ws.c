/*
//
// Copyright (C) 2022-2024 Jean-François DEL NERO
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
 websocket layer
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>

#include "env.h"
#include "cache.h"
#include "wave.h"
#include "videotex.h"
#include "bmp_file.h"
#include "fifo.h"
#include "modem.h"
#include "dtmf.h"
#include "FIR/band_pass_rx_Filter.h"
#include "FIR/low_pass_tx_Filter.h"

#include "vdt2bmp.h"
#include "ws.h"
#include "libwsclient/wsclient.h"

int onclose(wsclient *c) {
	fprintf(stderr, "ws onclose called: %d\n", c->sockfd);
	return 0;
}

int onerror(wsclient *c, wsclient_error *err) {
	fprintf(stderr, "ws onerror: (%d): %s\n", err->code, err->str);
	return 0;
}

int onmessage(wsclient *c, wsclient_message *msg) {
	int i;
	app_ctx * ctx;

	ctx = c->app_ctx;

	for(i=0;i<msg->payload_len;i++)
	{
		while ( is_fifo_full(&ctx->mdm->tx_fifo) )
		{
			usleep(1000);
		}
		push_to_fifo(&ctx->mdm->tx_fifo, (unsigned char)msg->payload[i]);
	}

	return 0;
}

int onopen(wsclient *c) {
	fprintf(stderr, "ws onopen called: %d\n", c->sockfd);
	return 0;
}

void * init_ws(void * app_ctx)
{
	websocket_ctx * ctx;
	wsclient *client;

	ctx = malloc(sizeof(websocket_ctx));
	if(ctx)
	{
		memset(ctx,0,sizeof(websocket_ctx));

		client = libwsclient_new("ws://go.minipavi.fr:8182", app_ctx);
		if(!client) {
			fprintf(stderr, "Unable to initialize new WS client.\n");
			free(ctx);
			return NULL;
		}

		//set callback functions for this client
		libwsclient_onopen(client, &onopen);
		libwsclient_onmessage(client, &onmessage);
		libwsclient_onerror(client, &onerror);
		libwsclient_onclose(client, &onclose);

		ctx->libwsclient_ctx = client;

		libwsclient_run(client);
	}

	return ctx;
}

void ws_send(void *ws, unsigned char c)
{
	websocket_ctx * ctx;
	char str[2];

	if(ws)
	{
		str[1] = 0;
		str[0] = c;

		ctx = (websocket_ctx *)ws;
		libwsclient_send(ctx->libwsclient_ctx, str);
	}
}

void deinit_ws(void *ws)
{
	websocket_ctx * ctx;

	ctx = (websocket_ctx *)ws;
	libwsclient_finish(ctx->libwsclient_ctx);
}
