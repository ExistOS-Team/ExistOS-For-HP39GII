#include "interrupt.h"
#include "cpu.h"

static int enabled;

/* Pending interrupt flags */
static int interrupt_IF = 0xE0;

/* Interrupt masks */
static int interrupt_IE = 0;

int interrupt_pending(void)
{
	return interrupt_IF & interrupt_IE & 0x1F;
}

void interrupt_flush(void)
{
	unsigned int pending;

	pending = interrupt_pending();

	if(!pending)
		return;

	if(!enabled)
	{
		if(cpu_halted())
			cpu_unhalt();
		return;
	}

	cpu_interrupt_begin();

	/* Check again here incase the above changed IF through PUSH SP (push_ei.gb) */
	pending = interrupt_pending();

	if(pending & INTR_VBLANK)
	{
		interrupt_IF ^= INTR_VBLANK;
		cpu_interrupt(0x40);
	}
	else if(pending & INTR_LCDSTAT)
	{
		interrupt_IF ^= INTR_LCDSTAT;
		cpu_interrupt(0x48);
	}
	else if(pending & INTR_TIMER)
	{
		interrupt_IF ^= INTR_TIMER;
		cpu_interrupt(0x50);
	}
	else if(pending & INTR_SERIAL)
	{
		interrupt_IF ^= INTR_SERIAL;
		cpu_interrupt(0x58);
	}
	else if(pending & INTR_JOYPAD)
	{
		interrupt_IF ^= INTR_JOYPAD;
		cpu_interrupt(0x60);
	}
	else
	{
		cpu_interrupt(0x00);
	}
}

int interrupt_enabled(void)
{
	return enabled;
}

void interrupt_enable(void)
{
	enabled = 1;
}

void interrupt_disable(void)
{
	enabled = 0;
}

int interrupt_get_enabled(void)
{
	return enabled;
}

void interrupt(unsigned int n)
{
	interrupt_IF |= n;
}

unsigned char interrupt_get_IF(void)
{
	return interrupt_IF;
}

void interrupt_set_IF(unsigned char mask)
{
	interrupt_IF = 0xE0 | mask;
}

unsigned char interrupt_get_mask(void)
{
	return interrupt_IE;
}

void interrupt_set_mask(unsigned char mask)
{
	interrupt_IE = mask;
}
