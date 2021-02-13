#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "ti8xemu.h"
#include "z80.h"
#include "z80io.h"
#include "hardware.h"
//#include "key.h"

static int key_stuck = 0;
static int ROMSIZE;
static int RAMSIZE;
static int SAVSIZE;
static int BMPSIZE;
static byte CALC;
static char LINK = 'K';
static word LinkPort = 0;
static byte LinkReg = 0;
static byte LinkSendBit = 0;
static byte LinkSendByte = 0;
static byte LinkRecvBit = 0;
static byte LinkRecvByte = 0;
static byte LinkRecvAgain = 0;
static byte InitRAM = 0;
static byte *ROM;
static byte RAM[0x20030];
static byte Mem_Type = 0;
static byte Sector1_Type = 0;
static byte Sector2_Type = 0;
static byte Sector1_Page = 0;
static byte Sector2_Page = 0;
static int xInt;
static byte xByte;
static byte KeypadMask = 0;
static byte ONmask = 0;
static byte LCDmask = 0;
static byte LCDon = 8;
static byte PowerReg = 0;
static byte VidOffset = 0x3C;
static byte Contrast = 0;
static byte VidRAM[0x600];
static byte VidX = 0;
static byte VidY = 0;
static byte VidDir = 7;
static byte VidMode = 0;
static byte VidScroll = 0;
static byte VidCol,VidBit,VidByte;
static byte* VidPtr;

static dword* lcdmem;
static dword* vidmem;
static byte VIDJUMP;
static byte LCDWIDTH;

byte invert[4] = {3,1,2,0};


byte KeyboardRead(void)
{
	return 0xFF;
	/*
  if ((KeyMatrixLo == 0)&&(KeyMatrixHi == 0))
    return 0xFF;
	     
	xByte = 0xFF;
	if((CALC == 85) || (CALC == 86))
	{
		if(~KeypadMask&1)
		{
			if(kb_key_h(KB_SCAN_DOWN))
				xByte ^= 1;
			if(kb_key_h(KB_SCAN_LEFT))
				xByte ^= 2;
			if(kb_key_h(KB_SCAN_RIGHT))
				xByte ^= 4;
			if(kb_key_h(KB_SCAN_UP))
				xByte ^= 8;
		}

		if(~KeypadMask&2)
		{
			if(kb_key(KB_SCAN_ENTER))
				xByte ^= 1;
			if(kb_key(KB_SCAN_X))
				xByte ^= 2;
			if(kb_key(KB_SCAN_T))
				xByte ^= 4;
			if(kb_key(KB_SCAN_O))
				xByte ^= 8;
			if(kb_key(KB_SCAN_J))
				xByte ^= 16;
			if(kb_key(KB_SCAN_E))
				xByte ^= 32;
			if(kb_key(KB_SCAN_BACKSPACE))
				xByte ^= 64;
		}

		if(~KeypadMask&4)
		{
			if(kb_key(KB_SCAN_MINUS))
				xByte ^= 1;
			if(kb_key(KB_SCAN_W))
				xByte ^= 2;
			if(kb_key(KB_SCAN_S))
				xByte ^= 4;
			if(kb_key(KB_SCAN_N))
				xByte ^= 8;
			if(kb_key(KB_SCAN_I))
				xByte ^= 16;
			if(kb_key(KB_SCAN_D))
				xByte ^= 32;
			if(kb_key(KB_SCAN_F10))
				xByte ^= 64;
		}

		if(~KeypadMask&8)
		{
			if(kb_key(KB_SCAN_Z))
				xByte ^= 1;
			if(kb_key(KB_SCAN_V))
				xByte ^= 2;
			if(kb_key(KB_SCAN_R))
				xByte ^= 4;
			if(kb_key(KB_SCAN_M))
				xByte ^= 8;
			if(kb_key(KB_SCAN_H))
				xByte ^= 16;
			if(kb_key(KB_SCAN_C))
				xByte ^= 32;
			if(kb_key(KB_SCAN_F9))
				xByte ^= 64;
			if(kb_key_h(KB_SCAN_DELETE))
				xByte ^= 128;
		}

		if(~KeypadMask&16)
		{
			if(kb_key(KB_SCAN_Y))
				xByte ^= 1;
			if(kb_key(KB_SCAN_U))
				xByte ^= 2;
			if(kb_key(KB_SCAN_Q))
				xByte ^= 4;
			if(kb_key(KB_SCAN_L))
				xByte ^= 8;
			if(kb_key(KB_SCAN_G))
				xByte ^= 16;
			if(kb_key(KB_SCAN_B))
				xByte ^= 32;
			if(kb_key(KB_SCAN_F8))
				xByte ^= 64;
			if(kb_key_h(KB_SCAN_QUOTE))
				xByte ^= 128;
		}

		if(~KeypadMask&32)
		{
			if(kb_key_h(KB_SCAN_EQUAL))
				xByte ^= 2;
			if(kb_key_h(KB_SCAN_P))
				xByte ^= 4;
			if(kb_key_h(KB_SCAN_K))
				xByte ^= 8;
			if(kb_key_h(KB_SCAN_F))
				xByte ^= 16;
			if(kb_key_h(KB_SCAN_A))
				xByte ^= 32;
			if(kb_key_h(KB_SCAN_F7))
				xByte ^= 64;
			if(kb_key_h(KB_SCAN_LCONTROL))
				xByte ^= 128;
		}

		if(~KeypadMask&64)
		{
			if(kb_key_h(KB_SCAN_F5))
				xByte ^= 1;
			if(kb_key_h(KB_SCAN_F4))
				xByte ^= 2;
			if(kb_key_h(KB_SCAN_F3))
				xByte ^= 4;
			if(kb_key_h(KB_SCAN_F2))
				xByte ^= 8;
			if(kb_key_h(KB_SCAN_F1))
				xByte ^= 16;
			if(kb_key_h(KB_SCAN_LSHIFT))
				xByte ^= 32;
			if(kb_key_h(KB_SCAN_ESC))
				xByte ^= 64;
			if(kb_key_h(KB_SCAN_TAB))
				xByte ^= 128;
		}
	}
        return xByte;
		*/
}

