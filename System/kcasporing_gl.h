#pragma once 


#define VIR_LCD_PIX_W   256
#define VIR_LCD_PIX_H   127



#ifdef __cplusplus
extern "C" {
#endif
  extern int khicas_1bpp;

int vGL_Initialize() ;

void vGL_putString(int x0, int y0, char *s, int fg, int bg, int fontSize);
void vGL_putChar(int x0, int y0, char ch, int fg, int bg, int fontSize);
void vGL_reverseArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) ;
void vGL_clearArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1) ;
void vGL_setArea(unsigned int x0, unsigned int y0, unsigned int x1, unsigned int y1, unsigned int color) ;
void vGL_FlushVScreen();
int vGL_GetPoint(unsigned int x, unsigned int y);
void vGL_SetPoint(unsigned int x, unsigned int y, int c);

void vGL_ConsLocate(int x, int y);
void vGL_ConsOut(char *s, bool rev);
void vGL_concurEnable(bool enable);
void vGL_locateConcur(int x, int y);

bool vGL_getkey(int *keyid);
bool vGL_chkEsc();

#ifdef __cplusplus
}
#endif


