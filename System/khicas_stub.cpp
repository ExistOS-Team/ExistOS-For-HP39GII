
#include "sys_llapi.h"

#include <stdio.h>
#include <string.h>

#include "SysConf.h"

#if FS_TYPE == FS_FATFS
    #include "Fatfs/ff.h"
#else
    #include "lfs.h"
#endif



#include "porting.h"

#include "menuGUI.h"

#include "kcasporing_gl.h"

#include "keyboard_gii39.h"

#include "SystemUI.h"
#include "SystemFs.h"



/*
int doMenu(Menu* menu, MenuItemIcon* icontable)
{
   (menu);
   (icontable);

   return 0;
}

int showCatalog(char* insertText,int preselect,int menupos)
{

   return 0;
}
*/

extern "C" {
bool khicasRunning = false;

char keyStatus = 0;
int intBit = 0;
int rshift = 0;

void (*XcasExitCb)(void) = NULL;

/*
void dConsolePut(const char * S){
  //if (!dconsole_mode)
  //  return;
  int l=strlen(S);
  char s[l+1];
  strcpy(s,S);
  for (int i=0;i<l-1;++i){
    if (s[i]=='\n' ||
        s[i]==10)
      s[i]=' ';
  }
  printf("%s", s);
  if (l && S[l-1]=='\n')
    printf("\n");
  //Console_Output((const unsigned char *)s);
  //if (l && S[l-1]=='\n')
  //  Console_NewLine(LINE_TYPE_OUTPUT, 1);
}

void dConsoleRedraw()
{


}


void dConsolePutChar(const char ch)
{

} 
*/

char Setup_GetEntry(unsigned int index) {
    // 0x14
    //  = 1 shift
    //  = 4 alpha 1
    //  = 8 alpha 2
    // printf("Setup_GetEntry:%d\n", index);

    if (index == 0x14) {

        return keyStatus;
    }

    printf("Setup_GetEntry:%d\n", index);

    return 0;
}

void flush_indBit()
{
        intBit = 0;
        intBit |= (keyStatus & 1) ? INDICATE_LEFT : 0;
        intBit |= (keyStatus & 4) ? INDICATE_A__Z : 0;
        intBit |= (keyStatus & 8) ? INDICATE_a__z : 0;
        intBit |= rshift ? INDICATE_RIGHT : 0;
        ll_disp_set_indicator(intBit, -1);
}

char *Setup_SetEntry(unsigned int index, char setting) {

    if (index == 0x14) {
        keyStatus = setting;
        flush_indBit();

        return NULL;
    }

    // printf("Setup_SetEntry:%d, %d\n", index, setting);
    return NULL;
}

#define INPUT_TRANSLATE(_rawin, normal, shift_l, shift_r, alpha_capital, alpha_small) \
    case _rawin: {                                                                    \
        if (keyStatus & 1) {                                                          \
            *key = shift_l;                                                           \
            keyStatus = 0;                                                            \
            flush_indBit();                                                           \
        } else if (rshift) {                                                          \
            *key = shift_r;                                                           \
        } else if (keyStatus & 4) {                                                   \
            *key = alpha_small;                                                       \
        } else if (keyStatus & 8) {                                                   \
            *key = alpha_capital;                                                     \
        } else {                                                                      \
            *key = normal;                                                            \
        }                                                                             \
        break;                                                                        \
    }

bool IsKeyDown(int test_key)
{
    uint32_t tr_key;
    uint32_t keys, key_get, kpress, *key = &tr_key;
    keys = ll_vm_check_key();
    key_get = keys & 0xFFFF;
    kpress = keys >> 16;
    //printf("IsKeyDown:%d\n",test_key);
    if(kpress){
    switch (key_get)
    {

        INPUT_TRANSLATE(KEY_F1, KEY_CTRL_F1, KEY_CTRL_F1, KEY_CTRL_F1, KEY_CTRL_F1, KEY_CTRL_F1);
        INPUT_TRANSLATE(KEY_F2, KEY_CTRL_F2, KEY_CTRL_F2, KEY_CTRL_F2, KEY_CTRL_F2, KEY_CTRL_F2);
        INPUT_TRANSLATE(KEY_F3, KEY_CTRL_F3, KEY_CTRL_F3, KEY_CTRL_F3, KEY_CTRL_F3, KEY_CTRL_F3);
        INPUT_TRANSLATE(KEY_F4, KEY_CTRL_F4, KEY_CTRL_F4, KEY_CTRL_F4, KEY_CTRL_F4, KEY_CTRL_F4);
        INPUT_TRANSLATE(KEY_F5, KEY_CTRL_F5, KEY_CTRL_F5, KEY_CTRL_F5, KEY_CTRL_F5, KEY_CTRL_F5);
        INPUT_TRANSLATE(KEY_F6, KEY_CTRL_F6, KEY_CTRL_F6, KEY_CTRL_F6, KEY_CTRL_F6, KEY_CTRL_F6);
        
        //INPUT_TRANSLATE(_rawin, normal, shift_l, shift_r, alpha_small, alpha_capital)
        INPUT_TRANSLATE(KEY_UP, KEY_CTRL_UP, KEY_CTRL_PAGEUP, KEY_CTRL_UP, KEY_CTRL_UP, KEY_CTRL_UP);
        INPUT_TRANSLATE(KEY_DOWN, KEY_CTRL_DOWN, KEY_CTRL_PAGEDOWN, KEY_CTRL_DOWN, KEY_CTRL_DOWN, KEY_CTRL_DOWN);
        INPUT_TRANSLATE(KEY_LEFT, KEY_CTRL_LEFT, KEY_SHIFT_LEFT, KEY_SHIFT_LEFT, KEY_CTRL_LEFT, KEY_CTRL_LEFT);
        INPUT_TRANSLATE(KEY_RIGHT, KEY_CTRL_RIGHT, KEY_SHIFT_RIGHT, KEY_SHIFT_RIGHT, KEY_CTRL_RIGHT, KEY_CTRL_RIGHT);

        INPUT_TRANSLATE(KEY_VIEWS, KEY_CTRL_EXIT, KEY_CTRL_QUIT, KEY_CTRL_EXIT, KEY_CTRL_EXIT, KEY_CTRL_EXIT);

        INPUT_TRANSLATE(KEY_NUM, KEY_CTRL_OPTN, KEY_SHIFT_OPTN, KEY_SHIFT_OPTN, KEY_CTRL_OPTN, KEY_CTRL_OPTN);
        INPUT_TRANSLATE(KEY_SYMB, KEY_CTRL_SETUP, KEY_CTRL_SETUP, KEY_CTRL_SETUP, KEY_CTRL_SETUP, KEY_CTRL_SETUP);
        INPUT_TRANSLATE(KEY_VARS, KEY_CTRL_VARS, KEY_CTRL_INS, KEY_CTRL_VARS, 'a', 'A');
        INPUT_TRANSLATE(KEY_MATH, KEY_CTRL_MENU, 0000000000000, 0000000000000, 'b', 'B');
        INPUT_TRANSLATE(KEY_ABC, KEY_CTRL_FRACCNVRT, KEY_CTRL_MIXEDFRAC, 0000000000000, 'c', 'C');
        INPUT_TRANSLATE(KEY_XTPHIN, KEY_CTRL_XTT, KEY_CHAR_EXPN10, KEY_CHAR_EXPN10, 'd', 'D');
        INPUT_TRANSLATE(KEY_BACKSPACE, KEY_CTRL_DEL, KEY_CTRL_DEL, KEY_CTRL_DEL, KEY_CTRL_DEL, KEY_CTRL_DEL);
        INPUT_TRANSLATE(KEY_SIN, KEY_CHAR_SIN, KEY_CHAR_ASIN, KEY_CHAR_ASIN, 'e', 'E');
        INPUT_TRANSLATE(KEY_COS, KEY_CHAR_COS, KEY_CHAR_ACOS, KEY_CHAR_ACOS, 'f', 'F');
        INPUT_TRANSLATE(KEY_TAN, KEY_CHAR_TAN, KEY_CHAR_ATAN, KEY_CHAR_ATAN, 'g', 'G');
        INPUT_TRANSLATE(KEY_LN, KEY_CHAR_LN, KEY_CHAR_EXP, KEY_CHAR_EXP, 'h', 'H');
        INPUT_TRANSLATE(KEY_LOG, KEY_CHAR_LOG, KEY_CHAR_EXPN10, KEY_CHAR_EXPN10, 'i', 'I');
        INPUT_TRANSLATE(KEY_X2, KEY_CHAR_SQUARE, KEY_CHAR_ROOT, KEY_CHAR_ROOT, 'j', 'J');
        INPUT_TRANSLATE(KEY_XY, KEY_CHAR_POW, KEY_CHAR_POWROOT, KEY_CHAR_POWROOT, 'k', 'K');
 
        INPUT_TRANSLATE(KEY_LEFTBRACKET, '(', '[', KEY_CTRL_CLIP, 'l', 'L');
        INPUT_TRANSLATE(KEY_RIGHTBRACKET, ')', ']', KEY_CTRL_PASTE, 'm', 'M');

        INPUT_TRANSLATE(KEY_COMMA, ',', ',', ',', 'o', 'O');

        INPUT_TRANSLATE(KEY_0, '0', KEY_CTRL_CATALOG, KEY_CTRL_CATALOG, '"', '"');
        INPUT_TRANSLATE(KEY_1, '1', KEY_CTRL_PRGM, KEY_CTRL_PRGM, 'x', 'X');
        INPUT_TRANSLATE(KEY_2, '2', KEY_CHAR_IMGNRY, KEY_CHAR_IMGNRY, 'y', 'Y');
        INPUT_TRANSLATE(KEY_3, '3', KEY_CHAR_PI, KEY_CHAR_PI, 'z', 'Z');
        INPUT_TRANSLATE(KEY_4, '4', KEY_CHAR_MAT, KEY_CHAR_MAT, 't', 'T');
        INPUT_TRANSLATE(KEY_5, '5', '[', '[', 'u', 'U');
        INPUT_TRANSLATE(KEY_6, '6', ']', ']', 'v', 'V');
        INPUT_TRANSLATE(KEY_7, '7', KEY_CHAR_LIST, KEY_CHAR_LIST, 'p', 'P');
        INPUT_TRANSLATE(KEY_8, '8', '{', '{', 'q', 'Q');
        INPUT_TRANSLATE(KEY_9, '9', '}', '}', 'r', 'R');

        INPUT_TRANSLATE(KEY_DOT, '.', '=', '`', ':', ':');


        INPUT_TRANSLATE(KEY_PLUS, KEY_CHAR_PLUS, KEY_CHAR_PLUS, KEY_CHAR_PLUS, ' ', ' ');
        INPUT_TRANSLATE(KEY_SUBTRACTION, KEY_CHAR_MINUS, KEY_CHAR_THETA, KEY_CHAR_ANGLE, 'w', 'W');
        INPUT_TRANSLATE(KEY_MULTIPLICATION, KEY_CHAR_MULT, '!', '!', 's', 'S');
        INPUT_TRANSLATE(KEY_DIVISION, KEY_CHAR_DIV, KEY_CHAR_RECIP, KEY_CHAR_PMINUS, 'n', 'N');

        //INPUT_TRANSLATE(KEY_ON, KEY_CTRL_AC, KEY_CTRL_QUIT, KEY_CTRL_AC, KEY_CTRL_AC, KEY_CTRL_AC);

        INPUT_TRANSLATE(KEY_ENTER, KEY_CTRL_EXE, KEY_CHAR_ANS, KEY_CHAR_ANS, KEY_CHAR_CR, KEY_CHAR_CR);

        INPUT_TRANSLATE(KEY_NEGATIVE, KEY_CHAR_PMINUS, '|', '|', ';', ';');

    default:
        break;
    }

    }

    //printf("kpress:%d, %d, %d\n",test_key, tr_key, kpress);
    if(kpress)
    {
        if(tr_key == test_key)
        {
            
            return true;
        } 
    }
    
    return false;

}

int GetKey(int *key) {
    int pkey;
    bool press = vGL_getkey(&pkey);

    if (pkey != -1 && press) {
        // printf("pkey:%08x\n", key);

        switch (pkey) {


        case KEY_SHIFT:
            *key = 0;
            // *key = KEY_CTRL_SHIFT; 
            if (keyStatus & 1) {
                keyStatus &= ~1;
                rshift = 1;
            } else if(rshift)
            {
                rshift = 0;
            } else {
                keyStatus |= 1;
                rshift = 0;
            }

            flush_indBit();

            break;

        case KEY_ALPHA:
            *key = 0;
            // *key = KEY_CTRL_ALPHA;

            if (keyStatus & 4) {
                keyStatus &= ~4;
                keyStatus |= 8;
            } else {

                if (keyStatus & 8) {
                    keyStatus &= ~4;
                    keyStatus &= ~8;
                } else {

                    keyStatus |= 4;
                }
            }

            flush_indBit();

            break;

        
        INPUT_TRANSLATE(KEY_F1, KEY_CTRL_F1, KEY_CTRL_F1, KEY_CTRL_F1, KEY_CTRL_F1, KEY_CTRL_F1);
        INPUT_TRANSLATE(KEY_F2, KEY_CTRL_F2, KEY_CTRL_F2, KEY_CTRL_F2, KEY_CTRL_F2, KEY_CTRL_F2);
        INPUT_TRANSLATE(KEY_F3, KEY_CTRL_F3, KEY_CTRL_F3, KEY_CTRL_F3, KEY_CTRL_F3, KEY_CTRL_F3);
        INPUT_TRANSLATE(KEY_F4, KEY_CTRL_F4, KEY_CTRL_F4, KEY_CTRL_F4, KEY_CTRL_F4, KEY_CTRL_F4);
        INPUT_TRANSLATE(KEY_F5, KEY_CTRL_F5, KEY_CTRL_F5, KEY_CTRL_F5, KEY_CTRL_F5, KEY_CTRL_F5);
        INPUT_TRANSLATE(KEY_F6, KEY_CTRL_F6, KEY_CTRL_F6, KEY_CTRL_F6, KEY_CTRL_F6, KEY_CTRL_F6);
        
        //INPUT_TRANSLATE(_rawin, normal, shift_l, shift_r, alpha_small, alpha_capital)
        INPUT_TRANSLATE(KEY_UP, KEY_CTRL_UP, KEY_CTRL_PAGEUP, KEY_CTRL_UP, KEY_CTRL_UP, KEY_CTRL_UP);
        INPUT_TRANSLATE(KEY_DOWN, KEY_CTRL_DOWN, KEY_CTRL_PAGEDOWN, KEY_CTRL_DOWN, KEY_CTRL_DOWN, KEY_CTRL_DOWN);
        INPUT_TRANSLATE(KEY_LEFT, KEY_CTRL_LEFT, KEY_SHIFT_LEFT, KEY_SHIFT_LEFT, KEY_CTRL_LEFT, KEY_CTRL_LEFT);
        INPUT_TRANSLATE(KEY_RIGHT, KEY_CTRL_RIGHT, KEY_SHIFT_RIGHT, KEY_SHIFT_RIGHT, KEY_CTRL_RIGHT, KEY_CTRL_RIGHT);

        INPUT_TRANSLATE(KEY_VIEWS, KEY_CTRL_EXIT, KEY_CTRL_QUIT, KEY_CTRL_EXIT, KEY_CTRL_EXIT, KEY_CTRL_EXIT);

        INPUT_TRANSLATE(KEY_NUM, KEY_CTRL_OPTN, KEY_SHIFT_OPTN, KEY_SHIFT_OPTN, KEY_CTRL_OPTN, KEY_CTRL_OPTN);
        INPUT_TRANSLATE(KEY_SYMB, KEY_CTRL_SETUP, KEY_CTRL_SETUP, KEY_CTRL_SETUP, KEY_CTRL_SETUP, KEY_CTRL_SETUP);
        INPUT_TRANSLATE(KEY_VARS, KEY_CTRL_VARS, KEY_CTRL_INS, KEY_CTRL_VARS, 'a', 'A');
        INPUT_TRANSLATE(KEY_MATH, KEY_CTRL_MENU, 0000000000000, 0000000000000, 'b', 'B');
        INPUT_TRANSLATE(KEY_ABC, KEY_CTRL_FRACCNVRT, KEY_CTRL_MIXEDFRAC, 0000000000000, 'c', 'C');
        INPUT_TRANSLATE(KEY_XTPHIN, KEY_CTRL_XTT, KEY_CHAR_EXPN10, KEY_CHAR_EXPN10, 'd', 'D');
        INPUT_TRANSLATE(KEY_BACKSPACE, KEY_CTRL_DEL, KEY_CTRL_DEL, KEY_CTRL_DEL, KEY_CTRL_DEL, KEY_CTRL_DEL);
        INPUT_TRANSLATE(KEY_SIN, KEY_CHAR_SIN, KEY_CHAR_ASIN, KEY_CHAR_ASIN, 'e', 'E');
        INPUT_TRANSLATE(KEY_COS, KEY_CHAR_COS, KEY_CHAR_ACOS, KEY_CHAR_ACOS, 'f', 'F');
        INPUT_TRANSLATE(KEY_TAN, KEY_CHAR_TAN, KEY_CHAR_ATAN, KEY_CHAR_ATAN, 'g', 'G');
        INPUT_TRANSLATE(KEY_LN, KEY_CHAR_LN, KEY_CHAR_EXP, KEY_CHAR_EXP, 'h', 'H');
        INPUT_TRANSLATE(KEY_LOG, KEY_CHAR_LOG, KEY_CHAR_EXPN10, KEY_CHAR_EXPN10, 'i', 'I');
        INPUT_TRANSLATE(KEY_X2, KEY_CHAR_SQUARE, KEY_CHAR_ROOT, KEY_CHAR_ROOT, 'j', 'J');
        INPUT_TRANSLATE(KEY_XY, KEY_CHAR_POW, KEY_CHAR_POWROOT, KEY_CHAR_POWROOT, 'k', 'K');
 
        INPUT_TRANSLATE(KEY_LEFTBRACKET, '(', '[', KEY_CTRL_CLIP, 'l', 'L');
        INPUT_TRANSLATE(KEY_RIGHTBRACKET, ')', ']', KEY_CTRL_PASTE, 'm', 'M');

        INPUT_TRANSLATE(KEY_COMMA, ',', ',', ',', 'o', 'O');

        INPUT_TRANSLATE(KEY_0, '0', KEY_CTRL_CATALOG, KEY_CTRL_CATALOG, '"', '"');
        INPUT_TRANSLATE(KEY_1, '1', KEY_CTRL_PRGM, KEY_CTRL_PRGM, 'x', 'X');
        INPUT_TRANSLATE(KEY_2, '2', KEY_CHAR_IMGNRY, KEY_CHAR_IMGNRY, 'y', 'Y');
        INPUT_TRANSLATE(KEY_3, '3', KEY_CHAR_PI, KEY_CHAR_PI, 'z', 'Z');
        INPUT_TRANSLATE(KEY_4, '4', KEY_CHAR_MAT, KEY_CHAR_MAT, 't', 'T');
        INPUT_TRANSLATE(KEY_5, '5', '[', '[', 'u', 'U');
        INPUT_TRANSLATE(KEY_6, '6', ']', ']', 'v', 'V');
        INPUT_TRANSLATE(KEY_7, '7', KEY_CHAR_LIST, KEY_CHAR_LIST, 'p', 'P');
        INPUT_TRANSLATE(KEY_8, '8', '{', '{', 'q', 'Q');
        INPUT_TRANSLATE(KEY_9, '9', '}', '}', 'r', 'R');

        INPUT_TRANSLATE(KEY_DOT, '.', '=', '`', ':', ':');


        INPUT_TRANSLATE(KEY_PLUS, KEY_CHAR_PLUS, KEY_CHAR_PLUS, KEY_CHAR_PLUS, ' ', ' ');
        INPUT_TRANSLATE(KEY_SUBTRACTION, KEY_CHAR_MINUS, KEY_CHAR_THETA, KEY_CHAR_ANGLE, 'w', 'W');
        INPUT_TRANSLATE(KEY_MULTIPLICATION, KEY_CHAR_MULT, '!', '!', 's', 'S');
        INPUT_TRANSLATE(KEY_DIVISION, KEY_CHAR_DIV, KEY_CHAR_RECIP, KEY_CHAR_PMINUS, 'n', 'N');

        //INPUT_TRANSLATE(KEY_ON, KEY_CTRL_AC, KEY_CTRL_QUIT, KEY_CTRL_AC, KEY_CTRL_AC, KEY_CTRL_AC);

        INPUT_TRANSLATE(KEY_ENTER, KEY_CTRL_EXE, KEY_CHAR_ANS, KEY_CHAR_ANS, KEY_CHAR_CR, KEY_CHAR_CR);

        INPUT_TRANSLATE(KEY_NEGATIVE, KEY_CHAR_PMINUS, '|', '|', ';', ';');

        case KEY_ON:
        {

            if(keyStatus & 1){
                if(XcasExitCb)
                {
                    
                    
                    vGL_clearArea(0, 0, 255, 126);
                    vGL_putString(0, 0, (char *)"Quitting...", COLOR_BLACK , COLOR_WHITE, 16);
                    vGL_putString(0, 16, (char *)"Waiting session save...", COLOR_BLACK , COLOR_WHITE, 16);

                    XcasExitCb();
                    
                    
                    //ll_flash_sync();
                    //printf("power OFF\n");
                    khicasRunning = false;
                    keyStatus = 0;
                    vTaskDelay(pdMS_TO_TICKS(1000));

                    lv_obj_invalidate(lv_scr_act());
                    SystemUIResume();

                    vTaskDelete(NULL);

                    
                    //ll_power_off();
                   //ll_power_off();
                    //ll_power_off();
                }
            }  
            else if(rshift)  {*key = KEY_CTRL_AC;}  
            else if(keyStatus & 4)  {*key = KEY_CTRL_AC;}  
            else if(keyStatus & 8)  {*key = KEY_CTRL_AC;}  
            else {*key = KEY_CTRL_AC;}  

          break;

        }

        default:
            *key = 0;
            return KEYREP_NOEVENT;
            break;
        }

        return KEYREP_KEYEVENT;
    }

    *key = 0;

    return KEYREP_NOEVENT;
}

int RTC_GetTicks() {
    return ll_rtc_get_sec();
}
 
void RTC_SetDateTime(unsigned char *time) {
    uint8_t hours = time[4];
    uint8_t minus = time[5];
    printf("set time %02d:%02d\n", hours, minus);
    ll_rtc_set_sec(hours * (60 * 60) + minus * 60);
}



void Sleep(int millisecond) {
    int start = ll_get_time_ms();
    while (ll_get_time_ms() - start < millisecond) {
        ;
    }
}



} // extern "C"    

