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

#include "display.h"
#include "memory.h"
#include "keyboard.h"
extern "C"{
	
	#include "utils.h"
	void timer_init();
	char timer_set(char n, unsigned short count, unsigned int *callback);
	void timer_start(char n);
	void timer_stop(char n);
}

using namespace std;
Screen sc;
vector<int> vec;

void callback(){
	printf("callback.\n");
	fflush(stdout);
}

int main(){
	printf("main");
	fflush(stdout);
	printf("Key test mode!!Press enter or on");
	fflush(stdout);
	key_scan();
	while (!is_key_down(KEY_ON)){
		fflush(stdout);
		key_scan();
		delay_us(500000);
		if(is_key_down(KEY_ENTER)) printf("Enter pressed!/n");
	};
	
	//delay_us(5000000);
	return 0;
}