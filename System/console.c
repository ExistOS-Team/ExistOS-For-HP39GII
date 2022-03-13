
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>

#include "console.h"
#include "sys_llapi.h"

#define MAX_BLOCK_SIZE  1024

#define COL      35
#define ROW       7

#define FONTSIZE_H        12
#define FONTSIZE_W        8

typedef struct log_block_t
{
    uint32_t blocks;
    struct log_block_t *next;
    struct log_block_t *prev;
    char align;
    char *str;
}log_block_t;

log_block_t *log_blocks_head = NULL;
log_block_t *log_blocks_tail = NULL;

int selectItem = -1;

void refresh_menu(char *s[], uint8_t selectBit)
{
    ll_DispPutBox(0, (ROW + 2) * FONTSIZE_H + 5, 255, 127, true, 0);

    ll_DispHLine((ROW + 2) * FONTSIZE_H + 5, 0, 255, 255);

    for(int i = 1; i < 7; i++){
        ll_DispVLine(i * (259 / 6), (ROW + 2) * FONTSIZE_H + 5, 126, 255);
    }

    for(int i = 0; i < 6; i++){
        ll_DispPutStr(s[i], (i * (259 / 6)) + 2, (ROW + 2) * FONTSIZE_H + 6, 
        ((selectBit >> i) & 1) ? 0 : 255,
        ((selectBit >> i) & 1) ? 255 : 0,
        12);
    }
    
}

int getTotalBlock()
{
    return log_blocks_head->blocks;
}

int selectBlock = -1;

void setSelectBlock(int which)
{
    selectBlock = which;
}

char *getSelectBlockStr()
{
    log_block_t *chain = log_blocks_head;
    uint32_t bcnt = 0;
    if(selectBlock < 0){
        return NULL;
    }

    while(chain){
        
        if((bcnt + 1) == selectBlock)
        {
            return chain->str;
        }
        bcnt++;
        chain = chain->next;
    }

    return NULL;
}

void refresh_block(bool isfromSelectOn)
{
    log_block_t *chain = log_blocks_tail;
    int sline = ROW;
    int bcnt = log_blocks_head->blocks;
    char linebuf[COL + 1];
    bool lastLine, firstLine;

    if(isfromSelectOn && (selectBlock > -1))
    {
        while((chain != NULL) && (bcnt > -1)){
            
            if((bcnt) == selectBlock){
                break;
            }
            chain = chain->prev;
            bcnt --;
        }
    }
    
    while((chain != NULL) && (bcnt-- > -1)){
        lastLine = true;

        int linecnt = strlen(chain->str) / COL ;
        int lineLen;
        while((sline >= 0) && (linecnt >= 0)){
            //printf("sline:%d, linecnt:%d\n",sline, linecnt);
            //printf("bcnt:%d, selectBlock:%d\n",bcnt, selectBlock);
            memset(linebuf, 0, sizeof(linebuf));
            memcpy(linebuf, &chain->str[linecnt * COL], lastLine ? strlen(chain->str) % COL : COL);
            linebuf[lastLine ? strlen(chain->str) % COL : COL] = 0;
            lastLine = false;
            
            ll_DispPutBox(0, sline * FONTSIZE_H, COL * FONTSIZE_W, (sline + 1) * FONTSIZE_H, true, 0);
            if((chain->align == ALIGN_LEFT) || (linecnt > 0)){
                ll_DispPutStr(linebuf, 
                 0  ,
                sline * FONTSIZE_H, 
                ((bcnt + 1) == selectBlock) ? 0 : 255,
                ((bcnt + 1) == selectBlock) ? 255 : 0, 
                12);
            }else
            {

                lineLen = strlen(linebuf);
                ll_DispPutStr(linebuf, 
                 255 - (lineLen * (FONTSIZE_W - 1)) ,
                sline * FONTSIZE_H, 
                ((bcnt + 1) == selectBlock) ? 0 : 255,
                ((bcnt + 1) == selectBlock) ? 255 : 0, 
                12);
            }
            

            sline--;
            linecnt--;
        }

        if(sline > 0){
            ll_DispPutBox(0, 0, COL * FONTSIZE_W, (sline + 1) * FONTSIZE_H, true, 0);
        }

        chain = chain->prev;
    }

    ll_DispHLine((ROW + 1) * FONTSIZE_H + 1, 0, 255, 255);


}

char *console_get_block_string(uint32_t block)
{
    log_block_t *chain = log_blocks_head;
    uint32_t bcnt = 0;
    while((bcnt != block) && (chain->next))
    {
        chain = chain->next;
        bcnt++;
    }
    if(chain != NULL){
        return chain->str;
    }

    return NULL;
}

