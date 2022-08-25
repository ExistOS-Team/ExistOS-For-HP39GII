

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>

#include "keyboard.h"
#include "llapi_code.h"
#include "sys_llapi.h"

#include "SysConf.h"

#include "FreeRTOS.h"
#include "task.h"

//#include "ff.h"

#include "lv_conf.h"
#define LV_CONF_INCLUDE_SIMPLE
#include "lvgl.h"

#include "debug.h"

#include "SystemUI.h"
#include "SystemFs.h"

#include "lv_demo_keypad_encoder.h"

#include "Fatfs/ff.h"
//#include "mpy_port.h"

volatile unsigned long ulHighFrequencyTimerTicks;




char pcWriteBuffer[4096];
void printTaskList() {
    vTaskList((char *)&pcWriteBuffer);
    printf("=============SYSTEM STATUS=================\r\n");
    printf("Task Name         Task Status   Priority   Stack   ID\n");
    printf("%s\n", pcWriteBuffer);
    printf("Task Name                Running Count         CPU %%\n");
    vTaskGetRunTimeStats((char *)&pcWriteBuffer);
    printf("%s\n", pcWriteBuffer);
    printf("Status:  X-Running  R-Ready  B-Block  S-Suspend  D-Delete\n");
    printf("Free memory:   %d Bytes\n", (unsigned int)xPortGetFreeHeapSize());
}

