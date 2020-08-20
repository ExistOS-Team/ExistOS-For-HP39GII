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
#include "keyboard.h"
extern "C"{

	#include "utils.h"
	void timer_init();
	char timer_set(char n, unsigned short count, unsigned int *callback);
	void timer_start(char n);
	void timer_stop(char n);
//=======
	void rtc_init();
	void rtc_ms_set(unsigned int count);
	unsigned int rtc_ms_get();
	void rtc_ms_reset();
	char rtc_sec_set(unsigned int count);
	unsigned int rtc_sec_get();
	char rtc_persistent_set(char n, unsigned int general);
	unsigned int rtc_persistent_get(char n);
	
	extern void fs_test_main();

}

using namespace std;
Screen sc;
vector<int> vec;

int main(){
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
	rtc_init();
	rtc_ms_set(1000);
	for (int i = 0; i < 5; i++) {
		printf("%d\n", rtc_ms_get());
	}
	printf("set: 0x%x\n", rtc_sec_set(10));
	int volatile k = 1000000;
	while (k--)
		;
	printf("0x%x\n", rtc_sec_get());
	printf("set: 0x%x\n", rtc_persistent_set(2, 0x12345678));
	printf("0x%x\n", rtc_persistent_get(2));
	
	fs_test_main();
	
    reboot_test(3);   //reboot to flash. 1=entire reset, 2=reset the digital sections of the chip, 3 or any number else=nothing to do
    

	return 0;
}