byte LinkRecv(void)
{
	if(CALC == 83)
                {
                        if(LINK == 'G')
                        {                              
                                if(0 && !LinkRecvBit && !LinkRecvAgain) //0:SERIAL CACHE NOT EMPTY
                                {                                                         
                                        LinkRecvByte = 0x00; // COMM RECV BYTE
                                        LinkRecvBit = 1;
                                        LinkReg = 3;                                     
                                }

                                if(LinkRecvBit || LinkRecvAgain)
                                {
                                        LinkRecvAgain = 0;

                                        if(LinkReg == 0)
                                        {
                                                LinkReg = 3;                                                
                                                return ((Sector1_Page & 8) << 1) | 0xC;
                                        }
                                        if(LinkReg == 1)
                                                return ((Sector1_Page & 8) << 1) | LinkReg | 8;
                                        if(LinkReg == 2)
                                                return ((Sector1_Page & 8) << 1) | LinkReg | 4;                                        
                                        
                                        xByte = ((Sector1_Page & 8) << 1) | LinkReg | ((LinkRecvByte & LinkRecvBit) ? 4 : 8);

                                        return xByte;
                                }

                                if((LinkReg == 1) || (LinkReg == 2))
                                        return ((Sector1_Page & 8) << 1) | LinkReg;                            

                                return ((Sector1_Page & 8) << 1) | LinkReg | 0xC;
                        }
                        if(LINK == 'P')
                                return ((Sector1_Page & 8) << 1) | LinkReg | ((0xFF & 0x30) >> 2);
                        if(LINK == 'S')
                                return ((Sector1_Page & 8) << 1) | LinkReg | ((0xFF & 0x30) >> 2);
                        if(LINK == 'K')
                                return ((Sector1_Page & 8) << 1) | LinkReg | 0xC;                  
                }
                else
                {
                        if(LINK == 'G')
                        {                              
                                if(0 && !LinkRecvBit && !LinkRecvAgain)//REM
                                {                   
                                        LinkRecvByte = 0x00;//REM
                                        LinkRecvBit = 1;
                                        LinkReg = 3;                                     
                                }

                                if(LinkRecvBit || LinkRecvAgain)
                                {
                                        LinkRecvAgain = 0;

                                        if(LinkReg == 0)
                                        {
                                                LinkReg = 3;                                                
                                                return 3;
                                        }
                                        if(LinkReg == 1)
                                                return (LinkReg << 2) | 2;
                                        if(LinkReg == 2)
                                                return (LinkReg << 2) | 1;                                        
                                        
                                        xByte = (LinkReg << 2) | ((LinkRecvByte & LinkRecvBit) ? 1 : 2);

                                        return xByte;
                                }

                                if((LinkReg == 1) || (LinkReg == 2))
                                        return LinkReg << 2;                            

                                return (LinkReg << 2) | 3;
                        }
                        if(LINK == 'P')
                                return (LinkReg << 2) | ((0xFF & 0x30) >> 4);
                        if(LINK == 'S')
                                return (LinkReg << 2) | ((0xFF & 0x30) >> 4);
                        if(LINK == 'K')
                                return (LinkReg << 2) | 3;                  
                }
}