void sprint_double(char *ch, double d)
{
    sprintf(ch, "%g", d);
}

void PrintMini(int x, int y, unsigned char const *s, int rev) {
    // printf("Pmini(%d,%d,%d):%s\n", x,y,rev ,s);
    if (rev) {
        vGL_putString(x, y, (char *)s, 255, 0, 12);
    } else {
        vGL_putString(x, y, (char *)s, 0, 255, 12);
    }
}

void PrintXY(int x, int y, unsigned char const *s, int rev) {
    // printf("Pxy(%d,%d,%d):%s\n",x,y, rev,s);
    if (rev) {
        vGL_putString(x, y, (char *)s, 255, 0, 12);
    } else {
        vGL_putString(x, y, (char *)s, 0, 255, 12);
    }
}
 
void PopUpWin(int win) {
    printf("PopUpWin:%d\n", win);
} 
 
void Bdisp_AreaClr_VRAM(DISPBOX *area) {
    //printf("clra:%d,%d,%d,%d\n", area->left, area->top, area->right, area->bottom);
    vGL_clearArea(area->left, area->top, area->right, area->bottom);
    // vGL_FlushVScreen();     
}  


void Bdisp_AreaFillVRAM(DISPBOX *area, unsigned short color) 
{
    //printf("SETAREA:%d,%d,%d,%d, %08x\n", area->left, area->top, area->right, area->bottom, color);
    vGL_setArea(area->left, area->top, area->right, area->bottom, color);
}

