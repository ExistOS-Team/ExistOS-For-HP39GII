#ifndef _LIBTEXT_H
#define _LIBTEXT_H

#define TXT_FONTS 11

void Txt_Init   (int);
void Txt_Pixel  (int,int,int);
void Txt_Point  (int,int,int,int);
int  Txt_Get    (int,int);
void Txt_Char   (char,int,int,int,int,int,int);
void Txt_Text   (char *,int,int,int,int);
int  Txt_Width  (int);
int  Txt_Height (int);
void Txt_Quit   (void);

#define TXT_OR  0x01
#define TXT_ON  0x02
#define TXT_AND 0x04
#define TXT_REV 0x08
#define TXT_XOR 0x10

#define TXT_MINISD           0x00
#define TXT_7SEGMINI         0x01
#define TXT_SYSTEM           0x02
#define TXT_7SEG             0x03
#define TXT_ARCADIUM         0x04
#define TXT_SERIF            0x05
#define TXT_SERIFITALIC      0x06
#define TXT_SERIFBOLD        0x07
#define TXT_SERIFBOLDITALIC  0x08
#define TXT_RUNES            0x09
#define TXT_DOS              0x0A

#define FONT_MINISD          0x00000001
#define FONT_7SEGMINI        0x00000002
#define FONT_SYSTEM          0x00000004
#define FONT_7SEG            0x00000008
#define FONT_ARCADIUM        0x00000010
#define FONT_SERIF           0x00000020
#define FONT_SERIFITALIC     0x00000040
#define FONT_SERIFBOLD       0x00000080
#define FONT_SERIFBOLDITALIC 0x00000100
#define FONT_RUNES           0x00000200
#define FONT_DOS             0x00000400
#define FONT_ALL             0xFFFFFFFF

#endif // _LIBTEXT_H