void LinkSend(byte Value)
{
	if(CALC == 83)
		{
                        Sector1_Page = (Sector1_Page & 7) | ((Value & 16) >> 1);                                         

                        LinkReg = Value & 3;

                        if(LINK == 'G')
                        {                                
                                switch(LinkReg)
                                {
                                        case 1:
                                                if(!LinkRecvBit)
                                                {
                                                        if(!LinkSendBit)
                                                                LinkSendBit = 1;                                                                                                                                                                                    

                                                        LinkSendBit <<= 1;

                                                        if(!LinkSendBit)
                                                        {
                                                                //comm_port_out(Comm,LinkSendByte);
                                                                LinkSendByte = 0;
                                                        }                                                    
                                                }
                                                else
                                                {
                                                        LinkRecvBit <<= 1;

                                                        if(!LinkRecvBit)
                                                                LinkRecvAgain = 1;
                                                }
                                                break;
                                        case 2:
                                                if(!LinkRecvBit)
                                                {
                                                        if(!LinkSendBit)
                                                                LinkSendBit = 1;

                                                        LinkSendByte |= LinkSendBit;

                                                        LinkSendBit <<= 1;

                                                        if(!LinkSendBit)
                                                        {
                                                                //comm_port_out(Comm,LinkSendByte);
                                                                LinkSendByte = 0;
                                                        }
                                                }
                                                else
                                                {
                                                        LinkRecvBit <<= 1;

                                                        if(!LinkRecvBit)
                                                                LinkRecvAgain = 1;
                                                }
                                }                                    
                        }

                        /*if(LINK == 'P')
                                outp(LinkPort, invert[LinkReg]);
                        if(LINK == 'S')                                
                                outp(LinkPort+4, invert[LinkReg]);                                                           
                        if(LINK == 'K')
                                outp(0x61, (inp(0x61) &~ 0x03) | (LinkReg ? 0x2 : 0x0));*/
		}
                else
		{                   
                        LinkReg = (Value & 0xC) >> 2;

                        if(LINK == 'G')
                        {                                
                                switch(LinkReg)
                                {
                                        case 1:
                                                if(!LinkRecvBit)
                                                {
                                                        if(!LinkSendBit)
                                                                LinkSendBit = 1;                                                                                                                                                                                    

                                                        LinkSendBit <<= 1;

                                                        if(!LinkSendBit)
                                                        {
                                                                //comm_port_out(Comm,LinkSendByte);
                                                                LinkSendByte = 0;
                                                        }                                                    
                                                }
                                                else
                                                {
                                                        LinkRecvBit <<= 1;

                                                        if(!LinkRecvBit)
                                                                LinkRecvAgain = 1;
                                                }
                                                break;
                                        case 2:
                                                if(!LinkRecvBit)
                                                {
                                                        if(!LinkSendBit)
                                                                LinkSendBit = 1;

                                                        LinkSendByte |= LinkSendBit;

                                                        LinkSendBit <<= 1;

                                                        if(!LinkSendBit)
                                                        {
                                                                //comm_port_out(Comm,LinkSendByte);
                                                                LinkSendByte = 0;
                                                        }
                                                }
                                                else
                                                {
                                                        LinkRecvBit <<= 1;

                                                        if(!LinkRecvBit)
                                                                LinkRecvAgain = 1;
                                                }
                                }                                    
                        }
                                        
                        /*if(LINK == 'P')
                                outp(LinkPort, invert[LinkReg]);
                        if(LINK == 'S')                                
                                outp(LinkPort+4, invert[LinkReg]);                                                           
                        if(LINK == 'K')
                                outp(0x61, (inp(0x61) &~ 0x03) | (LinkReg ? 0x2 : 0x0));*/
		}
}

