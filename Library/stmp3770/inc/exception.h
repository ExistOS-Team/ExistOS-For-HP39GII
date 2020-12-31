/*
 * A simple user interface for this project
 *
 * Copyright 2020 Creep_er
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
 
#ifndef _EXCEPTION_H
#define _EXCEPTION_H
#ifdef __cplusplus 
extern "C" { 
#endif

#define EXCEPTION_VECTOR_TABLE_BASE_ADDR	0x00000000

typedef enum exception_type{
	EXCEPTION_RESET = 0,
	EXCEPTION_UND,
	EXCEPTION_SWI,
	EXCEPTION_PABORT,
	EXCEPTION_DABORT,
	EXCEPTION_RESERVED,
	EXCEPTION_IRQ,
	EXCEPTION_FIQ
}exception_type;

void exception_init();
void exception_install(exception_type type, unsigned int *exception_handler_addr);
void install_swi_service(unsigned int swi_num, void *service);
#ifdef __cplusplus 
} 
#endif
#endif

