/*
 * A simple user interface for this project
 *
 * Copyright 2020 LiberCalc Team
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
 
#include "main.h"
#include <stdio.h>

#include <string.h>
#include <Screen.h>
#include <cstring>
#include <vector>

#include "memory.h"

extern "C"{
	void timer_init();
	char timer_set(char n, char isRepeat, char accuracy, unsigned int *callback);
	void timer_start(char n, unsigned short count);
	void timer_stop(char n);
	void timer_reset(char n);
	unsigned short timer_get(char n);
}

using namespace std;
Screen sc;
vector<int> vec;

void callback(){
	printf("callback.\n");
	fflush(stdout);
}

int main(){
	timer_init();
	printf("set: 0x%x.\n", timer_set(0, 1, 0x8, (unsigned int *)callback));
	fflush(stdout);
	timer_start(0, 32000);
	return 0;
}


