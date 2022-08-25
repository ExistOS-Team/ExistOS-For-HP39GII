
#include "pch.h"
#include "emu48.h"
#include "io.h"
#include "kml.h"

#include "FreeRTOS.h"
#include "task.h"

#include "sys_llapi.h"
#include "SystemUI.h"

void emu48_update_display();

//#define DEBUG_DISPLAY

#define LCD_ROW		(36*4)					// max. pixel per line

#define NOCOLORSGRAY	8					// no. of colors in gray scale mode
#define NOCOLORSBW		2					// no. of colors in black and white mode

#define DISPLAY_FREQ	19					// display update 1/frequency (1/64) in ms (gray scale mode)

static DWORD dwGrayMask;


//#define DIBPIXEL(d,p) *((DWORD*)(d)) = ((*((DWORD*)(d)) & dwGrayMask ) << 1) | (p); d += 1 //*(((DWORD*)(d))++) = ((*((DWORD*)(d)) & dwGrayMask) << 1) | (p)
#define DIBPIXEL(d,p) *d = (p); d += 1 //*(((DWORD*)(d))++) = ((*((DWORD*)(d)) & dwGrayMask) << 1) | (p)

#define GRAYMASK(c)	(((((c)-1)>>1)<<24) \
					|((((c)-1)>>1)<<16) \
					|((((c)-1)>>1)<<8)  \
					|((((c)-1)>>1)))

BOOL   bGrayscale = FALSE;

LPBYTE pbyLcd;

static LARGE_INTEGER lLcdRef;				// reference time for VBL counter
static UINT uLcdTimerId = 0;

static DWORD Pattern[16];
static BYTE  Buf[36];


static BYTE byVblRef = 0;					// VBL stop reference

BYTE (*GetLineCounter)(VOID) = NULL;
VOID (*StartDisplay)(BYTE byInitial) = NULL;
VOID (*StopDisplay)(VOID) = NULL;

#define OutputDebugString(str) printf("%s\n",str)

/*
void DIBPIXEL(BYTE *d,DWORD p)
{

}
*/
VOID UpdateContrast(BYTE byContrast)
{
	RGBQUAD c,b;
	INT i,nColors;

	// table for max. 8 colors
	const INT nCAdj[] = { 0, 1, 1, 2, 1, 2, 2, 3 };

	// when display is off use contrast 0
	if ((Chipset.IORam[BITOFFSET] & DON) == 0) byContrast = 0;

	return;
}


VOID UpdateDisplayPointers(VOID)
{
	EnterCriticalSection(&csLcdLock);
	{
		#if defined DEBUG_DISPLAY
		{
			TCHAR buffer[256];
			wsprintf(buffer,_T("%.5lx: Update Display Pointer\n"),Chipset.pc);
			OutputDebugString(buffer);
		}
		#endif

		// calculate display width
		Chipset.width = (34 + Chipset.loffset + (Chipset.boffset / 4) * 2) & 0xFFFFFFFE;
		Chipset.end1 = Chipset.start1 + MAINSCREENHEIGHT * Chipset.width;
		if (Chipset.end1 < Chipset.start1)
		{
			// calculate first address of main display
			Chipset.start12 = Chipset.end1 - Chipset.width;
			// calculate last address of main display
			Chipset.end1 = Chipset.start1 - Chipset.width;
		}
		else
		{
			Chipset.start12 = Chipset.start1;
		}
		Chipset.end2 = Chipset.start2 + MENUHEIGHT * 34;
	}
	LeaveCriticalSection(&csLcdLock);
	return;
}