void Bdisp_AreaReverseVRAM(int x0, int y0, int x1, int y1) {
    //printf("areaRev:%d,%d,%d,%d\n", x0,y0,x1,y1);
    vGL_reverseArea(x0, y0, x1, y1); 

    // vGL_FlushVScreen();
}

void Bdisp_AllClr_VRAM() {
    vGL_clearArea(0, 0, VIR_LCD_PIX_W - 1, VIR_LCD_PIX_H - 1);
    // vGL_FlushVScreen();
}
void Bdisp_SetPoint_VRAM(int x, int y, int c) {
    //printf("setp:%d,%d,%d\n",x,y,c);
    //vGL_SetPoint(x, y, (c > 0) ? COLOR_BLACK : COLOR_WHITE);
    vGL_SetPoint(x, y, c);
}

int Bdisp_GetPoint_VRAM(int x, int y) {

    return vGL_GetPoint(x, y);
}

void Bdisp_PutDisp_DD() {
    // vGL_FlushVScreen();
    // printf("Bdisp_PutDisp_DD\n");
} 
 

void Bdisp_DrawLineVRAM(int x0, int y0, int x1, int y1) {
    printf("pline:%d,%d,%d,%d\n", x0, y0, x1, y1);
    
} 




#define MAIN_DIR "/xcas"
#define RAW_PATH "\\\\fls0"