byte Z80_In(byte Port)
{   
        Port &= ((CALC == 82) || (CALC == 83)) ? 0x17 : 0x27;
	
	switch(Port)
	{
	case 0x00:
                if((CALC == 82) || (CALC == 83))
                        return LinkRecv();
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
	case 0x01:           
		return KeyboardRead();
	case 0x02:
		if((CALC == 82) || (CALC == 83))
			return (Sector1_Page & 7) | 0x88;               
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
	case 0x03:
                xByte = 0;

                /*if(kb_key_h(KB_ON))
					
                        xByte |= ONmask;
                else*/
					xByte |= 8;
            
                if(LCDon)
                        xByte |= LCDmask;

		return xByte;
	case 0x04:
		if(CALC == 86)
			return PowerReg;
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
	case 0x05:
		if(CALC == 85)
			return Sector1_Page;
		if(CALC == 86)
			return Sector1_Page | Sector1_Type;
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
	case 0x06:                                                                    
		if(CALC == 85)
			return PowerReg;
		if(CALC == 86)
			return Sector2_Page | Sector2_Type;
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
        case 0x07:       
                if((CALC == 85) || (CALC == 86))
                        return LinkRecv();
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
	case 0x11:
		if((CALC == 82) || (CALC == 83))
		{                                                   
			if(VidDir == 4)
				++VidY;
			if(VidDir == 5)
				--VidY;
			if(VidDir == 6)
				++VidX;
			if(VidDir == 7)
				--VidX;

			if(VidMode)
				xByte = VidRAM[(12*VidY)+VidX];
			else
			{
				VidCol = VidX * 6;                                                                                                
				VidBit = VidCol & 7;
				VidPtr = &VidRAM[(VidY*12)+(VidCol>>3)];
				xByte = ((((*VidPtr)<<8)+VidPtr[1])>>(10-VidBit));
			}

			if(VidDir == 4)
				VidY-=2;
			if(VidDir == 5)
				VidY+=2;
			if(VidDir == 6)
				VidX-=2;
			if(VidDir == 7)
				VidX+=2;

			VidX &= 15;
			VidY &= 63;

			return xByte;
		}                     
	default:
                return ((CALC == 82) || (CALC == 83)) ? 0x08 : 0x78;
	}
}

void Z80_Out(byte Port,byte Value)
{
        Port &= ((CALC == 82) || (CALC == 83)) ? 0x17 : 0x27;

	switch(Port)
	{
	case 0x00:
                if((CALC == 82) || (CALC == 83))
                        LinkSend(Value);
                else
			VidOffset = Value;
		return;
	case 0x01:
		KeypadMask = Value;
		return;
	case 0x02:
		if(CALC == 82)
			Sector1_Page = Value & 7;
		if(CALC == 83)
			Sector1_Page = (Sector1_Page & 8) | (Value & 7);
                if((CALC == 85) || (CALC == 86))
                        Contrast = Value;
		return;
	case 0x03:                                                                        
		ONmask = Value & 1;
                LCDmask = Value & 2;
                LCDon = Value & 8;
		return;
	case 0x04:
		if(CALC == 86)
			PowerReg = Value & 0xC1;
		return;
	case 0x05:
		if(CALC == 85)             
			Sector1_Page = Value & 7;
		if(CALC == 86)
		{
			Sector1_Type = Value & 0x40;
			Sector1_Page = Value & 15;
		}
		return;
	case 0x06:
		if(CALC == 85)
			PowerReg = Value & 0xC1;
		if(CALC == 86)
		{
			Sector2_Type = Value & 0x40;
			Sector2_Page = Value & 15;
		}
		return;
	case 0x07:
                if((CALC == 85) || (CALC == 86))
                        LinkSend(Value);
		return;
	case 0x10:
		if((CALC == 82) || (CALC == 83))
		{
			if((Value == 0) || (Value == 1))
				VidMode = Value;
			if((Value >= 0x20) && (Value <= 0x30))
				VidX = Value - 0x20;
			if((Value >= 0x80) && (Value <= 0xBF))
				VidY = Value - 0x80;
			if((Value == 4) || (Value == 5) || (Value == 6) || (Value == 7))
				VidDir = Value;
                        if((Value >= 0x40) && (Value <= 0x7F))
                                VidScroll = Value - 0x40;
                        if(Value >= 0xD8)
                                Contrast = Value - 0xD8;
		}
		return;                    
	case 0x11:
		if((CALC == 82) || (CALC == 83))
		{
			if(VidMode)                        
				VidRAM[(12*VidY)+VidX] = Value;
			else
			{
				Value = Value<<2;
				VidCol = VidX * 6;                                                                                                
				VidBit = VidCol & 7;
				VidPtr = &VidRAM[(VidY*12)+(VidCol>>3)];
				*VidPtr = (*VidPtr & ~(0xFC>>VidBit)) | (Value>>VidBit);
				if(VidBit>2)
					VidPtr[1] = (VidPtr[1] & ~(0xFC<<(8-VidBit))) | (Value<<(8-VidBit));
			}

			if(VidDir == 4)
				--VidY;
			if(VidDir == 5)
				++VidY;
			if(VidDir == 6)
				--VidX;
			if(VidDir == 7)
				++VidX;

			VidX &= 15;
			VidY &= 63;
		}                    
	}
}

