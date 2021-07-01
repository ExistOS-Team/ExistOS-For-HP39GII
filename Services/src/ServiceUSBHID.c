#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "tusb.h"

/* System serive includes. */
#include "ServiceUSBHID.h"
#include "ServiceKeyboard.h"

/* Library includes. */
#include "display.h"
#include "keyboard.h"
#include "irq.h"
#include "regsuartdbg.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t itf, uint8_t report_id,
                               hid_report_type_t report_type, uint8_t *buffer,
                               uint16_t reqlen) {
  // TODO not Implemented
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)reqlen;

  return 0;
}

void tud_hid_set_report_cb(uint8_t itf, uint8_t report_id,
                           hid_report_type_t report_type, uint8_t const *buffer,
                           uint16_t bufsize) {
  // TODO set LED based on CAPLOCK, NUMLOCK etc...
  (void)report_id;
  (void)report_type;
  (void)buffer;
  (void)bufsize;
}

static uint8_t _buttons = 0;
static void buttons(uint8_t b){
  if (b != _buttons) {
    _buttons = b;
    HID_mouse_move(0, 0, 0);
  }
}

void HID_mouse_click(uint8_t btn) {
  _buttons = btn;
  HID_mouse_move(0, 0, 0);
  _buttons = 0;
  HID_mouse_move(0, 0, 0);
}

void HID_mouse_move(signed char x, signed char y, signed char wheel){
  if(tud_suspended()){
    tud_remote_wakeup();
  }
  while(!tud_hid_n_ready(1)) vTaskDelay(10/portTICK_RATE_MS);
  tud_hid_n_mouse_report(1,REPORT_ID_MOUSE,_buttons,x,y,wheel,0);
}

void HID_mouse_press(uint8_t btn) { buttons(_buttons | btn); }
void HID_mouse_release(uint8_t btn) { buttons(_buttons & ~btn); }
bool HID_mouse_isPressed(uint8_t btn){
  if ((btn & _buttons) > 0)
    return true;
  return false;
}