void Bfile_StrToName_ncpy(unsigned short *dest, const unsigned char *source, int len) {
    //int l = path_replace((char *)source);
    //memcpy(dest, path_buf, l);

    char *d = (char *)dest, *s = (char *)source;

    if(memcmp(RAW_PATH, s, sizeof(RAW_PATH) - 1) == 0)
    {   
        int ll = strlen(s);
        ll -= sizeof(RAW_PATH) + 1;
        strcpy(d, MAIN_DIR);

        d += sizeof(MAIN_DIR) - 1;
        s += sizeof(RAW_PATH) - 1;
        strcpy( d, 
                s);  
    }else{
        strcpy(d, MAIN_DIR "/");
        d += sizeof(MAIN_DIR "/") - 1;
        strcpy( d, 
                s);  
    } 

    for(int i = 0; i < len; i++)
    {
        if(d[i] == '\\')
        {
            d[i] = '/';
        }
    }


    printf("StrToName:%s->%s\n",(char *)source, (char *)dest);

    //char *cDest = (char *)dest;
    //strlcpy(cDest, (const char *)source, len);
}
void Bfile_NameToStr_ncpy(unsigned char *dest, const unsigned short *source , int len) {
    printf("NameToStr:%s\n", (char *)source);
    strlcpy((char *)dest, (const char *)source, len);
}


