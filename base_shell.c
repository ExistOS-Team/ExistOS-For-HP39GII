
#include "ff.h"

#include "base_shell.h"
#include "ServiceFatFs.h"
#include "ServiceFlashMap.h"
#include "ServiceGraphic.h"
#include "ServiceRawFlash.h"
#include "ServiceSTMPPartition.h"
#include "ServiceKeyboard.h"
#include "ServiceUSBHID.h"
#include "display.h"
#include "keyboard.h"
#include "tusb.h"

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"

#define line_height 12
#define font_width 6
#define max_line_chars 256 / font_width - 1
unsigned int cur_x = 0;

unsigned char *screen_buffer;
TimerHandle_t Blink_timer;
// 0 ~ 9 (0~108)
char bufferline[max_line_chars];

void shell_put_a_line(char *text) {
    memcpy(
        &screen_buffer[0],
        &screen_buffer[(line_height * 1) * 258 + 0],
        (line_height * 9 - 3) * 258);
    memset(
        &screen_buffer[(line_height * 9 - 3) * 258 + 0],
        0,
        (line_height * 1) * 258);
    LCD_show_string(0, line_height * 8, 255, 12, 12, 255, text);
    LCD_dma_flush_buffer();
}

char last_ch;
void type_in_buffer_line(char ch) {
    last_ch = ch;
    if (ch) {
        bufferline[cur_x] = ch;
        LCD_show_char(cur_x * font_width, line_height * 9 + 3, ch, 12, 255, 0);
        LCD_dma_flush_buffer();
        if (cur_x < max_line_chars) {
            cur_x++;
        }
    } else {
        if (cur_x > 0) {
            bufferline[cur_x] = '\0';
            LCD_show_char(cur_x * font_width, line_height * 9 + 3, ' ', 12, 255, 0);
            LCD_dma_flush_buffer();
            cur_x--;
        }
    }
}

void clear_buffer_line() {
    memset(bufferline, 0, max_line_chars - 1);
    memset(
        &screen_buffer[(line_height * 10 - 2) * 258 + 0],
        0,
        (line_height + 3) * 258);
    LCD_dma_flush_buffer();
    cur_x = 0;
    LCD_show_string(cur_x * font_width, line_height * 9 + 3, 255, 12, 12, 255, "<");
}

unsigned int cur_blink_s = 0;
unsigned int continue_type_in = 0;
unsigned int continue_type_in_cnt = 0;
unsigned int divider_cnt = 0;
void blink_call_back() {
    divider_cnt++;
    if (divider_cnt % 5 == 0) {
        if (cur_blink_s) {
            LCD_show_string(cur_x * font_width, line_height * 9 + 3, 255, 12, 12, 255, "<");
            cur_blink_s = 0;
        } else {
            LCD_show_string(cur_x * font_width, line_height * 9 + 3, 255, 12, 12, 255, " ");
            cur_blink_s = 1;
        }
        LCD_dma_flush_buffer();
    }
    if (continue_type_in) {
        continue_type_in_cnt++;
    }
    if (continue_type_in_cnt > 5) {
        type_in_buffer_line(last_ch);
    }
}

