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
 * \brief Radio config.
 *
 * \author Daniele Basile <asterix@develer.com>
 */

#ifndef RADIO_CFG_H
#define RADIO_CFG_H

#include "measure.h"

#include <cpu/types.h>

/*
 * Radio ids table.
 */
#define RADIO_MASTER    0

typedef struct RadioCfg
{
	uint8_t id;
	uint8_t fmt[10];
	size_t fmt_len;
	measure_t callbacks[10];
} RadioCfg;

const RadioCfg *radio_cfg(int id);
uint8_t radio_cfg_id(void);
void radio_cfg_init(void);

#endif /* RADIO_CFG_H */