VOID UpdateMainDisplay(VOID)
{
	UINT  x, y;
	DWORD d = Chipset.start1;
	BYTE *p = pbyLcd+(Chipset.d0size*LCD_ROW);  // CdB for HP: add 64/80 ligne display for apples

	#if defined DEBUG_DISPLAY
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("%.5lx: Update Main Display\n"),Chipset.pc);
		OutputDebugString(buffer);
	}
	#endif

	if (!(Chipset.IORam[BITOFFSET]&DON))
	{
		ZeroMemory(pbyLcd, LCD_ROW * SCREENHEIGHT);
	}
	else
	{
		for (y = 0; y < MAINSCREENHEIGHT; ++y)
		{
			Npeek(Buf,d,36);
			for (x=0; x<36; ++x)		// every 4 pixel
			{
				DIBPIXEL(p,Pattern[Buf[x]]);

				// check for display buffer overflow
				_ASSERT(p <= pbyLcd + LCD_ROW * SCREENHEIGHT);
			}
			d+=Chipset.width;
		}
	}
	EnterCriticalSection(&csGDILock);		// solving NT GDI problems
	{
		//emu48_update_display();

		 // CdB for HP: add 64/80 ligne display for apples
         /*
       StretchBlt(hWindowDC, 
		      nLcdX, nLcdY+Chipset.d0size*nLcdZoom, 
			  131*nLcdZoom, MAINSCREENHEIGHT*nLcdZoom,
	          hLcdDC, 
			  Chipset.boffset, Chipset.d0size, 
			  131, MAINSCREENHEIGHT,
			  SRCCOPY);
		GdiFlush();*/

	}
	LeaveCriticalSection(&csGDILock);
	return;
}


VOID UpdateMenuDisplay(VOID)
{
	UINT x, y;
	BYTE *p;
	DWORD d = Chipset.start2;

	#if defined DEBUG_DISPLAY
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("%.5lx: Update Menu Display\n"),Chipset.pc);
		OutputDebugString(buffer);
	}
	#endif

	if (!(Chipset.IORam[BITOFFSET]&DON)) return;
	if (MENUHEIGHT==0) return;				// menu disabled

	// calculate bitmap offset
	p = pbyLcd + ((Chipset.d0size+MAINSCREENHEIGHT)*LCD_ROW);  // CdB for HP: add 64/80 ligne display for apples
	for (y = 0; y < MENUHEIGHT; ++y)
	{
		Npeek(Buf,d,34);				// 34 nibbles are viewed
		for (x=0; x<34; ++x)			// every 4 pixel
		{
			DIBPIXEL(p,Pattern[Buf[x]]);
			// check for display buffer overflow
			_ASSERT(p <= pbyLcd + LCD_ROW * SCREENHEIGHT);
		}
		// adjust pointer to 36 DIBPIXEL drawing calls
		p += (36-34) * sizeof(DWORD);
		d+=34;
	}
	EnterCriticalSection(&csGDILock);		// solving NT GDI problems
	{
		//emu48_update_display();
		 // CdB for HP: add 64/80 ligne display for apples
         /*
        StretchBlt(hWindowDC, 
			   nLcdX, nLcdY+(MAINSCREENHEIGHT+Chipset.d0size)*nLcdZoom,
	           131*nLcdZoom, MENUHEIGHT*nLcdZoom,
	           hLcdDC, 
			   0, (MAINSCREENHEIGHT+Chipset.d0size), 
			   131, MENUHEIGHT, 
			   SRCCOPY);
		GdiFlush();*/
	}
	LeaveCriticalSection(&csGDILock);
	return;
}


VOID UpdateAnnunciators(VOID)
{
	BYTE c;

	c = (BYTE)(Chipset.IORam[ANNCTRL] | (Chipset.IORam[ANNCTRL+1]<<4));
	// switch annunciators off if timer stopped
	if ((c & AON) == 0 || (Chipset.IORam[TIMER2_CTRL] & RUN) == 0)
		c = 0;

	int intBit = 0;
    intBit |= c&LA1 ? INDICATE_LEFT : 0;
    intBit |= c&LA2 ? INDICATE_RIGHT : 0;
    intBit |= c&LA3 ? INDICATE_A__Z : 0;
    intBit |= c&LA4 ? INDICATE_a__z : 0;
    intBit |= c&LA5 ? INDICATE_BUSY : 0;
    intBit |= c&LA6 ? INDICATE_RX : 0;
    intBit |= c&LA6 ? INDICATE_TX : 0;


	ll_disp_set_indicator(intBit,-1);

/*
	DrawAnnunciator(1,c&LA1);
	DrawAnnunciator(2,c&LA2);
	DrawAnnunciator(3,c&LA3);
	DrawAnnunciator(4,c&LA4);
	DrawAnnunciator(5,c&LA5);
	DrawAnnunciator(6,c&LA6);*/

	return;
}