void vTask1(void *par1) {
    while (1) {
        printTaskList();
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void softDelayMs(uint32_t ms)
{
    uint32_t cur = ll_get_time_ms();
    while( (ll_get_time_ms() - cur) < ms )
    {
        ;
    }
}



void vTask2(void *par1) {

    //__asm volatile("sub R13,R13,#4");

    uint32_t ticks = 0;

    

    while (1) {
/*
        static float f1 = 0.1;
        static float f2 = 0.2;
        f1 += 0.01;
        f1 /= 1.001;
        f2 = f2 + f1;
        printf("test:%f,%f\n", (f1 * 10), (f2 * 10));
        printf("R13:%08x\n", get_stack());*/

        printf("SYS Run Time: %d s\n", ticks);
        ticks++;

        vTaskDelay(pdMS_TO_TICKS(1000));
        
    }
}


void vApplicationIdleHook( void )
{
    ll_system_idle();

}

void emu48Btn(lv_event_t *e);
void khicasBtn(lv_event_t *e);
static bool suspend = false;
static lv_obj_t *screen;
static lv_obj_t *win;
void draw_main_win()
{




    screen = lv_obj_create(lv_scr_act());

    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

	static lv_style_t style_screen_main_main_default;
	if (style_screen_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_main_main_default);
	else
		lv_style_init(&style_screen_main_main_default);
	lv_style_set_bg_color(&style_screen_main_main_default, lv_color_black());
	lv_style_set_bg_opa(&style_screen_main_main_default, LV_OPA_40);
	lv_obj_add_style(screen, &style_screen_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    lv_obj_set_x(screen, 0);
    lv_obj_set_y(screen, 0);
    lv_obj_set_size(screen, 256, 128);

    win = lv_win_create(lv_scr_act(), 13);
	lv_win_add_title(win, "Exist OS");
	lv_obj_set_pos(win, 2, 2);
	lv_obj_set_size(win, 252, 125);

	static lv_style_t style_screen_win_1_main_main_default;
	if (style_screen_win_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_win_1_main_main_default);
	else
		lv_style_init(&style_screen_win_1_main_main_default);
	lv_style_set_bg_color(&style_screen_win_1_main_main_default, lv_color_make(0xee, 0xee, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_win_1_main_main_default, lv_color_make(0xee, 0xee, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_win_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_win_1_main_main_default, 255);
	lv_style_set_outline_color(&style_screen_win_1_main_main_default, lv_color_make(0x08, 0x1A, 0x0F));
	lv_style_set_outline_width(&style_screen_win_1_main_main_default, 1);
	lv_style_set_outline_opa(&style_screen_win_1_main_main_default, 255);
	lv_obj_add_style(win, &style_screen_win_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);


	//Write style state: LV_STATE_DEFAULT for style_screen_win_1_extra_content_main_default
	static lv_style_t style_screen_win_1_extra_content_main_default;
	if (style_screen_win_1_extra_content_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_win_1_extra_content_main_default);
	else
		lv_style_init(&style_screen_win_1_extra_content_main_default);
	lv_style_set_bg_color(&style_screen_win_1_extra_content_main_default, lv_color_make(0xee, 0xee, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_win_1_extra_content_main_default, lv_color_make(0xee, 0xee, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_win_1_extra_content_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_win_1_extra_content_main_default, 255);
	lv_style_set_text_color(&style_screen_win_1_extra_content_main_default, lv_color_make(0x39, 0x3c, 0x41));
	lv_style_set_text_font(&style_screen_win_1_extra_content_main_default, &lv_font_montserrat_12);
	lv_style_set_text_letter_space(&style_screen_win_1_extra_content_main_default, 0);
	lv_style_set_text_line_space(&style_screen_win_1_extra_content_main_default, 2);
	lv_obj_add_style(lv_win_get_content(win), &style_screen_win_1_extra_content_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_win_1_extra_header_main_default
	static lv_style_t style_screen_win_1_extra_header_main_default;
	if (style_screen_win_1_extra_header_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_win_1_extra_header_main_default);
	else
		lv_style_init(&style_screen_win_1_extra_header_main_default);

	lv_style_set_bg_color(&style_screen_win_1_extra_header_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_grad_color(&style_screen_win_1_extra_header_main_default, lv_color_make(0xff, 0xff, 0xff));

    lv_style_set_bg_main_stop( &style_screen_win_1_extra_header_main_default, 0 );
    lv_style_set_bg_grad_stop( &style_screen_win_1_extra_header_main_default, 255 );

	lv_style_set_bg_grad_dir(&style_screen_win_1_extra_header_main_default, LV_GRAD_DIR_HOR);
	lv_style_set_bg_opa(&style_screen_win_1_extra_header_main_default, 255);
	lv_style_set_text_color(&style_screen_win_1_extra_header_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_screen_win_1_extra_header_main_default, &lv_font_montserrat_12);
	lv_style_set_text_letter_space(&style_screen_win_1_extra_header_main_default, 0);
	lv_style_set_text_line_space(&style_screen_win_1_extra_header_main_default, 2);
	lv_obj_add_style(lv_win_get_header(win), &style_screen_win_1_extra_header_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_DEFAULT for style_screen_win_1_extra_btns_main_default
	static lv_style_t style_screen_win_1_extra_btns_main_default;
	if (style_screen_win_1_extra_btns_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_win_1_extra_btns_main_default);
	else
		lv_style_init(&style_screen_win_1_extra_btns_main_default);
	lv_style_set_radius(&style_screen_win_1_extra_btns_main_default, 8);
	lv_style_set_bg_color(&style_screen_win_1_extra_btns_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_win_1_extra_btns_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_win_1_extra_btns_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_win_1_extra_btns_main_default, 255);
	lv_obj_t *screen_win_1_btn;
	lv_obj_t *screen_win_1_label = lv_label_create(lv_win_get_content(win));
	lv_label_set_text(screen_win_1_label, "");







/*
    obj = lv_textarea_create(lv_scr_act());
    lv_textarea_add_text(obj, "字体：思源黑体 Light\n 字号：11\n");
    lv_textarea_add_text(obj, te);
    lv_textarea_set_align(obj, LV_TEXT_ALIGN_LEFT);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLL_WITH_ARROW);
*/ 

    //char* argv[] = {"gb", "test.gb", NULL};
    //int argc = sizeof(argv) / sizeof(argv[0]) - 1;
    //extern int gb_main(int argc, char *argv[]);
    //gb_main(argc, argv);
    
    //SystemTest(); 
 
    //lv_scr_act();

    //void mpy_main();
      
    //mpy_main();


}

static lv_group_t *g;
static lv_obj_t *charge_chb, *slow_down_chb;

static void charge_msgbox_event_cb(lv_event_t *e) 
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);
    

    if (code == LV_EVENT_VALUE_CHANGED) {
        uint16_t select_btn = lv_msgbox_get_active_btn(msgbox); // 0 or 1

        if(select_btn == 0){
            ll_charge_enable(true);

        }else{
            ll_charge_enable(false);
            lv_obj_clear_state(charge_chb, LV_STATE_CHECKED);
        }

        
        lv_msgbox_close(msgbox);
        lv_group_focus_freeze(g, false);
    }

}

static void slowdown_chb_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_VALUE_CHANGED) {
        //const char * txt = lv_checkbox_get_text(obj);

        if(lv_obj_get_state(obj) & LV_STATE_CHECKED)
        {
            ll_cpu_slowdown_enable(true);
        }else{
            ll_cpu_slowdown_enable(false);
        }
    }

}

static void charge_chb_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);

    static char *msgbox_button[] = {"OK", "Cancel", ""};

    if(code == LV_EVENT_VALUE_CHANGED) {
        //const char * txt = lv_checkbox_get_text(obj);

        if(lv_obj_get_state(obj) & LV_STATE_CHECKED)
        {

            lv_obj_t *mbox = lv_msgbox_create( lv_scr_act(), "!!! Warning !!!", "[Experimental Features] PLEASE make sure use \n1.2 V Rechargeable Battery, i.e. NiCd, NiMH, etc.", (const char **)msgbox_button, false);

            lv_obj_add_event_cb(mbox, charge_msgbox_event_cb, LV_EVENT_ALL, NULL);
            lv_obj_align(mbox, LV_ALIGN_CENTER, 0, 0);
            lv_obj_center(mbox);
            
            lv_group_focus_next(g);
            lv_group_focus_freeze(g, true);
        }else{
            ll_charge_enable(false);
        }

        //const char * state = lv_obj_get_state(obj) & LV_STATE_CHECKED ? "Checked" : "Unchecked";
        //LV_LOG_USER("%s: %s", txt, state);
    }
}

lv_obj_t* label_cpuminirac;

void slider_cpu_minimum_frac_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    char buf[32];
    int val = (int)lv_slider_get_value(slider);
    lv_snprintf(buf, sizeof(buf), "CPU Freq Minimum Frac: %d", val);
    lv_label_set_text(label_cpuminirac, buf);

    ll_cpu_slowdown_min_frac(val);
}


lv_obj_t *title, *tv, *t1, *t2, *imgbtn, *imgbtn2;

void main_thread() {


    //printf("R13:%08x\n", get_stack());

    SystemUIInit();
    SystemFSInit();


/*
    SystemUISuspend();
    ll_cpu_slowdown_enable(false);
    uarmLinuxMain();
*/

/*
    void emu48_main(int select);
    SystemUISuspend();
    emu48_main(2);
*/
/*
    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }*/
    //vTaskDelay(pdMS_TO_TICKS(1000));
    
    
    // lv_demo_benchmark();
    //  lv_demo_stress();
    //  lv_demo_music();
    //  lv_demo_widgets();


    //SystemUIMsgBox("测试?", "Unicode测试", SYSTEMUI_MSGBOX_BUTTON_CANCAL);
    //SystemUIMsgBox("测试?", "Unicode测试", SYSTEMUI_MSGBOX_BUTTON_CANCAL);

    draw_main_win();


    lv_obj_t * cont = lv_scr_act();

    //title = lv_label_create(cont);
    //lv_label_set_text(title, "Exist OS");

    static lv_style_t style;
    {
    lv_style_init(&style);

    tv = lv_tabview_create(cont, LV_DIR_TOP, LV_DPI_DEF / 4);
	lv_obj_set_pos(tv, 4, 15);
	lv_obj_set_size(tv, 248, 109);


    t1 = lv_tabview_add_tab(tv, "Application");
    t2 = lv_tabview_add_tab(tv, "Status");

    LV_IMG_DECLARE(xcaslogo_s);
    imgbtn = lv_imgbtn_create(t1);

	lv_obj_set_scrollbar_mode(imgbtn, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_size(imgbtn, 48, 48);
    lv_obj_set_pos(imgbtn, 12, 2);

	static lv_style_t style_screen_imgbtn_1_main_main_default;
	if (style_screen_imgbtn_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_imgbtn_1_main_main_default);
	else
		lv_style_init(&style_screen_imgbtn_1_main_main_default);
	lv_style_set_shadow_width(&style_screen_imgbtn_1_main_main_default, 2);
	lv_style_set_shadow_color(&style_screen_imgbtn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_shadow_opa(&style_screen_imgbtn_1_main_main_default, 128);
	lv_style_set_shadow_spread(&style_screen_imgbtn_1_main_main_default, 2);
	lv_style_set_shadow_ofs_x(&style_screen_imgbtn_1_main_main_default, 0);
	lv_style_set_shadow_ofs_y(&style_screen_imgbtn_1_main_main_default, 0);
	lv_style_set_text_color(&style_screen_imgbtn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_align(&style_screen_imgbtn_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_style_set_img_recolor(&style_screen_imgbtn_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_imgbtn_1_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_imgbtn_1_main_main_default, 255);
	lv_obj_add_style(imgbtn, &style_screen_imgbtn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

	//Write style state: LV_STATE_PRESSED for style_screen_imgbtn_1_main_main_pressed
    /*
	static lv_style_t style_screen_imgbtn_1_main_main_pressed;
	if (style_screen_imgbtn_1_main_main_pressed.prop_cnt > 1)
		lv_style_reset(&style_screen_imgbtn_1_main_main_pressed);
	else
		lv_style_init(&style_screen_imgbtn_1_main_main_pressed);
	lv_style_set_text_color(&style_screen_imgbtn_1_main_main_pressed, lv_color_make(0xFF, 0x33, 0xFF));
	lv_style_set_text_align(&style_screen_imgbtn_1_main_main_pressed, LV_TEXT_ALIGN_CENTER);
	lv_style_set_img_recolor(&style_screen_imgbtn_1_main_main_pressed, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_img_recolor_opa(&style_screen_imgbtn_1_main_main_pressed, 0);
	lv_style_set_img_opa(&style_screen_imgbtn_1_main_main_pressed, 255);
	lv_obj_add_style(imgbtn, &style_screen_imgbtn_1_main_main_pressed, LV_PART_MAIN|LV_STATE_PRESSED);
*/
/*
	//Write style state: LV_STATE_CHECKED for style_screen_imgbtn_1_main_main_checked
	static lv_style_t style_screen_imgbtn_1_main_main_checked;
	if (style_screen_imgbtn_1_main_main_checked.prop_cnt > 1)
		lv_style_reset(&style_screen_imgbtn_1_main_main_checked);
	else
		lv_style_init(&style_screen_imgbtn_1_main_main_checked);
	lv_style_set_text_color(&style_screen_imgbtn_1_main_main_checked, lv_color_make(0xFF, 0x33, 0xFF));
	lv_style_set_text_align(&style_screen_imgbtn_1_main_main_checked, LV_TEXT_ALIGN_CENTER);
	lv_style_set_img_recolor(&style_screen_imgbtn_1_main_main_checked, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_img_recolor_opa(&style_screen_imgbtn_1_main_main_checked, 0);
	lv_style_set_img_opa(&style_screen_imgbtn_1_main_main_checked, 255);
	lv_obj_add_style(imgbtn, &style_screen_imgbtn_1_main_main_checked, LV_PART_MAIN|LV_STATE_CHECKED);
*/

    
    LV_IMG_DECLARE(emu48ico);

	lv_imgbtn_set_src(imgbtn, LV_IMGBTN_STATE_RELEASED, NULL, &xcaslogo_s, NULL);
	lv_obj_add_flag(imgbtn, LV_OBJ_FLAG_CHECKABLE);


    lv_obj_t *btn1;
    btn1 = lv_btn_create(t1);
    lv_obj_set_size(btn1, 48, 13);
    lv_obj_set_pos(btn1, 12, 52);
    lv_obj_t * label = lv_label_create(btn1);


    lv_obj_add_event_cb(btn1, khicasBtn, LV_EVENT_ALL, NULL);


	static lv_style_t style_screen_btn_1_main_main_default;
	if (style_screen_btn_1_main_main_default.prop_cnt > 1)
		lv_style_reset(&style_screen_btn_1_main_main_default);
	else
		lv_style_init(&style_screen_btn_1_main_main_default);
	lv_style_set_radius(&style_screen_btn_1_main_main_default, 2);
	lv_style_set_bg_color(&style_screen_btn_1_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_bg_grad_color(&style_screen_btn_1_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_btn_1_main_main_default, LV_GRAD_DIR_NONE);
	lv_style_set_bg_opa(&style_screen_btn_1_main_main_default, 255);
	lv_style_set_border_color(&style_screen_btn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_border_width(&style_screen_btn_1_main_main_default, 1);
	lv_style_set_border_opa(&style_screen_btn_1_main_main_default, 100);
	lv_style_set_text_color(&style_screen_btn_1_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_text_font(&style_screen_btn_1_main_main_default, &lv_font_montserrat_12);
	lv_style_set_text_align(&style_screen_btn_1_main_main_default, LV_TEXT_ALIGN_CENTER);
	lv_obj_add_style(btn1, &style_screen_btn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	

	lv_label_set_text(label, "KhiCAS");
	lv_obj_set_style_pad_all(btn1, 0, LV_STATE_DEFAULT);
	lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);



    imgbtn2 = lv_imgbtn_create(t1);

	lv_obj_set_scrollbar_mode(imgbtn2, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_size(imgbtn2, 48, 48);
    lv_obj_set_pos(imgbtn2, 48 + 45, 2);

    
	lv_obj_add_style(imgbtn2, &style_screen_imgbtn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
	lv_imgbtn_set_src(imgbtn2, LV_IMGBTN_STATE_RELEASED, NULL, &emu48ico, NULL);
	lv_obj_add_flag(imgbtn2, LV_OBJ_FLAG_CHECKABLE);



    lv_obj_t *btn2;
    btn2 = lv_btn_create(t1);
    lv_obj_set_size(btn2, 48, 13);
    lv_obj_set_pos(btn2, 12 + 40*2 , 52);
    lv_obj_t * label2 = lv_label_create(btn2);
	lv_label_set_text(label2, "Emu48");
	lv_obj_set_style_pad_all(btn2, 0, LV_STATE_DEFAULT);
	lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
	lv_obj_add_style(btn2, &style_screen_btn_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);

    
    lv_obj_add_event_cb(btn2, emu48Btn, LV_EVENT_ALL, NULL);
    }

    //lv_obj_clear_flag(btn1, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    


//===============================================================
//tv2
    //lv_obj_set_scrollbar_mode(t2, LV_SCROLLBAR_MODE_ON);


    lv_obj_set_flex_flow(t2, LV_FLEX_FLOW_COLUMN);
    //

    
    g = lv_group_get_default();

    #define DEF_INFO_LABEL(label_name, fcous_able) lv_obj_t* label_name; label_name = lv_label_create(t2); \
    lv_obj_set_flex_grow(label_name, 0); \
    if(fcous_able){lv_group_add_obj(g, label_name); \
    lv_obj_add_flag(label_name, LV_OBJ_FLAG_SCROLL_ON_FOCUS);}
    #define SET_LABEL_TEXT(label, ...) lv_label_set_text_fmt(label, __VA_ARGS__)


    DEF_INFO_LABEL(info_line1, true);
    DEF_INFO_LABEL(info_line2, false);
    DEF_INFO_LABEL(info_line3, false);
    DEF_INFO_LABEL(info_line4, false);


    
    label_cpuminirac = lv_label_create(t2);
    SET_LABEL_TEXT(label_cpuminirac, "CPU Freq Minimum Frac: 12");
    lv_obj_set_flex_grow(label_cpuminirac, 0);

    lv_obj_t *cpu_minpwr_slider = lv_slider_create(t2);
    lv_obj_set_flex_grow(cpu_minpwr_slider, 0);
    lv_slider_set_range(cpu_minpwr_slider, 2, 14);
    lv_obj_add_flag(cpu_minpwr_slider, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_slider_set_value(cpu_minpwr_slider, 12, false);
    lv_obj_add_event_cb(cpu_minpwr_slider, slider_cpu_minimum_frac_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    slow_down_chb = lv_checkbox_create(t2);
    lv_checkbox_set_text(slow_down_chb, "CPU Auto Slow-Down");
    lv_obj_set_flex_grow(slow_down_chb, 0); 
    lv_obj_add_event_cb(slow_down_chb, slowdown_chb_handler, LV_EVENT_ALL, NULL);
    lv_obj_add_state(slow_down_chb, LV_STATE_CHECKED);

    charge_chb = lv_checkbox_create(t2);
    lv_checkbox_set_text(charge_chb, "Enable Charge");
    lv_obj_set_flex_grow(charge_chb, 0);
    lv_obj_add_event_cb(charge_chb, charge_chb_handler, LV_EVENT_ALL, NULL);



    //lv_demo_keypad_encoder(); 
    int cur_cpu_div = 1;
    int cur_cpu_frac = 25;
    int cur_hclk_div = 2;
    unsigned int runTime = 0;

    uint32_t tmp[3];
    uint32_t cur_fcpu;
    uint32_t cur_batt_volt;
    uint32_t cur_soc_temp;

    FIL *f = pvPortMalloc(sizeof(FIL));

    for (;;) {
        

        runTime ++;

        if(!suspend){

            

            
            ll_get_clkctrl_div(tmp);
            cur_cpu_div = tmp[0];
            cur_cpu_frac = tmp[1];
            cur_hclk_div = tmp[2];


            cur_fcpu = ll_get_cur_freq();
            cur_batt_volt = ll_get_bat_voltage();
            cur_soc_temp = ll_get_core_temp();

            SET_LABEL_TEXT(info_line1, "CPU Freq:%d / %d MHz,  Temp:%d °C", cur_fcpu , 480 * 18 / cur_cpu_div / cur_cpu_frac, cur_soc_temp );
            SET_LABEL_TEXT(info_line2, "Mem: %d / %d KB,  Ticks:%d s", xPortGetFreeHeapSize() / 1024, 8192, runTime );
            SET_LABEL_TEXT(info_line3, "Batt: %d mv,  Charging: %s", cur_batt_volt, ll_get_charge_status() ? "Yes" : "NO" );
            SET_LABEL_TEXT(info_line4, "Pwr Speed: %d Ticks", ll_get_pwrspeed() );

            //printf("f=%d,v=%d,t=%d\r\n", cur_fcpu, cur_batt_volt, cur_soc_temp);
            /*
            if(runTime % 30 == 0)
            {

                
                if(f)
                {
                    f_open(f, "pwr_infolog.txt", FA_OPEN_ALWAYS | FA_OPEN_APPEND | FA_WRITE);
                    f_printf(f, "%d\t%d\t%d\t%d\r\n", runTime, cur_fcpu, cur_batt_volt, cur_soc_temp);
                    f_sync(f);
                    f_close(f);
                    
                }
                
            }*/


/*
            SET_LABEL_TEXT(lab_cpu_freq, "CPU Freq:%d/%d MHz", ll_get_cur_freq() , 480 * 18 / cur_cpu_div / cur_cpu_frac);
            lv_label_set_text_fmt(lab_runtime, "RunTime:%d s", runTime);
            lv_label_set_text_fmt(lab_free_mem, "Free:%d KB", xPortGetFreeHeapSize() / 1024);
            lv_label_set_text_fmt(lab_voltage, "Batt:%d mV", ll_get_bat_voltage());
            lv_label_set_text_fmt(lab_cpu_temp, "Core: %d °C" , ll_get_core_temp());*/
        }

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void khicasTask(void *arg)
{
    lv_obj_t *win = lv_win_create(lv_scr_act(), 0);
    lv_obj_t *cont = lv_win_get_content(win);

    lv_obj_t *text = lv_label_create(cont);

    lv_label_set_text(text, "Loading...");
    vTaskDelay(pdMS_TO_TICKS(1000));


    SystemUISuspend();
    void testcpp();
    testcpp(); 

    SystemUIResume();
    vTaskDelete(NULL);
}

void khicasBtn(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {
        xTaskCreate(khicasTask, "KhiCAS", 16384, NULL, configMAX_PRIORITIES - 3, NULL);

    }
}
static lv_obj_t *emu48_msgbox ;


static void emu48_msgbox_event_cb(lv_event_t *e) {
    
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *msgbox = lv_event_get_current_target(e);

    if (code == LV_EVENT_VALUE_CHANGED) {
        
        lv_msgbox_close(msgbox);
        lv_group_focus_freeze(g, false);
    }
}


const char *msgboxbtn[] = {""};


void emu48_preThread(void *sel)
{
    if(sel == 0){
        emu48_msgbox = lv_msgbox_create(lv_scr_act(), "Error", "Cound not fine the ROM: /rom.39g", msgboxbtn , false);
        lv_obj_add_event_cb(emu48_msgbox, emu48_msgbox_event_cb, LV_EVENT_ALL, 0);
        lv_obj_align(emu48_msgbox, LV_ALIGN_CENTER, 0, 0);
        lv_obj_center(emu48_msgbox);

        lv_group_focus_freeze(g, true);
        
        vTaskDelay(pdMS_TO_TICKS(2500));

        lv_msgbox_close(emu48_msgbox);

        lv_group_focus_freeze(g, false);
        vTaskDelete(NULL);
    }


    lv_obj_t *win = lv_win_create(lv_scr_act(), 0);
    lv_obj_t *cont = lv_win_get_content(win);
    lv_obj_t *text = lv_label_create(cont);
    lv_label_set_text(text, "Loading...");
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    void emu48_main(int select);
    SystemUISuspend();

    emu48_main((int)sel);

    for(;;)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }
    
}

void emu48Btn(lv_event_t *e)
{
    int romf = -1;
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) {
        FIL *f = pvPortMalloc(sizeof(FIL));
        FRESULT fr;

/*
        fr = f_open(f, "/rom.39g.unpack", FA_OPEN_EXISTING);
        if(fr == FR_OK)
        {
            romf = 2;
            goto fopen_testok;
        }
*/
        fr = f_open(f, "/rom.39g", FA_OPEN_EXISTING);
        if(fr == FR_OK)
        {
            romf = 1;
            goto fopen_testok;
        }
        xTaskCreate(emu48_preThread, "emu48", configMINIMAL_STACK_SIZE, (void *)0, configMAX_PRIORITIES - 3, NULL);


        return;

        fopen_testok:
        f_close(f);
        vPortFree(f);

        xTaskCreate(emu48_preThread, "emu48", configMINIMAL_STACK_SIZE, (void *)romf, configMAX_PRIORITIES - 3, NULL);

    }
}
 
extern int __HEAP_START[384*1024 / 4];

void main() { 

    // SYS STACK      0x023FA000
    // IRQ STACK      0x023FFFF0
    void IRQ_ISR();
    void SWI_ISR();
    ll_set_irq_stack(IRQ_STACK_ADDR);
    ll_set_irq_vector(((uint32_t)IRQ_ISR) + 4); 
    ll_set_svc_stack(SWI_STACK_ADDR);
    ll_set_svc_vector(((uint32_t)SWI_ISR) + 4);
    ll_enable_irq(false);
    // ll_set_keyboard(true);  
    ll_cpu_slowdown_enable(false);

    //memset(&__HEAP_START[0], 0xFF, 384 * 1024);
  
    printf("System Booting...\n");

    xTaskCreate(vTask1, "Task1", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
    xTaskCreate(vTask2, "Task2", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);

    xTaskCreate(main_thread, "System", configMINIMAL_STACK_SIZE, NULL, configMAX_PRIORITIES - 3, NULL);

    ll_cpu_slowdown_enable(true);

    vTaskStartScheduler();

    for (;;) {
        *((double *) 0x45678901) = 114514.1919810f;
    }
} 

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName) {
    PANNIC("StackOverflowHook:%s\n", pcTaskName);
}

void vAssertCalled(char *file, int line) {
    PANNIC("ASSERT FAILED AT %s:%d\n", file, line);
}

void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    *ppxTimerTaskTCBBuffer = (StaticTask_t *)pvPortMalloc(sizeof(StaticTask_t));
    *ppxTimerTaskStackBuffer = (StackType_t *)pvPortMalloc(configMINIMAL_STACK_SIZE * 4);
    *pulTimerTaskStackSize = configMINIMAL_STACK_SIZE;
}

void vApplicationMallocFailedHook() {
    PANNIC("ASSERT: Out of Memory.\n");
}