BYTE work[FF_MAX_SS];
void parse_line(char *line) {
    char text_buf[64];

    if (strcmp(line, "usbon") == 0) {
        tud_connect();
        shell_put_a_line("USB Connected.");
        return;
    }
    if (strcmp(line, "usboff") == 0) {
        tud_disconnect();
        shell_put_a_line("USB Disconnected.");
        return;
    }
    if (strcmp(line, "usbreload") == 0) {
        tud_disconnect();
        vTaskDelay(1000);
        tud_connect();
        shell_put_a_line("USB Reload.");
        return;
    }

    if (strcmp(line, "hms") == 0) {
        for (size_t i = 0; i < 5; i++)
        {
          HID_mouse_move(10, 10, 0);
          vTaskDelay(300/portTICK_RATE_MS);
        }
        HID_mouse_click(MOUSE_BUTTON_LEFT);
        vTaskDelay(300 / portTICK_RATE_MS);
        HID_mouse_click(MOUSE_BUTTON_RIGHT);
        shell_put_a_line("okay");
        return;
    }

    if (strcmp(line, "hkb") == 0) {
      HID_kbd_print("Hello world!");
      shell_put_a_line("okay");
      return;
    }

      if (strcmp(line, "format") == 0) {
        shell_put_a_line("Start erasing...");
        extern unsigned int FSOK;
        FSOK = 0;
        shell_put_a_line("Lock disk...");
        lockFmap(true);
        flashMapClear();
        shell_put_a_line("Erasing...");
        for (int i = getDataRegonStartBlock(); i < getDataRegonStartBlock() + getDataRegonTotalBlocks(); i++) {
                xEraseFlashBlocks(i, 1, 5000);
                vTaskDelay(1);
        }
        shell_put_a_line("Reset Mapping...");
        flashMapReset();
        MKFS_PARM opt;
        opt.fmt = FM_FAT;
        opt.au_size = 2048;
        opt.align = 2048;
        opt.n_fat = 2;
        FATFS fs;
        int fr = f_mkfs("", &opt, work, sizeof(work));
        sprintf(text_buf,"Format result:%d\n", fr);
        shell_put_a_line(text_buf);
        shell_put_a_line("Mounting...");
        f_mount(&fs, "/", 1);
        lockFmap(false);
        FSOK = 1;
        vTaskDelay(2000);
        flashSyncNow();
        shell_put_a_line("Format finish...");
        return;
    }

    if (strcmp(line, "menu") == 0) {
        reset_shell();
        return;
    }

    if (strcmp(line, "help") == 0) {
        shell_put_a_line("commane list:");
        shell_put_a_line("menu    help    format    usbon    usboff");
        shell_put_a_line("usbreload hms hkb");
        return;
    }

    shell_put_a_line("command not found. type 'help' to check the");
    shell_put_a_line("commane list.");
    }

void key_input(unsigned int key) {
    unsigned char to_alpha;
    unsigned char to_letter;
    if (key & 1) {
        continue_type_in = 1;

        to_alpha = key_map_to_alpha(key >> 8);
        if (to_alpha) {
            if ((to_alpha >= 'A') && (to_alpha <= 'Z')) {
                to_letter = to_alpha + ('a' - 'A');
                type_in_buffer_line(to_letter);
            }
        } else {
            switch (key >> 8) 
            {
            case KEY_BACKSPACE:
                type_in_buffer_line(0);
                break;
            case KEY_ENTER:
                continue_type_in = 0;
                continue_type_in_cnt = 0;
                shell_put_a_line(bufferline);
                if(cur_x != 0)
                {
                    parse_line(bufferline);
                }
                clear_buffer_line();

                break;
            default:
                    continue_type_in = 0;
                    continue_type_in_cnt = 0;
                break;
            }
        }

    } else {

        continue_type_in = 0;
        continue_type_in_cnt = 0;
    }
    //printf("g:%c\n",key_map_to_alpha(key >> 8));
}

void reset_shell() {

    memset(bufferline, 0, max_line_chars);
    LCD_clear_buffer();
    LCD_dma_flush_buffer();
    LCD_show_string(0, line_height * 6, 255, 24, 24, 255, "Exist OS");
    LCD_show_string(0, line_height * 8, 255, 12, 12, 255, "Exist OS Basic Shell v0.1");
    for (int x = 0; x < 256; x++) {
        screen_buffer[(line_height * 10 - 3) * 258 + x] = 0xFF;
    }
    LCD_dma_flush_buffer();

    if (!isfatFsInited()) {
        shell_put_a_line("The disk is not formatted.");
        shell_put_a_line("Type 'format' to format the disk.");
    }
}

void base_shell_main() {
    screen_buffer = (unsigned char *)getVramAddress();

    Blink_timer = xTimerCreate(
        "Shell Blink",
        100,
        pdTRUE,
        (void *)0,
        blink_call_back);
    if (Blink_timer != NULL) {
        xTimerStart(Blink_timer, 0);
    }

    register_keyboard_callback(key_input);

    reset_shell();

    vTaskSuspend(NULL);
}