/*
static uint8_t MainDsp_buffer[256*127];
#define bufPoint(x,y,c) if(x < 256)MainDsp_buffer[(y) * 256 + (x)] = (c)
#define nextp()	 {x+=1;if((x >= lWidth * 4) && s ){x = 0;y++;}}*/

//{x+=4;if(x >= lWidth ){x = 0;y++;}}

VOID WriteToMainDisplay(LPBYTE a, DWORD d, UINT s)
{
/*
	UINT x0, x;
	UINT y0, y;
	INT  lWidth = abs(Chipset.width);

	d -= Chipset.start1;					// nibble offset to DISPADDR (start of display)
	y0 = y = (d / lWidth) + Chipset.d0size;		// bitmap row
	x0 = x = d % lWidth;					// bitmap coloumn

	//printf("disp:%d, %d, %d, lwidth:%d\n", x0, y0, s, lWidth);

	uint8_t *p = (uint8_t *)a;
	uint8_t bitDat;
	uint32_t bitcnt = 0;

	x*=4;
	while(s--)
	{
		bitDat = *a++;

		bufPoint(x    ,y, ((bitDat >> 0) & 1) ? 0: 255);nextp();
		bufPoint(x    ,y, ((bitDat >> 1) & 1) ? 0: 255);nextp();
		bufPoint(x    ,y, ((bitDat >> 2) & 1) ? 0: 255);nextp();
		bufPoint(x    ,y, ((bitDat >> 3) & 1) ? 0: 255);nextp();
	}

	ll_disp_put_area(MainDsp_buffer, 0, 1, 255, 126);
	*/

	#if 1
	UINT x0, x;
	UINT y0, y;
	DWORD *p;

	INT  lWidth = abs(Chipset.width);		// display width

	if (bGrayscale)
	{
		return;
	}

	#if defined DEBUG_DISPLAY
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("%.5lx: Write Main Display %x,%u\n"),Chipset.pc,d,s);
		OutputDebugString(buffer);
	}
	#endif

	if (!(Chipset.IORam[BITOFFSET]&DON)) return;	// display off
	if (MAINSCREENHEIGHT == 0) return;				// menu disabled

	d -= Chipset.start1;					// nibble offset to DISPADDR (start of display)
	y0 = y = (d / lWidth) + Chipset.d0size;		// bitmap row
	x0 = x = d % lWidth;					// bitmap coloumn
	p = (DWORD*)(pbyLcd + y0*LCD_ROW + x0*sizeof(*p));

	// outside main display area
//	_ASSERT(y0 >= (INT)Chipset.d0size && y0 < (INT)(MAINSCREENHEIGHT+Chipset.d0size));
	if (!(y0 >= (INT)Chipset.d0size && y0 < (INT)(MAINSCREENHEIGHT+Chipset.d0size))) return;

	while (s--)								// loop for nibbles to write
	{
		if (x<36)							// only fill visible area
		{
			
			*p = Pattern[*a];
		}
		a++;								// next value to write
		x++;								// next x position
		if (((INT) x==lWidth)&&s)			// end of display line
		{
			x = 0;							// first coloumn
			y++;							// next row
			if (y == (INT) MAINSCREENHEIGHT+Chipset.d0size) break;
			// recalculate bitmap memory position of new line
			p = (DWORD*) (pbyLcd+y*LCD_ROW);  // CdB for HP: add 64/80 ligne display for apples
		} else p++;
	}
	if (y==y0) y++;

	EnterCriticalSection(&csGDILock);	// solving NT GDI problems
	{
		emu48_update_display();
        /*
		StretchBlt(hWindowDC, 
			   nLcdX, nLcdY+y0*nLcdZoom, 
			   131*nLcdZoom, (y-y0)*nLcdZoom,
			   hLcdDC, 
			   Chipset.boffset, y0,
			   131, y-y0,
			   SRCCOPY);  // CdB for HP: add 64/80 ligne display for apples
		GdiFlush();
        */
	}
	LeaveCriticalSection(&csGDILock);
	return;
	#endif
}