int console_put_block_align(char *s, char align)
{

    log_block_t *chain = log_blocks_head;
    uint32_t len = 0;
    uint32_t retBlock = 0;
    char *buf;

    while(s[len] != '\0')
    {
        len++;
        if(len > MAX_BLOCK_SIZE){
            return -EOVERFLOW;
        }
    }
    

    buf = malloc(len + 1);
    if(buf == NULL){
        return -ENOMEM;
    }
    memcpy(buf, s, len);
    buf[len] = '\0';


    if(log_blocks_head == NULL){
        log_blocks_head = malloc(sizeof(log_block_t));
        if(log_blocks_head == NULL){
            return -ENOMEM;
        }
        chain = log_blocks_head;
        chain->blocks = 0;
        chain->prev = NULL;
        retBlock = 0;
    }else{
        while(chain->next != NULL){
            chain = chain->next;
            retBlock++;
        }
        chain->next = malloc(sizeof(log_block_t));
        if(chain->next == NULL){
            return -ENOMEM;
        }
        chain->next->prev = chain;
        chain = chain->next;
    }

    chain->next = NULL;
    chain->str = buf;
    chain->align = align;

    log_blocks_tail = chain;
    log_blocks_head->blocks++;
    retBlock++;

    return retBlock;
}

int console_put_block(char *s)
{
    console_put_block_align(s, ALIGN_LEFT);
}

char con_buffer[256];
int console_printf(char *format, ...)
{
    va_list aptr;
    int ret;

    va_start(aptr, format);
    ret = vsprintf(con_buffer, format, aptr);
    console_put_block(con_buffer);
    va_end(aptr);
    
    refresh_block(false);
    return(ret);
}

char InputBuf[1024] = {0};
char InputBuf2[1024] = {0};
int indicate = 0;
int inBufLen = 0;

int leftBoundary = 0;

void refresh_inputBox()
{
    char linebuf[COL + 1];
    bool clear = false;

    

    while(indicate > leftBoundary + COL){
        leftBoundary += COL / 4;
        clear = true;
    }
    while (indicate < leftBoundary)
    {
        leftBoundary -= COL / 4;
        clear = true;
    }
    if(leftBoundary > 1024 - COL){
        leftBoundary = 1024 - COL;
    }
    if(leftBoundary < 0){
        leftBoundary = 0;
    }

    memcpy(linebuf, &InputBuf[leftBoundary], COL);
    linebuf[COL] = 0;

    //if(clear)
    {
        ll_DispPutBox(0, (ROW + 1) * FONTSIZE_H + 3 , COL * FONTSIZE_W, (ROW + 2) * FONTSIZE_H + 3 , true, 0);
    }

    ll_DispPutStr(linebuf, 0, (ROW + 1) * FONTSIZE_H + 4, 255, 0, 12);    
    ll_DispVLine((indicate - leftBoundary) * 7, (ROW + 1) * FONTSIZE_H + 3, (ROW + 2) * FONTSIZE_H + 3 , 255);
}

void shiftIndicate(int num)
{
    
    if(indicate + num > inBufLen){
        indicate = inBufLen;
    }
    if(indicate + num < 0){
        indicate = 0;
    }
    indicate += num;
    refresh_inputBox();
}

void backspace_in()
{
    if(indicate - 1 < 0){
        return;
    }

    if(indicate == inBufLen){
        InputBuf[--inBufLen] = 0;
        indicate--;
    }else{
        strcpy(InputBuf2, InputBuf);
        memcpy(&InputBuf[indicate - 1], &InputBuf2[indicate], inBufLen - indicate + 1);
        inBufLen--;
        InputBuf[inBufLen] = 0;
        indicate--;
    }

    refresh_inputBox();

}




void InputBox_in_ch(char c)
{
    if(c == 0){
        return;
    }
    if(indicate == inBufLen){
        InputBuf[indicate] = c;
        indicate++;
        InputBuf[indicate] = 0;
        inBufLen++;
    }else{
        strcpy(InputBuf2, InputBuf);
        memcpy(&InputBuf[indicate + 1], &InputBuf2[indicate], inBufLen - indicate + 1);
        InputBuf[indicate] = c;
        indicate++;
        inBufLen++;
    }

    refresh_inputBox();

}


int enter_in()
{
    if(inBufLen == 0)
    {
        return -1;
    }
    console_put_block(InputBuf);
    indicate = 0;
    inBufLen = 0;
    InputBuf[0] = 0;
    refresh_inputBox();
    refresh_block(false);

}

void InputBox_Clear()
{
    indicate = 0;
    inBufLen = 0;
    InputBuf[0] = 0;
    refresh_inputBox();
    refresh_block(false);
}

void console_init()
{


}