int Address(dword addr)
{
	int return_addr = (int)addr;

	if((CALC == 82) || (CALC == 83) || (CALC == 85))
	{
		if((return_addr >= 0x4000) && (return_addr < 0x8000))
		{
			Mem_Type = 0;                                                                
			return_addr = return_addr - 0x4000 + (0x4000 * (int)Sector1_Page);
		}

		else if(return_addr < 0x4000)
		{
			Mem_Type = 0;
		}

		else if(return_addr >= 0x8000)
		{
			Mem_Type = 1;
			return_addr -= 0x8000;
		}         
	}

	if(CALC == 86)
	{
		if((return_addr >= 0x4000) && (return_addr < 0x8000))
		{
			Mem_Type = Sector1_Type;
			return_addr = return_addr - 0x4000 + (0x4000 * (int)Sector1_Page);
		}

		else if((return_addr >= 0x8000) && (return_addr < 0xC000))
		{
			Mem_Type = Sector2_Type; 
			return_addr = return_addr - 0x8000 + (0x4000 * (int)Sector2_Page);
		}

		else if(return_addr < 0x4000)
		{
			Mem_Type = 0;
		}

		else if(return_addr >= 0xC000)
		{
			Mem_Type = 1;
			return_addr -= 0xC000;
		}
	}

	return return_addr;
}

unsigned Z80_RDMEM(dword A)
{
	int addr;

	addr = Address(A);

	if(Mem_Type)
		return RAM[addr];
	else
		return ROM[addr];

}

void Z80_WRMEM(dword A,byte V)
{
	int addr;

	addr = Address(A);

	if(Mem_Type)
		RAM[addr] = V;
}

void Z80_Reti(void)
{
}

void Z80_Retn(void)
{
}

void Z80_Patch(Z80_Regs* Regs)
{
}

int Z80_Interrupt(void)
{

  key_stuck = 0;
  //DrawScreen();

  return 0xFF;
}


void LoadMEM()
{                      
        if(CALC == 82)
        {
                ROMSIZE = 0x20000;
                RAMSIZE = 0x7FFF;
                SAVSIZE = 0x832B;
                BMPSIZE = 0x340;
                VIDJUMP = 7;
                LCDWIDTH = 3;                
        }
        if(CALC == 83)
        {
                ROMSIZE = 0x40000;
                RAMSIZE = 0x7FFF;
                SAVSIZE = 0x832C;
                BMPSIZE = 0x340;
                VIDJUMP = 7;
                LCDWIDTH = 3;
        }
        if(CALC == 85)
        {
                ROMSIZE = 0x20000;
                RAMSIZE = 0x7FFF;
                SAVSIZE = 0x8028;
                BMPSIZE = 0x440;
                VIDJUMP = 6;
                LCDWIDTH = 4;
        }
        if(CALC == 86)
        {
                ROMSIZE = 0x40000;
                RAMSIZE = 0x1FFFF;
                SAVSIZE = 0x20029;
                BMPSIZE = 0x440;
                VIDJUMP = 6;
                LCDWIDTH = 4;
        }

        LoadROM();
	      LoadRAM();
}

void LoadROM(void)
{
	ROM = (unsigned char *)0x00080000;
}