VOID WriteToMenuDisplay(LPBYTE a, DWORD d, UINT s)
{
	UINT x0, x;
	UINT y0, y;
	DWORD *p;

	if (bGrayscale)
	{
		return;
	}

	#if defined DEBUG_DISPLAY
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("%.5lx: Write Menu Display %x,%u\n"),Chipset.pc,d,s);
		OutputDebugString(buffer);
	}
	#endif

	if (!(Chipset.IORam[BITOFFSET]&DON)) return;	// display off
	if (MENUHEIGHT == 0) return;				// menu disabled

	d -= Chipset.start2;					// nibble offset to DISPADDR (start of display)
	y0 = y = (d / 34) + MAINSCREENHEIGHT+Chipset.d0size;         	// bitmap row
	x0 = x = d % 34;                                        	// bitmap coloumn
	p = (DWORD*)(pbyLcd + y0*LCD_ROW + x0*sizeof(*p));

	// outside menu display area
//	_ASSERT(y0 >= (INT)(Chipset.d0size+MAINSCREENHEIGHT) && y0 < (INT)(SCREENHEIGHT));
	if (!(y0 >= (UINT)(Chipset.d0size+MAINSCREENHEIGHT) && y0 < (UINT)(SCREENHEIGHT))) return;

	while (s--)								// loop for nibbles to write
	{
		if (x<36)							// only fill visible area
		{
			*p = Pattern[*a];
		}
		a++;								// next value to write
		x++;								// next x position
		if ((x==34)&&s)					// end of display line
		{
			x = 0;							// first coloumn
			y++;							// next row
			if (y == SCREENHEIGHTREAL) break;
			// recalculate bitmap memory position of new line
			p=(DWORD*)(pbyLcd+y*LCD_ROW);  // CdB for HP: add 64/80 ligne display for apples
		} else p++;
	}
	if (y==y0) y++;
	EnterCriticalSection(&csGDILock);	// solving NT GDI problems
	{
		emu48_update_display();
        /*
		StretchBlt(hWindowDC, 
			   nLcdX, nLcdY+y0*nLcdZoom, 
			   131*nLcdZoom, (y-y0)*nLcdZoom,
			   hLcdDC, 
			   0, y0,
			   131, y-y0,
			   SRCCOPY);  // CdB for HP: add 64/80 ligne display for apples
		GdiFlush();*/
	}
	LeaveCriticalSection(&csGDILock);
	return;
}


// CdB for HP: add header management
VOID RefreshDisp0()
{
	UINT x, y;
	BYTE *p;
	BYTE* d = Chipset.d0memory;

	#if defined DEBUG_DISPLAY
	{
		TCHAR buffer[256];
		wsprintf(buffer,_T("%.5lx: Update header Display\n"),Chipset.pc);
		OutputDebugString(buffer);
	}
	#endif

	if (!(Chipset.IORam[BITOFFSET]&DON)) return;

	// calculate bitmap offset
	p = pbyLcd;
	for (y = 0; y<Chipset.d0size; ++y)
	{
		memcpy(Buf,d,34);				// 34 nibbles are viewed
		for (x=0; x<36; ++x)			// every 4 pixel
		{
			DIBPIXEL(p,Pattern[Buf[x]]);
		}
		d+=34;
	}
	EnterCriticalSection(&csGDILock);		// solving NT GDI problems
	{
		printf("update Disp0\n");
		emu48_update_display();
		//emu48_update_display();
        /*
		StretchBlt(hWindowDC, 
			       nLcdX, nLcdY,
			       131*nLcdZoom, Chipset.d0size*nLcdZoom,
			       hLcdDC, 
				   Chipset.d0offset, 0, 
				   131, Chipset.d0size,
				   SRCCOPY);
		GdiFlush();*/
	}
	LeaveCriticalSection(&csGDILock);
	return;
}

//################
//#
//# functions for gray scale implementation
//#
//################
#if 0
// main display update routine
static VOID CALLBACK LcdProc(UINT uEventId, UINT uMsg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	EnterCriticalSection(&csLcdLock);
	{
		UpdateMainDisplay();				// update display
		UpdateMenuDisplay();
		RefreshDisp0();  // CdB for HP: add header management
	}
	LeaveCriticalSection(&csLcdLock);

	QueryPerformanceCounter(&lLcdRef);		// actual time

	return;
	UNREFERENCED_PARAMETER(uEventId);
	UNREFERENCED_PARAMETER(uMsg);
	UNREFERENCED_PARAMETER(dwUser);
	UNREFERENCED_PARAMETER(dw1);
	UNREFERENCED_PARAMETER(dw2);
}

