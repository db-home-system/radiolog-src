/**
 * \file
 * <!--
 * This file is part of BeRTOS.
 *
 * Bertos is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * As a special exception, you may use this file as part of a free software
 * library without restriction.  Specifically, if other files instantiate
 * templates or use macros or inline functions from this file, or you compile
 * this file and link it with other files to produce an executable, this
 * file does not by itself cause the resulting executable to be covered by
 * the GNU General Public License.  This exception does not however
 * invalidate any other reasons why the executable file might be covered by
 * the GNU General Public License.
 *
 * Copyright 2013 Develer S.r.l. (http://www.develer.com/)
 * All Rights Reserved.
 * -->
 *
 * \brief Radio commands.
 *
 * \author Daniele Basile <asterix@develer.com>
 */

#include "protocol.h"

#include <cfg/debug.h>

#include <string.h>

static Devices local_dev[CMD_DEVICES];

/*
 * MASTER COMMAND FUNCTIONS
 */

static int cmd_master_broadcast(KFile *fd, Protocol *proto)
{
	uint8_t reply = 0;
	for (int i = 0; i < 5; i++)
	{
		if (local_dev[i].addr == proto->addr)
		{
			reply = PROTO_ACK;
			break;
		}
		else if (!local_dev[i].addr)
		{
			local_dev[i].addr =  proto->addr;
			local_dev[i].len =  proto->len;
			memcpy(local_dev[i].data, proto->data, proto->len);
			reply = PROTO_ACK;
			break;
		}
		else
			reply = PROTO_NACK;
	}

	//proto->data[proto->len] = '\0';
	//kprintf("type[%d], addr[%d], data[%s]\n", proto->type, proto->addr, proto->data);

	if (protocol_reply(fd, proto, proto->addr, &reply, sizeof(reply)))
		return 0;

	return -1;
}

static int cmd_master_data(KFile *_fd, Protocol *proto)
{
	//kprintf("type[%d], addr[%d]\n", proto->type, proto->addr);

	//kprintf("Decode data:len[%d]\n", proto->len);
	Radio *fd = RADIO_CAST(_fd);
	kprintf("%d;%d;%d;", proto->addr, fd->lqi, fd->rssi);
	int i = 0;
	for (i = 0; i < CMD_DEVICES; i++)
	{
		if (local_dev[i].addr == proto->addr)
		{
			size_t index = 0;
			//kprintf("l[%d],d[%s]\n", local_dev[i].len, local_dev[i].data);
			for (size_t j = 0; j < local_dev[i].len; j++)
			{
				if (local_dev[i].data[j] == 'h')
				{
					ASSERT(index <= proto->len);
					int16_t d;
					memcpy(&d, &proto->data[index], sizeof(int16_t));
					kprintf("%d;", d);
					index += sizeof(int16_t);
				}
				if (local_dev[i].data[j] == 'H')
				{
					ASSERT(index <= proto->len);
					uint16_t d;
					memcpy(&d, &proto->data[index], sizeof(uint16_t));
					kprintf("%d;", d);
					index += sizeof(uint16_t);
				}
			}

			break;
		}
	}
	/* Remove from list served devices */
	memset(&local_dev[i], 0, sizeof(Devices));
	kputs("\n");

	return 0;
}

const Cmd master_cmd[] = {
	{ CMD_TYPE_BROADCAST, cmd_master_broadcast     },
	{ CMD_TYPE_DATA, cmd_master_data      },
	{ 0   , NULL }
};


/*
 * SLAVE COMMAND FUNCTIONS
 */


static int cmd_slave_data(KFile *_fd, Protocol *proto)
{
		kprintf("ACK, Send data..\n");
		size_t index = 0;

		for (size_t i = 0; i < cfg->fmt_len; i++)
		{
			if (cfg->fmt[i] == 'h')
			{
				int16_t d;
				ASSERT(cfg->callbacks[i]);
				cfg->callbacks[i]((uint8_t *)&d, sizeof(d));
				memcpy(&tmp[index], (uint8_t *)&d, sizeof(d));
				index += sizeof(d);
				kprintf("%d;", d);
			}
			if (cfg->fmt[i] == 'H')
			{
				uint16_t d;
				ASSERT(cfg->callbacks[i]);
				cfg->callbacks[i]((uint8_t *)&d, sizeof(d));
				memcpy(&tmp[index], (uint8_t *)&d, sizeof(d));
				index += sizeof(d);
				kprintf("%d;", d);
			}
		}
		kputs("\n");

		ASSERT(index <= 60);
		protocol_data(&radio.fd, &proto, id, tmp, index);
}

static int cmd_slave_broadcast(KFile *_fd, Protocol *proto)
{
	int ret = protocol_checkReply(&radio.fd, &proto);
	if (ret == PROTO_ACK)
	{
		// Rimango in attesa del prossimo comando
		// dal master, non vado in stop mode
		// disattivare il watchdog.
		// o attivare il timer per farlo resettare.
		return 0;
	}

	/*
	 * Riprovo a inviare il messaggio di broadcast
	 * tra un po' di tempo perchè il master è occupato
	 * o non ha capito.
	 */
	if (ret == PROTO_NACK)
	{
		sent = protocol_broadcast(&radio.fd, &proto, id, cfg->fmt, cfg->fmt_len);
		kprintf("Sent[%d]\n", sent);
	}

	kprintf("err[%d]\n", ret);
	return ret;
}

const Cmd slave_cmd[] = {
	{ CMD_TYPE_BROADCAST,     cmd_slave_broadcast    }, /* Check reply from master    */
	{ CMD_TYPE_DATA, cmd_slave_data }, /* Send all measure to master */
	{ 0     , NULL }
};

void cmd_poll(void)
{
	for (int i = 0; i < CMD_DEVICES; i++)
	{
		kprintf("%d: ", i);
		if (local_dev[i].addr)
			kprintf("Addr[%d],data[%s],len[%d]\n", local_dev[i].addr, local_dev[i].data, local_dev[i].len);
		else
			kprintf("Empty\n");
	}
	kputs("-----\n");
}

