
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "keyboard.h"

extern uint8_t *configAddr;

char *SymbolTable = NULL;

const char numList[] = "0123456789";
static char numString[] = "0";


int getKey(char *key, char *press)
{
    if(configAddr[100] != 0xFE){
        *key = configAddr[100];
        *press = configAddr[101];
        configAddr[100] = 0xFE;
        return 1;
    }
    return 0;
}

char* keyTo_alpha(Keys_t k)
{

}


char* keyTo_Sym_string(Keys_t k, key_string_mode_t keyStringMode)
{
    uint32_t getKey = k;
    int sym_key;
    switch (getKey)
    {
    case KEY_XTPHIN:
        if(keyStringMode == KEY_STRING_CAS)     return "x";
        if(keyStringMode == KEY_STRING_PYTHON)  return "x";
        break;
    case KEY_COS:
        if(keyStringMode == KEY_STRING_CAS)     return "cos(";
        if(keyStringMode == KEY_STRING_PYTHON)  return "cos(";
        break;
    case KEY_SIN:
        if(keyStringMode == KEY_STRING_CAS)     return "sin(";
        if(keyStringMode == KEY_STRING_PYTHON)  return "sin(";
        break;
    case KEY_TAN:
        if(keyStringMode == KEY_STRING_CAS)     return "tan(";
        if(keyStringMode == KEY_STRING_PYTHON)  return "tan(";
        break;
    case KEY_LN:
        if(keyStringMode == KEY_STRING_CAS)     return "ln(";
        if(keyStringMode == KEY_STRING_PYTHON)  return "log(";
        break;
    case KEY_LOG:
        if(keyStringMode == KEY_STRING_CAS)     return "log(";
        if(keyStringMode == KEY_STRING_PYTHON)  return "log10(";
        break;
    case KEY_X2:
        if(keyStringMode == KEY_STRING_CAS)     return "^2";
        if(keyStringMode == KEY_STRING_PYTHON)  return "**2";
        break;
    case KEY_XY:
        if(keyStringMode == KEY_STRING_CAS)     return "^";
        if(keyStringMode == KEY_STRING_PYTHON)  return "**";
        break;
    case KEY_LEFTBRACKET:
        if(keyStringMode == KEY_STRING_CAS)     return "(";
        if(keyStringMode == KEY_STRING_PYTHON)  return "(";
        break;
    case KEY_RIGHTBRACET:
        if(keyStringMode == KEY_STRING_CAS)     return ")";
        if(keyStringMode == KEY_STRING_PYTHON)  return ")";
        break;
    case KEY_DIVISION:
        if(keyStringMode == KEY_STRING_CAS)     return "/";
        if(keyStringMode == KEY_STRING_PYTHON)  return "/";
        break;
    case KEY_COMMA:
        if(keyStringMode == KEY_STRING_CAS)     return ",";
        if(keyStringMode == KEY_STRING_PYTHON)  return ",";
        break;        
    case KEY_MULTIPLICATION:
        if(keyStringMode == KEY_STRING_CAS)     return "*";
        if(keyStringMode == KEY_STRING_PYTHON)  return "*";
        break;   
    case KEY_SUBTRACTION:
        if(keyStringMode == KEY_STRING_CAS)     return "-";
        if(keyStringMode == KEY_STRING_PYTHON)  return "-";
        break;   
    case KEY_PLUS:
        if(keyStringMode == KEY_STRING_CAS)     return "+";
        if(keyStringMode == KEY_STRING_PYTHON)  return "+";
        break;  
    case KEY_DOT:
        if(keyStringMode == KEY_STRING_CAS)     return ".";
        if(keyStringMode == KEY_STRING_PYTHON)  return ".";
        break;  
    case KEY_NEGATIVE:
        if(keyStringMode == KEY_STRING_CAS)     return "-";
        if(keyStringMode == KEY_STRING_PYTHON)  return "-";
        break;
    default:
        {
            char num;
            num = keyToNum(k);
            if(num != -1){
                numString[0] = num + '0';
                numString[1] = 0;
                return numString;
            }
        }
        break;
    }

    return NULL;
}


char keyTo_ALPHA(Keys_t k)
{
    switch (k)
    {
    case KEY_VARS:
        return 'A';
    case KEY_MATH:
        return 'B';
    case KEY_ABC:
        return 'C';
    case KEY_XTPHIN:
        return 'D';
    case KEY_SIN:
        return 'E';
    case KEY_COS:
        return 'F';
    case KEY_TAN:
        return 'G';
    case KEY_LN:
        return 'H';
    case KEY_LOG:
        return 'I';
    case KEY_X2:
        return 'J';
    case KEY_XY:
        return 'K';
    case KEY_LEFTBRACKET:
        return 'L';
    case KEY_RIGHTBRACET:
        return 'M';
    case KEY_DIVISION:
        return 'N';
    case KEY_COMMA:
        return 'O';
    case KEY_7:
        return 'P';
    case KEY_8:
        return 'Q';
    case KEY_9:
        return 'R';
    case KEY_MULTIPLICATION:
        return 'S';
    case KEY_4:
        return 'T';
    case KEY_5:
        return 'U';
    case KEY_6:
        return 'V';
    case KEY_SUBTRACTION:
        return 'W';
    case KEY_1:
        return 'X';
    case KEY_2:
        return 'Y';
    case KEY_3:
        return 'Z';
    case KEY_PLUS:
        return ' ';
    case KEY_0:
        return '"';
    case KEY_DOT:
        return ':';
    case KEY_NEGATIVE:
        return ';';
    default:
        return 0;
    }
}


char keyToNum(char key)
{
    switch (key)
    {
    case KEY_0:
        return 0;
    case KEY_1:
        return 1;
    case KEY_2:
        return 2;
    case KEY_3:
        return 3;
    case KEY_4:
        return 4;
    case KEY_5:
        return 5;
    case KEY_6:
        return 6;
    case KEY_7:
        return 7;
    case KEY_8:
        return 8;
    case KEY_9:
        return 9;
    default:
        return -1;
    }
}

char toLowerCase(char ch)
{
    if((ch >= 'A') && (ch <= 'Z')){
        return ch + ' ';
    }

    return ch;
}

char toUpperCase(char ch)
{
    if((ch >= 'a') && (ch <= 'z')){
        return ch - ' ';
    }

    return ch;
}
