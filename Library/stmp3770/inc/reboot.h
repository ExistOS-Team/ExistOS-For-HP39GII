

#ifndef _REBOOT_H_
#define _REBOOT_H_

#define HW_CLKCTRL_RESET *((unsigned int *)(0x800400F0))

#ifdef __cplusplus
extern "C" {
#endif

void reboot_test(int option); //reboot to flash. 1=entire reset, 2=reset the digital sections of the chip, 3 or any number else=nothing to do.

#ifdef __cplusplus
};
#endif

#endif
