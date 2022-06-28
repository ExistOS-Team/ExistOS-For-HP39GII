#include "timer.h"
#include "interrupt.h"
#include "cpu.h"

static unsigned int prev_time;
static unsigned short ticks = 0;
static unsigned int reload;
static unsigned int reloaded;

static unsigned char tac;
static unsigned int started;
static unsigned int speed;
static unsigned short counter;
static unsigned int modulo;

/* Any write to this register resets the timer */
void timer_set_div(unsigned char v)
{
	(void) v;
	unsigned int bit;

	bit = speed >> 1;

	/* Counter is incremented if the high bit transitions 1->0 */
	if(ticks & bit)
	{
		counter++;
		if(counter == 0x100)
		{
			counter = 0;
			reload = 1;
		}
	}
	ticks = 0;
}

unsigned char timer_get_div(void)
{
	return ticks>>8;
}

void timer_set_counter(unsigned char v)
{
	if(reload)
		reload = 0;
	if(!reloaded)
		counter = v;
}

unsigned char timer_get_counter(void)
{
	return counter;
}

void timer_set_modulo(unsigned char v)
{
	if(reloaded)
		counter = v;
	modulo = v;
}

unsigned char timer_get_modulo(void)
{
	return modulo;
}

void timer_set_tac(unsigned char v)
{
	int speeds[] = {1024, 16, 64, 256};

	tac = v;

	/* TAC high -> low */
	if(started && !(v&4))
	{
		if(ticks & (speed >> 1))
		{
			counter++;
			if(counter == 0x100)
			{
				counter = 0;
				reload = 1;
			}
		}
	}

	if(ticks & (speed >> 1) && !(ticks & (speeds[v&3]>>1)))
	{
		counter++;
		if(counter == 0x100)
		{
			counter = 0;
			reload = 1;
		}
	}

	started = v&4;
	speed = speeds[v&3];
}

unsigned char timer_get_tac(void)
{
	return tac;
}

static void timer_tick(int delta)
{
	while(delta--)
	{
		unsigned int old_ticks;

		reloaded = 0;

		/* Delays the counter reload by a tick */
		if(reload)
		{
			reload = 0;
			counter = modulo;
			reloaded = 1;
			interrupt(INTR_TIMER);
		}

		old_ticks = ticks;
		/* Our CPU runs at 1MHz, timer is 4MHz */
		ticks += 4;

		if(!started)
			continue;

		if(((old_ticks & (speed>>1)))
		 && (ticks & (speed>>1)) == 0)
			counter++;

		if(counter == 0x100)
		{
			counter = 0;
			reload = 1;
		}
	}
}

void timer_cycle(void)
{
	/* The amount of ticks since we last ran */
	unsigned int delta;

	delta = cpu_get_cycles() - prev_time;
	prev_time = cpu_get_cycles();

	timer_tick(delta);
}