// LCD line counter calculation
BYTE GetLineCounterGray(VOID)
{
	LARGE_INTEGER lLC;
	BYTE          byTime;

	if (uLcdTimerId == 0)					// display off
		return ((Chipset.IORam[LINECOUNT+1] & (LC5|LC4)) << 4) | Chipset.IORam[LINECOUNT];

	QueryPerformanceCounter(&lLC);			// get elapsed time since display update

	// elapsed ticks so far
	byTime = (BYTE) (((lLC.QuadPart - lLcdRef.QuadPart) << 12) / lFreq.QuadPart);

	if (byTime > 0x3F) byTime = 0x3F;		// all counts made

	return 0x3F - byTime;					// update display between VBL counter 0x3F-0x3E
}

static VOID StartDisplayGray(BYTE byInitial)
{
	if (uLcdTimerId)						// LCD update timer running
		return;								// -> quit

	if (Chipset.IORam[BITOFFSET]&DON)		// display on?
	{
		QueryPerformanceCounter(&lLcdRef);	// actual time of top line

		// adjust startup counter to get the right VBL value
		_ASSERT(byInitial <= 0x3F);			// line counter value 0 - 63
		lLcdRef.QuadPart -= ((LONGLONG) (0x3F - byInitial) * lFreq.QuadPart) >> 12;

		VERIFY(uLcdTimerId = timeSetEvent(DISPLAY_FREQ,0,(LPTIMECALLBACK)&LcdProc,0,TIME_PERIODIC));
	}
	return;
}

static VOID StopDisplayGray(VOID)
{
	BYTE a[2];
	ReadIO(a,LINECOUNT,2,TRUE);				// update VBL at display off time

	if (uLcdTimerId == 0)					// timer stopped
		return;								// -> quit

	timeKillEvent(uLcdTimerId);				// stop display update
	uLcdTimerId = 0;						// set flag display update stopped

	EnterCriticalSection(&csLcdLock);		// update to last condition
	{
		UpdateMainDisplay();				// update display
		UpdateMenuDisplay();
		RefreshDisp0();  // CdB for HP: add header management
	}
	LeaveCriticalSection(&csLcdLock);
	return;
}
#endif
//################
//#
//# functions for black and white implementation
//#
//################

// LCD line counter calculation in BW mode
static BYTE F4096Hz(VOID)					// get a 6 bit 4096Hz down counter value
{
	LARGE_INTEGER lLC;

	QueryPerformanceCounter(&lLC);			// get counter value

	// calculate 4096 Hz frequency down counter value
	return -(BYTE)(((lLC.QuadPart - lAppStart.QuadPart) << 12) / lFreq.QuadPart) & 0x3F;
}

static BYTE GetLineCounterBW(VOID)			// get line counter value
{
	_ASSERT(byVblRef < 0x40);
	return (0x40 + F4096Hz() - byVblRef) & 0x3F;
}

static VOID StartDisplayBW(BYTE byInitial)
{
	// get positive VBL difference between now and stop time
	byVblRef = (0x40 + F4096Hz() - byInitial) & 0x3F;
	return;
}

static VOID StopDisplayBW(VOID)
{
	BYTE a[2];
	ReadIO(a,LINECOUNT,2,TRUE);				// update VBL at display off time
	return;
}

static VOID BuildPattern(VOID)
	{

		WORD i,j;
		for (i=0; i<16; ++i)
		{
			Pattern[i] = 0;
			for (j=8; j>0; j>>=1)
			{
				Pattern[i] = (Pattern[i] << 8) | (((i&j) != 0) ? 0 : 255 );
			}
		}
		return;
}

VOID SetLcdMode(BOOL bMode)
{

	BuildPattern();
/*
	if ((bGrayscale = bMode))
	{
		// set pixel update mask
		dwGrayMask = GRAYMASK(NOCOLORSGRAY);
		GetLineCounter = GetLineCounterGray;
		StartDisplay = StartDisplayGray;
		StopDisplay = StopDisplayGray;
	}
	else*/
	{
		// set pixel update mask
		dwGrayMask = GRAYMASK(NOCOLORSBW);
		GetLineCounter = GetLineCounterBW;
		StartDisplay = StartDisplayBW;
		StopDisplay = StopDisplayBW;
	}
	UpdateContrast(Chipset.contrast);
	return;
}
