


#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "sys_llapi.h"
#include "llapi_code.h"

#include "console.h"
#include "SysConf.h"
#include "keyboard.h"

size_t xPortGetFreeHeapSize( void );
size_t xPortGetTotalHeapSize( void );

int coremain(void);

void testcpp();
char *giac_eval(char *s);

uint8_t *configAddr = (uint8_t *)0x023F0000;

void delayms(uint32_t ms)
{
    uint32_t startTime = *((volatile uint32_t *)0x8001C0C0);
    while((*((volatile uint32_t *)0x8001C0C0) - startTime) < (ms * 1000))
    {
        ;
    }
}


int f1(int x)
{
    if(x <= 1){
        return 1;
    }else{
        return x+f1(x-1);
    }
}

void libgiac_init()
{
    static bool libgiac_inited = false;

    if(libgiac_inited){
        return;
    }
    libgiac_inited = true;

    console_printf("LibGiac Initiating...");

    typedef void(*pfunc)();
    extern pfunc __ctors_start__[];
    extern pfunc __ctors_end__[];
    pfunc *p;

    uint32_t total, cur, percentage;
    total = ((uint32_t)__ctors_end__ - (uint32_t)__ctors_start__);


    for (p = __ctors_start__; p < __ctors_end__; p++)
    {
        cur = ((uint32_t)p) - ((uint32_t)__ctors_start__);

        percentage = (cur * 100) / total;
        if(percentage % 10 == 0)
            console_printf("Initiating: %d%%", percentage);

        (*p)();
    }

    console_printf("Initialized.");
        
}



#define ITEM_HISTORY    -1
#define ITEM_INPUTBOX   1

#define INPUT_MODE_SYM    0
#define INPUT_MODE_ALPHA  1
#define INPUT_MODE_alpha  2

#define CONSOLE_SHELL   0
#define CONSOLE_CAS     1

char *menu1[] = {"F1 ","F2 ","   ","COPY","   ","   "};
char *menu2[] = {"F1 ","F2 ","   ","    ","   ","   "};

bool shift_key = false;
bool alpha_key = false;

int input_mode = 0;
bool input_mode_hold = false;
bool inBusy = false;
bool canCopy = false;
int console_mode = 0;
int key_string = KEY_STRING_PYTHON;

uint8_t DispIndication = 0;
uint8_t BatteryPercentage = 100;

void flushKeyIndicate()
{
    uint8_t batInd = 0, Ind = 0;
    if(BatteryPercentage > 5){
        batInd |= 1;
    }
    if(BatteryPercentage > 25){
        batInd |= (1<<1);
    }
    if(BatteryPercentage > 50){
        batInd |= (1<<2);
    }
    if(BatteryPercentage > 75){
        batInd |= (1<<3);
    }
    if(shift_key){
        Ind |= INDICATE_LEFT;
    }

    if(inBusy){
        Ind |= INDICATE_BUSY;
    }

    if(input_mode == INPUT_MODE_ALPHA){
        Ind |= INDICATE_A__Z;
    }

    if(input_mode == INPUT_MODE_alpha){
        Ind |= INDICATE_a__z;
    }
    ll_DispSetIndicate(Ind, batInd);
}

