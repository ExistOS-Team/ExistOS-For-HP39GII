#ifndef __HARDWARE_H__
#define __HARDWARE_H__

int Address(dword addr);
void TICalcExit(byte DoSaveMEM);
void LoadMEM();
void LoadROM(void);
void LoadRAM(void);
void DrawScreen(void);
void SaveRAM(void);

extern Z80_Regs R;

#endif