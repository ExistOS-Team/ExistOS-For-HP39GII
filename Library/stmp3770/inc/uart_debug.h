/*
 * Boot Prep common file
 *
 * Copyright 2008-2009 Freescale Semiconductor
 *
 * This program is free software; you can redistribute it and/or modify
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
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */

#ifndef _DEBUG_H_
#define _DEBUG_H_
#ifdef __cplusplus
extern "C" {
#endif
void uartdbg_printhex(int data);
void uartdbg_printf(char *fmt, ...);
void uartdbg_putc(char ch);
void uartdbg_print_regs();
#ifdef __cplusplus
};
#endif
#endif