void mainWindow(void *par){
    int currentSelect = 2;
    char linebuf[128];

    char key, press;

    console_put_block("Welcome to ExistOS Beta 0.0.2");
    console_put_block("[F1] Shell");
    console_put_block("[F2] CAS Mode");
    console_printf("free memory:%d/%d Bytes", xPortGetFreeHeapSize(), xPortGetTotalHeapSize());
    

                    setSelectBlock(getTotalBlock() + currentSelect);
                    refresh_block(false);
    //refresh_block(false);
    canCopy = false;
    refresh_menu(menu2, 0);
    
    for(;;)
    {
        if(getKey(&key, &press))
        {

            if(press == KEY_PRESS){
                switch (key)
                {
                case KEY_UP:
                    printf("currentSelect:%d\n",currentSelect);
                    currentSelect--;
                    
                    if(getTotalBlock() + currentSelect < 0){
                        currentSelect = -getTotalBlock();
                    }
                    canCopy = true;
                    refresh_menu(menu1, 0);
                    setSelectBlock(getTotalBlock() + currentSelect);
                    refresh_block(true);

                    break;
                
                case KEY_DOWN:
                    printf("currentSelect:%d\n",currentSelect);
                    currentSelect++;
                    setSelectBlock(getTotalBlock() + currentSelect);
                    refresh_block(true);
                    
                    if(currentSelect > 0){
                        setSelectBlock(-1);
                        refresh_block(false);
                        refresh_menu(menu2, 0);
                    }
                    if(currentSelect > ITEM_INPUTBOX){
                        currentSelect = ITEM_INPUTBOX;
                    }

                    break;
                case KEY_LEFT:
                    if(shift_key){
                        shiftIndicate(-100);
                    }else{
                        shiftIndicate(-1);
                    }
                    

                    break;
                case KEY_RIGHT:
                    if(shift_key){
                        shiftIndicate(100);
                    }else{
                        shiftIndicate(1);
                    }
                    break;

                case KEY_BACKSPACE:
                    backspace_in();
                    break;

                case KEY_PLOT:
                    ll_DispSendScreen();
                    break;

                case KEY_F4:
                    if(!canCopy){
                        break;
                    }else{
                        char *s = getSelectBlockStr();
                        if(s != NULL){
                            for(int i = 0; i< strlen(s); i++){
                                InputBox_in_ch(s[i]);
                            }
                        }
                        break;
                    }

                case KEY_F2:
                    libgiac_init();
                    setSelectBlock(-1);
                    console_put_block("Switch to CAS Mode.");
                    refresh_block(false);

                    console_mode = CONSOLE_CAS;
                    key_string = KEY_STRING_CAS;
                    break;
                case KEY_F1:
                    console_mode = CONSOLE_SHELL;
                    key_string = KEY_STRING_PYTHON;
                    
                    console_put_block("Switch to SHELL.");
                    refresh_block(false);
                    break;

                case KEY_ENTER:
                    input_mode = INPUT_MODE_SYM;
                    input_mode_hold = false;
                    alpha_key = false;
                    shift_key = false;
                    {
                        int res = enter_in();
                        char *s = console_get_block_string(res);
                        //if(s != NULL){
                        //    console_printf("%s", s);
                        //}
                        
                        switch (console_mode)
                        {
                            case CONSOLE_CAS:
                            {
                                char *ret;
                                char dummy[2];


                                inBusy = true;
                                flushKeyIndicate();

                                ret = giac_eval(s);

                                console_put_block_align(ret, ALIGN_RIGHT);
                                inBusy = false;
                                setSelectBlock(-1);
                                flushKeyIndicate();
                                refresh_block(false);
                                while(getKey(&dummy[0],&dummy[1]));
                                currentSelect = 2;
                            }
                            break;
                        
                        default:
                            break;
                        }

                    }
                    
                    break;

                case KEY_SHIFT:
                    if(shift_key){
                        shift_key = false;
                    }else{
                        shift_key = true;
                    }
                    flushKeyIndicate();
                    break;
                
                case KEY_ALPHA:

                    if(shift_key)
                    {
                        input_mode_hold = true;
                        switch (input_mode)
                        {
                        case INPUT_MODE_SYM:
                            input_mode = INPUT_MODE_ALPHA;
                            break;
                        case INPUT_MODE_ALPHA:
                            input_mode = INPUT_MODE_alpha;
                            break;
                        case INPUT_MODE_alpha:
                            input_mode = INPUT_MODE_ALPHA;
                            
                        default:
                            break;
                        }
                        break;
                    }else{
                        input_mode_hold = false;
                        switch (input_mode)
                        {
                        case INPUT_MODE_SYM:
                            input_mode = INPUT_MODE_ALPHA;
                            alpha_key = true;
                            break;
                        case INPUT_MODE_ALPHA:
                            input_mode = INPUT_MODE_alpha;
                            alpha_key = true;
                            break;
                        case INPUT_MODE_alpha:
                            input_mode = INPUT_MODE_SYM;
                            alpha_key = false;
                            break;
                            
                        default:
                            break;
                        }
                    }

                    flushKeyIndicate();

                    break;

                
                default:
                    {

                            switch (input_mode)
                            {
                                case INPUT_MODE_ALPHA:
                                    if(shift_key){
                                        char *s = keyTo_Sym_string(key, key_string);
                                        if(s != NULL){
                                            for(int i = 0; i < strlen(s); i++){
                                                InputBox_in_ch(s[i]);
                                            }
                                        }
                                    }else{
                                        InputBox_in_ch((keyTo_ALPHA(key)));
                                    }
                                    
                                    break;
                                case INPUT_MODE_alpha:
                                    if(shift_key){
                                        char *s = keyTo_Sym_string(key, key_string);
                                        if(s != NULL){
                                            for(int i = 0; i < strlen(s); i++){
                                                InputBox_in_ch(s[i]);
                                            }
                                        }
                                    }else{
                                        InputBox_in_ch(toLowerCase(keyTo_ALPHA(key)));
                                    }
                                    break;
                                case INPUT_MODE_SYM:
                                    {
                                        char *s = keyTo_Sym_string(key, key_string);
                                        if(s != NULL){
                                            for(int i = 0; i < strlen(s); i++){
                                                InputBox_in_ch(s[i]);
                                            }
                                        }
                                    }
                                    break;
                                default:
                                    break;
                                
                            }

                    }
                }

                
                if(key != KEY_SHIFT){
                    shift_key = false;
                }
                if((input_mode_hold == false) && (key != KEY_ALPHA)){
                    input_mode = INPUT_MODE_SYM;
                }
                flushKeyIndicate();
            }
        }

        delayms(5);

        //console_printf("free memory:%d Byte", xPortGetFreeHeapSize());
        
    }
}



void IRQ_ISR();



void main()
{
    
    //SYS STACK      0x023FA000
    //IRQ STACK      0x023FFFF0
    ll_set_configAddr((uint32_t)configAddr);
    ll_set_irq_stack (0x023FFFF0);
    ll_set_irq_vector((uint32_t)IRQ_ISR);

    configAddr[100] = 0xFE;


    printf("Start Test....\n");



    mainWindow(NULL);

    for(;;){

    }

}