int Bfile_OpenFile_OS(unsigned short *pFile, int mode) {
    //path_replace((char *)pFile);
    char *c_path = (char *)pFile;
    printf("open[%d]:%s\n", mode, (char *)c_path);

    #if FS_TYPE == FS_FATFS

    FIL *handle = (FIL *)pvPortMalloc(sizeof(FIL));
    FRESULT fr;
    if(handle == NULL)
    {
        return -1;
    }
    fr = f_open(handle, c_path, mode == _OPENMODE_READ ? (FA_OPEN_EXISTING | FA_READ) : (FA_OPEN_EXISTING | FA_READ | FA_WRITE));
    if(fr != FR_OK)
    {
        printf("f_open:[%s]:err:%d,%d\n",c_path,fr,mode);
        vPortFree(handle);
        return -1;
    }
    return (int)handle;
    #else
        lfs *fs = (lfs *)GetFsObj();
        lfs_file_t *handle = (lfs_file_t *)pvPortMalloc(sizeof(lfs_file_t));
        memset(handle, 0, sizeof(lfs_file_t));
        int fr;
        if(handle == NULL)
        {
            return -1;
        }

        fr = lfs_file_open(fs, handle, (const char *)c_path, _OPENMODE_READ ? (LFS_O_RDONLY) : (LFS_O_RDWR));
        printf("open res:%d\n",fr);
        if(fr)
        {
            printf("f_open:[%s]:err:%d,%d\n",c_path,fr,mode);
            vPortFree(handle);
            return -1;
        }

    return (int)handle;
    #endif

    
}

