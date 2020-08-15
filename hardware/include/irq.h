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
 
#ifndef _IRQ_H
#define _IRQ_H




#define IRQ_VECTOR_TABLE_BASE_ADDR	0x00000100


void irq_init();
void irq_set_enable(unsigned int irq_n, unsigned int enable);
void irq_install_serveice(unsigned int irq_n,unsigned int *service_program);

void enable_interrupts();
void disable_interrupts();




#endif