static KeyReport _keyReport;
static void sendReport(KeyReport *keys) {
  if (tud_suspended()) {
    tud_remote_wakeup();
  }
  while (tud_hid_n_ready(0)) vTaskDelay(10/portTICK_RATE_MS);
  tud_hid_n_keyboard_report(0,REPORT_ID_KEYBOARD, keys->modifiers, keys->keys);
  vTaskDelay(2/portTICK_RATE_MS);
}
#define SHIFT 0x80
const uint8_t _asciimap[128] = {
    0x00, // NUL
    0x00, // SOH
    0x00, // STX
    0x00, // ETX
    0x00, // EOT
    0x00, // ENQ
    0x00, // ACK
    0x00, // BEL
    0x2a, // BS Backspace
    0x2b, // TAB  Tab
    0x28, // LF Enter
    0x00, // VT
    0x00, // FF
    0x00, // CR
    0x00, // SO
    0x00, // SI
    0x00, // DEL
    0x00, // DC1
    0x00, // DC2
    0x00, // DC3
    0x00, // DC4
    0x00, // NAK
    0x00, // SYN
    0x00, // ETB
    0x00, // CAN
    0x00, // EM
    0x00, // SUB
    0x00, // ESC
    0x00, // FS
    0x00, // GS
    0x00, // RS
    0x00, // US

    0x2c,         //  ' '
    0x1e | SHIFT, // !
    0x34 | SHIFT, // "
    0x20 | SHIFT, // #
    0x21 | SHIFT, // $
    0x22 | SHIFT, // %
    0x24 | SHIFT, // &
    0x34,         // '
    0x26 | SHIFT, // (
    0x27 | SHIFT, // )
    0x25 | SHIFT, // *
    0x2e | SHIFT, // +
    0x36,         // ,
    0x2d,         // -
    0x37,         // .
    0x38,         // /
    0x27,         // 0
    0x1e,         // 1
    0x1f,         // 2
    0x20,         // 3
    0x21,         // 4
    0x22,         // 5
    0x23,         // 6
    0x24,         // 7
    0x25,         // 8
    0x26,         // 9
    0x33 | SHIFT, // :
    0x33,         // ;
    0x36 | SHIFT, // <
    0x2e,         // =
    0x37 | SHIFT, // >
    0x38 | SHIFT, // ?
    0x1f | SHIFT, // @
    0x04 | SHIFT, // A
    0x05 | SHIFT, // B
    0x06 | SHIFT, // C
    0x07 | SHIFT, // D
    0x08 | SHIFT, // E
    0x09 | SHIFT, // F
    0x0a | SHIFT, // G
    0x0b | SHIFT, // H
    0x0c | SHIFT, // I
    0x0d | SHIFT, // J
    0x0e | SHIFT, // K
    0x0f | SHIFT, // L
    0x10 | SHIFT, // M
    0x11 | SHIFT, // N
    0x12 | SHIFT, // O
    0x13 | SHIFT, // P
    0x14 | SHIFT, // Q
    0x15 | SHIFT, // R
    0x16 | SHIFT, // S
    0x17 | SHIFT, // T
    0x18 | SHIFT, // U
    0x19 | SHIFT, // V
    0x1a | SHIFT, // W
    0x1b | SHIFT, // X
    0x1c | SHIFT, // Y
    0x1d | SHIFT, // Z
    0x2f,         // [
    0x31,         // bslash
    0x30,         // ]
    0x23 | SHIFT, // ^
    0x2d | SHIFT, // _
    0x35,         // `
    0x04,         // a
    0x05,         // b
    0x06,         // c
    0x07,         // d
    0x08,         // e
    0x09,         // f
    0x0a,         // g
    0x0b,         // h
    0x0c,         // i
    0x0d,         // j
    0x0e,         // k
    0x0f,         // l
    0x10,         // m
    0x11,         // n
    0x12,         // o
    0x13,         // p
    0x14,         // q
    0x15,         // r
    0x16,         // s
    0x17,         // t
    0x18,         // u
    0x19,         // v
    0x1a,         // w
    0x1b,         // x
    0x1c,         // y
    0x1d,         // z
    0x2f | SHIFT, // {
    0x31 | SHIFT, // |
    0x30 | SHIFT, // }
    0x35 | SHIFT, // ~
    0             // DEL
};

// press() adds the specified key (printing, non-printing, or modifier)
// to the persistent key report and sends the report.  Because of the way
// USB HID works, the host acts like the key remains pressed until we
// call release(), releaseAll(), or otherwise clear the report and resend.
size_t HID_kbd_press(uint8_t k) {
  uint8_t i;
  if (k >= 136) { // it's a non-printing key (not a modifier)
    k = k - 136;
  } else if (k >= 128) { // it's a modifier key
    _keyReport.modifiers |= (1 << (k - 128));
    k = 0;
  } else { // it's a printing key
    k = *(_asciimap + k);
    if (!k) {
      //setWriteError();
      return 0;
    }
    if (k & 0x80) { // it's a capital letter or other character reached with shift
      _keyReport.modifiers |= 0x02; // the left shift modifier
      k &= 0x7F;
    }
  }

  // Add k to the key report only if it's not already present
  // and if there is an empty slot.
  if (_keyReport.keys[0] != k && _keyReport.keys[1] != k &&
      _keyReport.keys[2] != k && _keyReport.keys[3] != k &&
      _keyReport.keys[4] != k && _keyReport.keys[5] != k) {

    for (i = 0; i < 6; i++) {
      if (_keyReport.keys[i] == 0x00) {
        _keyReport.keys[i] = k;
        break;
      }
    }
    if (i == 6) {
      //setWriteError();
      return 0;
    }
  }
  sendReport(&_keyReport);
  return 1;
}