int Bfile_CreateFile(unsigned short *pFile, int size) {
    
    //path_replace((char *)pFile);
    char *c_path = (char *)pFile;
    printf("Create[%d]:%s\n", size, (char *)c_path);

    #if FS_TYPE == FS_FATFS

    FIL *handle = (FIL *)pvPortMalloc(sizeof(FIL));
    FRESULT fr;
    if(handle == NULL)
    {
        return -1;
    }
    fr = f_open(handle, c_path, FA_CREATE_NEW | FA_WRITE);
    if(fr != FR_OK)
    {
        printf("Create 1:[%s]:err:%d,%d\n",c_path,fr,size);
        vPortFree(handle);
        return -1;
    }
    fr = f_expand(handle, size, 1);
    if(fr != FR_OK)
    {
        printf("Create 2:[%s]:err:%d,%d\n",c_path,fr,size);
        vPortFree(handle);
        return -1;
    }

    f_sync(handle);
    f_close(handle);
    vPortFree(handle);

    #else
    lfs *fs = (lfs *)GetFsObj();
    lfs_file_t *handle = (lfs_file_t *)pvPortMalloc(sizeof(lfs_file_t));
    int fr;
    if(handle == NULL)
    {
        return -1;
    }
    fr = lfs_file_open(fs, handle, (const char *)c_path, (LFS_O_RDWR | LFS_O_CREAT));
    if(fr)
    {
        printf("Create 1:[%s]:err:%d,%d\n",c_path,fr,size);
        vPortFree(handle);
        return -1;
    }
    fr = lfs_file_truncate(fs, handle, size);
    if(fr)
    {
        printf("Create 2:[%s]:err:%d,%d\n",c_path,fr,size);
        vPortFree(handle);
        return -1;
    }

    lfs_file_close(fs, handle);
    vPortFree(handle);
    #endif

    return 0;
}
void Bfile_CloseFile_OS(int hFile) {
    #if FS_TYPE == FS_FATFS
    FIL *handle = (FIL *)hFile;
    f_sync(handle);
    f_close(handle);
    vPortFree(handle);
    #else
    lfs *fs = (lfs *)GetFsObj();
    lfs_file_t *handle = (lfs_file_t *)hFile;
    lfs_file_close(fs, handle);
    vPortFree(handle);
    #endif

}