void LoadRAM(void)
{        		                           
        memset(VidRAM,0xFF,768);
        memset(RAM,0xFF,SAVSIZE);
        InitRAM = 1;

        if(InitRAM)
	{          
		for(xInt = 0; xInt <= 0x4000; ++xInt)           
		{
			if(CALC == 86)
			{
				if((ROM[xInt] == 0x3E) && (ROM[xInt+1] == 0x56) &&
					(ROM[xInt+2] == 0xD3) && (ROM[xInt+3] == 0x04))
					{
						R.PC.W.l = xInt;
						break;
					}
			}
			else
			{
				if((ROM[xInt] == 0x3E) && (ROM[xInt+1] == 0x16) &&
					(ROM[xInt+2] == 0xD3) && (ROM[xInt+3] == 0x04))
					{
						R.PC.W.l = xInt;
						break;
					}
			}
		}
		return;
	}

        xInt = RAMSIZE;

	R.AF.B.h = RAM[++xInt];
	R.AF.B.l = RAM[++xInt];
	R.BC.B.h = RAM[++xInt];
	R.BC.B.l = RAM[++xInt];
	R.DE.B.h = RAM[++xInt];
	R.DE.B.l = RAM[++xInt];
	R.HL.B.h = RAM[++xInt];
	R.HL.B.l = RAM[++xInt];
	R.IX.B.h = RAM[++xInt];
	R.IX.B.l = RAM[++xInt];
	R.IY.B.h = RAM[++xInt];
	R.IY.B.l = RAM[++xInt];
	R.PC.B.h = RAM[++xInt];
	R.PC.B.l = RAM[++xInt];
	R.SP.B.h = RAM[++xInt];
	R.SP.B.l = RAM[++xInt];
	R.AF2.B.h = RAM[++xInt];
	R.AF2.B.l = RAM[++xInt];
	R.BC2.B.h = RAM[++xInt];
	R.BC2.B.l = RAM[++xInt];
	R.DE2.B.h = RAM[++xInt];
	R.DE2.B.l = RAM[++xInt];
	R.HL2.B.h = RAM[++xInt];
	R.HL2.B.l = RAM[++xInt];
	R.IFF1 = RAM[++xInt];
	R.IFF2 = RAM[++xInt];
	R.HALT = RAM[++xInt];
	R.IM = RAM[++xInt];
	R.I = RAM[++xInt];
	R.R = RAM[++xInt];
	R.R2 = RAM[++xInt];

	KeypadMask = RAM[++xInt];
	ONmask = RAM[++xInt];
        LinkReg = RAM[++xInt];
        LCDmask = RAM[++xInt];
        LCDon = RAM[++xInt];
        Contrast = RAM[++xInt];

        if(CALC == 83)
                Z80_Out(0,RAM[++xInt]);

	if((CALC == 82) || (CALC == 83))
	{
		Z80_Out(2,RAM[++xInt]);
		VidX = RAM[++xInt];
		VidY = RAM[++xInt];
		VidDir = RAM[++xInt];
		VidMode = RAM[++xInt];
                VidScroll = RAM[++xInt];
		memcpy(VidRAM,RAM + (++xInt),768);
	}

        if((CALC == 85) || (CALC == 86))
                Z80_Out(5,RAM[++xInt]);
        if(CALC == 86)
                Z80_Out(6,RAM[++xInt]);

	if((CALC == 85) || (CALC == 86))
	{
		VidOffset = RAM[++xInt];                      
		PowerReg = RAM[++xInt];            
	}                        
}

