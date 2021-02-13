#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ti8xemu.h"
#include "z80.h"
#include "hardware.h"

#define USE_CALC	86

void emu_main()
{          

	Z80_Reset();

	SetCalc(USE_CALC);
    
	LoadMEM();

	Z80();
}