void Bfile_WriteFile_OS(int hFile, const char *data, size_t len) {
    #if FS_TYPE == FS_FATFS
    FIL *handle = (FIL *)hFile;
    UINT bw;
    FRESULT fr;

    if((len == 0))
    {
        return;
    }
    printf("WR:%d\n", len);
    
    fr = f_write(handle, (char *)data, len, &bw);
    if(fr != FR_OK)
    {
        printf("WR FAIL.\n");
    }

    f_sync(handle);

    #else
    if((len == 0))
    {
        return;
    }
    lfs *fs = (lfs *)GetFsObj();
    lfs_file_t *handle = (lfs_file_t *)hFile;
    int fr;

    fr = lfs_file_write(fs, handle, data, len);
    if(fr < 0)
    {
        printf("WR FAIL.\n");
    }

    #endif
    //printf("Bfile_WriteFile_OS\n");
}

void Bfile_DeleteEntry(unsigned short *pFile) {
    #if FS_TYPE == FS_FATFS
    char *c_path = (char *)pFile;
    printf("delete:%s\n", c_path);
    f_unlink(c_path);
    #else
    lfs *fs = (lfs *)GetFsObj();
    lfs_remove(fs, (char *)pFile);

    #endif
}

int Bfile_ReadFile_OS(int HANDLE, void *buf, int size, int readpos) {
#if FS_TYPE == FS_FATFS
    FIL *handle = (FIL *)HANDLE;
    UINT bw;
    FRESULT fr;
    if(!handle)
    {
        printf("VOID POINTER!\n");
    }
    if(readpos >= 0)
        fr = f_lseek(handle, readpos);
    if(fr){
        printf("f_lseek failed.\n");
        return -1;
    }

    fr = f_read(handle, buf, size, &bw);

    if(fr){
        printf("f_read failed.\n");
        return -1;
    }
    return bw;
#else
    printf("handle:%08x\n", HANDLE);

    lfs *fs = (lfs *)GetFsObj();
    lfs_file_t *handle = (lfs_file_t *)HANDLE;
    int fr;
    

    if(!handle)
    {
        printf("VOID POINTER!\n");
    }

    if(readpos < 0)
    {
        //fr = lfs_file_seek(fs, handle, 0 + size - 1, LFS_SEEK_END);
        fr = 0;
    }else
    {
        fr = lfs_file_seek(fs, handle, readpos, LFS_SEEK_SET);
    }
    printf("sz:%d,adr:%d\n",lfs_file_size(fs, handle) , lfs_file_tell(fs, handle));
    
    if(fr < 0)
    {
        printf("f_lseek failed:%d, %d\n",fr, readpos);
        return fr;
    }
    fr = lfs_file_read(fs, handle, buf, size);
    if(fr < 0){
        printf("f_read failed:%d\n", fr);
    }
    printf("adr:%d\n", lfs_file_tell(fs, handle));
    printf("RD:");
        for(int i = 0; i < size; i ++)
        {
            printf("%02x ", ((char *)buf)[i]);
        }
        printf("\n");

    return fr;

#endif
}

int Bfile_ReadFile(int HANDLE, void *buf, int size, int readpos) {
    return Bfile_ReadFile_OS(HANDLE, buf, size, readpos);
}

size_t Bfile_GetFileSize_OS(int hFile) {
    #if FS_TYPE == FS_FATFS
    FIL *handle = (FIL *)hFile;
    return f_size(handle);
    #else
    lfs *fs = (lfs *)GetFsObj();
    lfs_file_t *handle = (lfs_file_t *)hFile;
    return lfs_file_size(fs, handle);
    #endif
}

#if FS_TYPE == FS_FATFS
typedef struct findHandle_t
{
    DIR dp;
    FILINFO finfo;

}findHandle_t;
#endif

int Bfile_FindFirst(const unsigned short *pathname, int *FindHandle, const unsigned short *foundfile, void *fileinfo) {

    printf("findFirst:%s\n", pathname);
    #if FS_TYPE == FS_FATFS
    FRESULT fr;
    findHandle_t *fh = (findHandle_t *)pvPortMalloc(sizeof(findHandle_t));
    FILE_INFO *foundfi = (FILE_INFO *)fileinfo;

    if(!fh)
    {
        printf("FF failed to open dir[NOMEM]:%s\n",(const char *)pathname);
        *FindHandle = 0;
        return -1;
    }
     

    fr = f_findfirst(&fh->dp, &fh->finfo, (const char *)pathname, "*.*");
    if(fr)
    {
        printf("FF failed to open dir[%d]:%s\n", fr, (const char *)pathname);
        vPortFree(fh);
        *FindHandle = 0;
        return -1;
    }

    *FindHandle = (int)fh;
    foundfi->fsize = fh->finfo.fsize;
    strcpy((char *)foundfile, fh->finfo.fname);

    return 0;
    #else


    return -1;
    #endif
}
int Bfile_FindNext(int FindHandle, const unsigned short *foundfile, void *fileinfo) {

 #if FS_TYPE == FS_FATFS
    if(!FindHandle)
    {
        return -1;
    }

    findHandle_t *fh = (findHandle_t *)FindHandle;
    FRESULT fr;
    FILE_INFO *foundfi = (FILE_INFO *)fileinfo;

    fr = f_findnext(&fh->dp, &fh->finfo);
    if(fr)
    {
        return -1;
        
    }

    if(!fh->finfo.fname[0])
    {
        return -1;
    }

    foundfi->fsize = fh->finfo.fsize;
    strcpy((char *)foundfile, fh->finfo.fname);
    return 0;
 #else


    return -1;
#endif
}
int Bfile_FindClose(int FindHandle) {
 #if FS_TYPE == FS_FATFS    
    if(!FindHandle)
    {
        return -1;
    }
    findHandle_t *fh = (findHandle_t *)FindHandle;
    vPortFree(fh);
    return 0;

 #else


    return -1;
#endif

}