void SaveRAM(void)
{
        InitRAM = 0;

        xInt = RAMSIZE;

	RAM[++xInt] = R.AF.B.h;
	RAM[++xInt] = R.AF.B.l;
	RAM[++xInt] = R.BC.B.h;
	RAM[++xInt] = R.BC.B.l;
	RAM[++xInt] = R.DE.B.h;
	RAM[++xInt] = R.DE.B.l;
	RAM[++xInt] = R.HL.B.h;
	RAM[++xInt] = R.HL.B.l;
	RAM[++xInt] = R.IX.B.h;
	RAM[++xInt] = R.IX.B.l;
	RAM[++xInt] = R.IY.B.h;
	RAM[++xInt] = R.IY.B.l;
	RAM[++xInt] = R.PC.B.h;
	RAM[++xInt] = R.PC.B.l;
	RAM[++xInt] = R.SP.B.h;
	RAM[++xInt] = R.SP.B.l;
	RAM[++xInt] = R.AF2.B.h;
	RAM[++xInt] = R.AF2.B.l;
	RAM[++xInt] = R.BC2.B.h;
	RAM[++xInt] = R.BC2.B.l;
	RAM[++xInt] = R.DE2.B.h;
	RAM[++xInt] = R.DE2.B.l;
	RAM[++xInt] = R.HL2.B.h;
	RAM[++xInt] = R.HL2.B.l;
	RAM[++xInt] = R.IFF1;
	RAM[++xInt] = R.IFF2;
	RAM[++xInt] = R.HALT;
	RAM[++xInt] = R.IM;
	RAM[++xInt] = R.I;
	RAM[++xInt] = R.R;
	RAM[++xInt] = R.R2;

	RAM[++xInt] = KeypadMask;
	RAM[++xInt] = ONmask;
        RAM[++xInt] = LinkReg;
        RAM[++xInt] = LCDmask;
        RAM[++xInt] = LCDon;
        RAM[++xInt] = Contrast;

        if(CALC == 83)
                RAM[++xInt] = Z80_In(0);

	if((CALC == 82) || (CALC == 83))
	{
		RAM[++xInt] = Z80_In(2);             
		RAM[++xInt] = VidX;
		RAM[++xInt] = VidY;
		RAM[++xInt] = VidDir;
		RAM[++xInt] = VidMode;
                RAM[++xInt] = VidScroll;
		memcpy(RAM + (++xInt),VidRAM,768);
	}
	if((CALC == 85) || (CALC == 86))
		RAM[++xInt] = Z80_In(5);
	if(CALC == 86)     
		RAM[++xInt] = Z80_In(6);

        if((CALC == 85) || (CALC == 86))
        {
                RAM[++xInt] = VidOffset;
                RAM[++xInt] = PowerReg;
        }

      //SAVE RAM HERE
}

void SetCalc(byte c)
{
        CALC = c;
}

void SetLink(char l,char p)
{
        LINK = toupper(l);

        if(((p < '1') || (p > '9')) && (LINK != 'K'))
        {
                printf("\aInvalid paramaters!\n");
        }
                
        switch(LINK)
        {
                case 'G':
                        //´ò¿ª´®¿Ú
                        break;        
                case 'P':
                        LinkPort = 0x378 - ((p-'1') << 8);
                        break;
                case 'S':
                        LinkPort = 0x3F8-(((p-'1')&0x1)<<8)-(((p-'1')&0x2)<<3);
                        break;
                case 'K':
                        break;
                default:
                        printf("\aInvalid paramaters!\n");
												break;
        }                        
}

unsigned char ReverseBits(unsigned char num)
{
    num = ((num&0x0f)<<4) | ((num&0xf0)>>4);
    num = ((num&0x33)<<2) | ((num&0xcc)>>2);
    num = ((num&0x55)<<1) | ((num&0xaa)>>1);
    return num;
}

void DrawScreen(void)
{                        
        byte x;
        byte y;
	unsigned char i,j;
	signed char k;
	unsigned char *ptr;
				unsigned char *vidptr;

	if((CALC == 82) || (CALC == 83))
	{
                memcpy(VidRAM+0x300,VidRAM,0x300);                        
                vidptr=VidRAM + (12 * VidScroll);
		//UPDATE LCD HERE
		for (i=0;i<64;i++)
		{
			ptr = (unsigned char *)(0x0803a000+i*20);
		  for (j=0;j<3;j++)
			{
				for (k=3;k>=0;k--)
				{
				  *(ptr+k) = ReverseBits(*vidptr);
				  vidptr++;
				}
				ptr+=4;
			}
		}   
	}

	if((CALC == 85) || (CALC == 86))
	{
		vidptr=RAM + ((VidOffset & 0x3F) * 0x100);
		if(CALC == 85)
			vidptr += 0x4000;
		for (i=0;i<64;i++)
		{
			ptr = (unsigned char *)(0x0803a000+i*20);
		  for (j=0;j<4;j++)
			{
				for (k=3;k>=0;k--)
				{
				  *(ptr+k) = ReverseBits(*vidptr);
				  vidptr++;
				}
				ptr+=4;
			}
		}   
		//memcpy(((unsigned char *)0x0803a000),vidptr,1024);
		//UPDATE LCD HERE
	}
}