// release() takes the specified key out of the persistent key report and
// sends the report.  This tells the OS the key is no longer pressed and that
// it shouldn't be repeated any more.
size_t HID_kbd_release(uint8_t k){
  uint8_t i;
  if (k >= 136) { // it's a non-printing key (not a modifier)
    k = k - 136;
  } else if (k >= 128) { // it's a modifier key
    _keyReport.modifiers &= ~(1 << (k - 128));
    k = 0;
  } else { // it's a printing key
    k = *(_asciimap + k);
    if (!k) {
      return 0;
    }
    if (k &
        0x80) { // it's a capital letter or other character reached with shift
      _keyReport.modifiers &= ~(0x02); // the left shift modifier
      k &= 0x7F;
    }
  }

  // Test the key report to see if k is present.  Clear it if it exists.
  // Check all positions in case the key is present more than once (which it
  // shouldn't be)
  for (i = 0; i < 6; i++) {
    if (0 != k && _keyReport.keys[i] == k) {
      _keyReport.keys[i] = 0x00;
    }
  }
  sendReport(&_keyReport);
  return 1;
}

void HID_kbd_releaseAll(void){
  _keyReport.keys[0] = 0;
  _keyReport.keys[1] = 0;
  _keyReport.keys[2] = 0;
  _keyReport.keys[3] = 0;
  _keyReport.keys[4] = 0;
  _keyReport.keys[5] = 0;
  _keyReport.modifiers = 0;
  sendReport(&_keyReport);
}

size_t HID_kbd_putch(char c){
  uint8_t p = HID_kbd_press(c); // Keydown
  HID_kbd_release(c);           // Keyup
  return p; // just return the result of press() since release() almost always
            // returns 1
}

size_t HID_kbd_write(const uint8_t *buffer, size_t size) {
  size_t n = 0;
  while (size--) {
    if (*buffer != '\r') {
      if (HID_kbd_putch(*buffer)) {
        n++;
      } else {
        break;
      }
    }
    buffer++;
  }
  return n;
}

size_t HID_kbd_print(char* str){
  return HID_kbd_write(str,strlen(str));
}