char memLoad_path_buf[270];
void *memory_load(char *adresse) {
    printf("memLoad:%s\n",adresse);
    Bfile_StrToName_ncpy((unsigned short *)memLoad_path_buf, (const unsigned char *)adresse, 270);
#if FS_TYPE == FS_FATFS
    
    FRESULT fr;
    FIL *f = (FIL *)pvPortMalloc(sizeof(FIL));
    UINT br;
    fr = f_open(f, memLoad_path_buf, FA_OPEN_EXISTING | FA_READ);
    if(fr)
    {
        vPortFree(f);
        return NULL;
    }

    int fSize = f_size(f);
    char *mem;
    if(fSize)
    {
        mem = (char *)pvPortMalloc(fSize);
        fr = f_read(f, mem, fSize, &br);
        if(!fr)
        {
            vPortFree(mem);
            vPortFree(f);
            return NULL;
        }
        vPortFree(f);
        return mem;
    }
    return NULL;
#else
    lfs *fs = (lfs *)GetFsObj();
    lfs_file file;
    int fr;
    fr = lfs_file_open(fs, &file, memLoad_path_buf, LFS_O_RDONLY);
    if(fr)
    {
        return NULL;
    }

    int fSize = lfs_file_size(fs, &file);
    char *mem;
    if(fSize)
    {
        mem = (char *)pvPortMalloc(fSize);
        fr = lfs_file_read(fs, &file, mem, fSize);
        if(!fr)
        {
            vPortFree(mem);
            return NULL;
        }
        lfs_file_close(fs, &file);
        return mem;
    }
    return NULL;

#endif
    
}

extern "C" {

int Cursor_SetPosition(char column, char row) {
    //printf("CSP:%d:%d\n", column, row);
    vGL_locateConcur(column, row);
    return 0;
}
int Cursor_SetFlashStyle(short int flashstyle) {
    //printf("CSF:%d\n", flashstyle);
    vGL_concurEnable(true);
    return 0;
}

void Cursor_SetFlashMode(long flashmode) {
    
    //vGL_concurEnable(flashmode > 0);
}
void Cursor_SetFlashOff(void) {
    vGL_concurEnable(false);
}
void Cursor_SetFlashOn(char flash_style) {
    vGL_concurEnable(true);
}

int MB_ElementCount(const char *buf) // like strlen but for the graphical length of multibyte strings
{
    // printf("MB_ElementCount:%08x\n",buf);
    return strlen(buf); // FIXME for UTF8
}

void SetQuitHandler(void (*callback)(void)) {
    XcasExitCb = callback;
}


}


bool chkEsc(void)
{   
    bool ret=vGL_chkEsc();
    //printf("ckESC:%d\n", ret);
    return ret;
}

bool ktimer_start = false;

typedef struct ktimer_t
{
    unsigned int cnt;
    unsigned int period;
    void (*callback)(void);
}ktimer_t;

ktimer_t ktimer[6];

void KillTimer(int TimerID)
{
    ktimer[TimerID].period = 0;
    ktimer[TimerID].cnt = 0;
    ktimer[TimerID].callback = NULL;
}

void kcas_timer(void *arg)
{

    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
        for(int i = 0; i < 6; i++)
        {
            if(ktimer[i].period > 0)
            {
                ktimer[i].cnt += 100;
            }

            if(ktimer[i].cnt > ktimer[i].period)
            {
                ktimer[i].cnt = 0;
                ktimer[i].callback(); 
            }
        }

        if(!khicasRunning)
        {
            for(int i = 0; i < 6; i++)
                KillTimer(i);
            vTaskDelete(NULL);
        }

    }
} 

int SetTimer(int TimerID, int period, void (*callback)(void))
{
    if(!ktimer_start){
        memset(&ktimer, 0, sizeof(ktimer));
        xTaskCreate(kcas_timer, "khicas ChkTask", 1000, NULL, configMAX_PRIORITIES - 2, NULL);
        ktimer_start = true;
    }

    if(ktimer[TimerID].period == 0)
    {
        ktimer[TimerID].cnt = 0;
        ktimer[TimerID].period = period;
        ktimer[TimerID].callback = callback;
        return TimerID;
    }else{
        return -1;
    }
    return -1;
}


