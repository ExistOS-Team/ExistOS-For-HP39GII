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
#include <reboot.h>
#include <cstring>
#include <vector>

#include "display.h"
#include "memory.h"
#include "dcp.h"
#include "irq.h"
#include "hw_irq.h"
#include "keyboard.h"

extern "C"{

	#include "utils.h"
	reg8_t overclock(reg8_t frac, reg8_t isFracEnabled, reg16_t divider, reg8_t isHbusFracEnabled, reg16_t hbusDivider, reg8_t isAutoSlow);

	extern void fs_test_main();
	extern void coremark_main();

};

using namespace std;
Screen sc;
vector<int> vec;

int main()
{
	overclock(18, 0, 2, 0, 1, 1);
/*
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
*/
	//delay_us(5000000);
//=======
	
	// fs_test_main();
	// 
	// printf("Running Coremark!!\n");
	// coremark_main();
    // reboot_test(3);   //reboot to flash. 1=entire reset, 2=reset the digital sections of the chip, 3 or any number else=nothing to do
}