void usb_keyboard_transparent_isr(unsigned int k) {
  if (k == KEY_SHIFT || k == KEY_ALPHA)
    return; // not used individual.
  uint8_t out;
  if (is_key_down(KEY_ALPHA)) {
    out = key_map_to_alpha(k);
    if (is_key_down(KEY_SHIFT)) {
      out = tolower(out);
    }
  } else if (!is_key_down(KEY_SHIFT)) {
    switch (k) {
    case KEY_F1: {
      out = HID_KEY_F1;
      break;
    }
    case KEY_F2: {
      out = HID_KEY_F2;
      break;
    }
    case KEY_F3: {
      out = HID_KEY_F3;
      break;
    }
    case KEY_F4: {
      out = HID_KEY_F4;
      break;
    }
    case KEY_F5: {
      out = HID_KEY_F5;
      break;
    }
    case KEY_F6: {
      out = HID_KEY_F6;
      break;
    }
    case KEY_UP:{
      out = HID_KEY_UP_ARROW;
      break;
    }
    case KEY_DOWN:{
      out = HID_KEY_DOWN_ARROW;
      break;
    }
    case KEY_RIGHT: {
      out = HID_KEY_RIGHT_ARROW;
      break;
    }
    case KEY_LEFT: {
      out = HID_KEY_LEFT_ARROW;
      break;
    }
    case KEY_SYMB: {
      out = HID_KEY_MEDIA_EDIT;
      break;
    }
    case KEY_PLOT: {
      out = HID_KEY_MEDIA_WWW;
      break;
    }
    case KEY_NUM: {
      out = HID_KEY_MEDIA_CALC;
      break;
    }
    case KEY_HOME: {
      out = HID_KEY_HOME;
      break;
    }
    case KEY_APPS: {
      out = HID_KEY_LEFT_GUI; // windows键/super键/command键
      break;
    }
    case KEY_VIEWS: {
      return; // TODO: email btn
      out = 0;
      break;
    }
    case KEY_VARS: {
      HID_kbd_print("var"); // LOL
      return;
    }
    case KEY_MATH: {
      out = HID_KEY_MEDIA_CALC;
      break;
    }
    case KEY_ABC: {
      out = '/';
      break;
    }
    case KEY_XTPHIN: {
      out = 'x';
      break;
    }
    case KEY_BACKSPACE: {
      out = HID_KEY_BACKSPACE;
      break;
    }
    case KEY_SIN: {
      HID_kbd_print("sin(");
      return;
    }
    case KEY_COS: {
      HID_kbd_print("cos(");
      return;
    }
    case KEY_TAN: {
      HID_kbd_print("tan(");
      return;
    }
    case KEY_LN: {
      HID_kbd_print("ln(");
      return;
    }
    case KEY_LOG: {
      HID_kbd_print("log(");
      return;
    }
    case KEY_X2: {
      HID_kbd_print("^2");
      return;
    }
    case KEY_XY: {
      out = '^';
      break;
    }
    case KEY_LEFTBRACKET: {
      out = '(';
      break;
    }
    case KEY_RIGHTBRACET: {
      out = ')';
      break;
    }
    case KEY_DIVISION: {
      out = '/';
      break;
    }
    case KEY_PLUS: {
      out = '+';
      break;
    }
    case KEY_MULTIPLICATION: {
      out = '*';
      break;
    }
    case KEY_SUBTRACTION: {
      out = '-';
      break;
    }
    case KEY_COMMA: {
      out = ',';
      break;
    }
    case KEY_0: {
      out = '0';
      break;
    }
    case KEY_1: {
      out = '1';
      break;
    }
    case KEY_2: {
      out = '2';
      break;
    }
    case KEY_3: {
      out = '3';
      break;
    }
    case KEY_4: {
      out = '4';
      break;
    }
    case KEY_5: {
      out = '5';
      break;
    }
    case KEY_6: {
      out = '6';
      break;
    }
    case KEY_7: {
      out = '7';
      break;
    }
    case KEY_8: {
      out = '8';
      break;
    }
    case KEY_9: {
      out = '9';
      break;
    }
    case KEY_DOT: {
      out = '.';
      break;
    }
    case KEY_NEGATIVE: {
      out = '-';
      break;
    }
    case KEY_ENTER: {
      out = HID_KEY_KEYPAD_ENTER;
      break;
    }
    }
  }else{
    switch (k) {
    case KEY_F1: {
      out = HID_KEY_F1;
      break;
    }
    case KEY_F2: {
      out = HID_KEY_F2;
      break;
    }
    case KEY_F3: {
      out = HID_KEY_F3;
      break;
    }
    case KEY_F4: {
      out = HID_KEY_F4;
      break;
    }
    case KEY_F5: {
      out = HID_KEY_F5;
      break;
    }
    case KEY_F6: {
      out = HID_KEY_F6;
      break;
    }
    case KEY_UP: {
      out = HID_KEY_PAGE_UP;
      break;
    }
    case KEY_DOWN: {
      out = HID_KEY_PAGE_DOWN;
      break;
    }
    case KEY_RIGHT: {
      out = HID_KEY_END;
      break;
    }
    case KEY_LEFT: {
      out = HID_KEY_HOME;
      break;
    }
    //setups
    case KEY_SYMB: {
      out = HID_KEY_MEDIA_PREVIOUSSONG;
      break;
    }
    case KEY_PLOT: {
      out = HID_KEY_MEDIA_PLAYPAUSE;
      break;
    }
    case KEY_NUM: {
      out = HID_KEY_MEDIA_NEXTSONG;
      break;
    }
    //modes,info,help
    case KEY_HOME: {
      out = HID_KEY_MEDIA_VOLUMEDOWN;
      break;
    }
    case KEY_APPS: {
      out = HID_KEY_MEDIA_VOLUMEUP;
      break;
    }
    case KEY_VIEWS: {
      out = HID_KEY_HELP;
      break;
    }
    case KEY_VARS: {
      return;
    }
    case KEY_MATH: {  //cmds
      HID_kbd_press(HID_KEY_LEFT_GUI);
      vTaskDelay(100/portTICK_RATE_MS);
      HID_kbd_press(HID_KEY_R);
      vTaskDelay(200 / portTICK_RATE_MS);
      HID_kbd_releaseAll();
      vTaskDelay(200 / portTICK_RATE_MS);
      HID_kbd_press(HID_KEY_BACKSPACE);
      vTaskDelay(50 / portTICK_RATE_MS);
      HID_kbd_releaseAll();
      vTaskDelay(50 / portTICK_RATE_MS);
      HID_kbd_print("cmd");
      vTaskDelay(50 / portTICK_RATE_MS);
      HID_kbd_press(HID_KEY_KEYPAD_ENTER);
      vTaskDelay(50/portTICK_RATE_MS);
      HID_kbd_releaseAll();
      break;
    }
    case KEY_ABC: {
      out = '\'';
      break;
    }
    case KEY_XTPHIN: {
      HID_kbd_print("*10^");
      return;
    }
    case KEY_BACKSPACE: {
      HID_kbd_press(HID_KEY_LEFT_CTRL);
      vTaskDelay(100 / portTICK_RATE_MS);
      HID_kbd_press(HID_KEY_A);
      vTaskDelay(100 / portTICK_RATE_MS);
      HID_kbd_releaseAll();
      out = HID_KEY_BACKSPACE;
      break;
    }
    case KEY_SIN: {
      HID_kbd_print("arcsin(");
      return;
    }
    case KEY_COS: {
      HID_kbd_print("arccos(");
      return;
    }
    case KEY_TAN: {
      HID_kbd_print("arctan(");
      return;
    }
    case KEY_LN: {
      HID_kbd_print("exp(");
      return;
    }
    case KEY_LOG: {
      HID_kbd_print("*10^");
      return;
    }
    case KEY_X2: {
      HID_kbd_print("sqrt(");
      return;
    }
    case KEY_XY: {
      HID_kbd_print("root(");
      return;
    }
    case KEY_LEFTBRACKET: {
      out = HID_KEY_COPY;
      break;
    }
    case KEY_RIGHTBRACET: {
      out = HID_KEY_PASTE;
      break;
    }
    case KEY_DIVISION: {
      HID_kbd_print("^(-1)");
      return;
    }
    case KEY_PLUS: {
      HID_kbd_print("sum(");
      return;
    }
    case KEY_MULTIPLICATION: {
      out = '!';
      break;
    }
    case KEY_SUBTRACTION: {
      return; //TODO
      out = 's';
      break;
    }
    case KEY_COMMA: {
      return; //TODO
      out = ',';
      break;
    }
    case KEY_0: {
      HID_kbd_print("note:");
      return;
    }
    case KEY_1: {//prgm
      HID_kbd_print("#include <stdio.h>\nint main(){\nprintf(\"Hello, world\");\nreturn 0;\n}");
      return;
    }
    case KEY_2: {
      out = 'i';
      break;
    }
    case KEY_3: {
      HID_kbd_print("pi");
      return;
    }
    case KEY_4: {
      HID_kbd_print("[[]]");
      return;
    }
    case KEY_5: {
      out = '[';
      break;
    }
    case KEY_6: {
      out = ']';
      break;
    }
    case KEY_7: {
      HID_kbd_print("{}");
      return;
    }
    case KEY_8: {
      out = '{';
      break;
    }
    case KEY_9: {
      out = '}';
      break;
    }
    case KEY_DOT: {
      out = '=';
      break;
    }
    case KEY_NEGATIVE: {
      HID_kbd_print("abs(");
      return;
    }
    case KEY_ENTER: {
      HID_kbd_print("ans");
      return;
    }
    }
  }
  HID_kbd_putch(out);
  return;
}

/**
 * @brief Turn your 39gii into a math keyboard.
 * @note Untested!
 */
void enable_service_usb_keyboard_transparent() {
  register_keyboard_callback(usb_keyboard_transparent_isr);
}

/**
 * @brief Stop your 39gii being a math keyboard.
 * @note Not implemented!
 */
void disable_service_usb_keyboard_transparent() {
}