//Translated almost line-to-line from https://github.com/cyborg5/TinyUSB_Mouse_and_Keyboard
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

enum {
  REPORT_ID_KEYBOARD = 1,
  REPORT_ID_MOUSE,
  REPORT_ID_COUNT
};

/*****************************
 *   MOUSE SECTION
 *****************************/
#define MOUSE_LEFT 1
#define MOUSE_RIGHT 2
#define MOUSE_MIDDLE 4
#define MOUSE_ALL (MOUSE_LEFT | MOUSE_RIGHT | MOUSE_MIDDLE)

void HID_mouse_click(uint8_t btn);
void HID_mouse_move(signed char x, signed char y, signed char wheel);
void HID_mouse_press(uint8_t btn);     // press LEFT by default
void HID_mouse_release(uint8_t btn);   // release LEFT by default
bool HID_mouse_isPressed(uint8_t btn); // check LEFT by default

/******************************
 *    KEYBOARD SECTION
 ******************************/
//  Keyboard codes
//  Note these are different in some respects to the TinyUSB codes but
//  are compatible with Arduino Keyboard.h API

#define HID_KEY_LEFT_CTRL 0x80
#define HID_KEY_LEFT_SHIFT 0x81
#define HID_KEY_LEFT_ALT 0x82
#define HID_KEY_LEFT_GUI 0x83
#define HID_KEY_RIGHT_CTRL 0x84
#define HID_KEY_RIGHT_SHIFT 0x85
#define HID_KEY_RIGHT_ALT 0x86
#define HID_KEY_RIGHT_GUI 0x87

#define HID_KEY_UP_ARROW 0xDA
#define HID_KEY_DOWN_ARROW 0xD9
#define HID_KEY_LEFT_ARROW 0xD8
#define HID_KEY_RIGHT_ARROW 0xD7
#define HID_KEY_ESC 0xB1


#define HID_KEY_OPEN 0x74       // Keyboard Execute
#define HID_KEY_HELP 0x75       // Keyboard Help
#define HID_KEY_PROPS 0x76      // Keyboard Menu
#define HID_KEY_FRONT 0x77      // Keyboard Select
#define HID_KEY_STOP 0x78       // Keyboard Stop
#define HID_KEY_AGAIN 0x79      // Keyboard Again
#define HID_KEY_UNDO 0x7a       // Keyboard Undo
#define HID_KEY_CUT 0x7b        // Keyboard Cut
#define HID_KEY_COPY 0x7c       // Keyboard Copy
#define HID_KEY_PASTE 0x7d      // Keyboard Paste
#define HID_KEY_FIND 0x7e       // Keyboard Find
#define HID_KEY_MUTE 0x7f       // Keyboard Mute
#define HID_KEY_VOLUMEUP 0x80   // Keyboard Volume Up
#define HID_KEY_VOLUMEDOWN 0x81 // Keyboard Volume Down
// 0x82  Keyboard Locking Caps Lock
// 0x83  Keyboard Locking Num Lock
// 0x84  Keyboard Locking Scroll Lock
#define HID_KEY_KPCOMMA 0x85 // Keypad Comma
// 0x86  Keypad Equal Sign
#define HID_KEY_RO 0x87               // Keyboard International1
#define HID_KEY_KATAKANAHIRAGANA 0x88 // Keyboard International2
#define HID_KEY_YEN 0x89              // Keyboard International3
#define HID_KEY_HENKAN 0x8a           // Keyboard International4
#define HID_KEY_MUHENKAN 0x8b         // Keyboard International5
#define HID_KEY_KPJPCOMMA 0x8c        // Keyboard International6
// 0x8d  Keyboard International7
// 0x8e  Keyboard International8
// 0x8f  Keyboard International9
#define HID_KEY_HANGEUL 0x90        // Keyboard LANG1
#define HID_KEY_HANJA 0x91          // Keyboard LANG2
#define HID_KEY_KATAKANA 0x92       // Keyboard LANG3
#define HID_KEY_HIRAGANA 0x93       // Keyboard LANG4
#define HID_KEY_ZENKAKUHANKAKU 0x94 // Keyboard LANG5
// 0x95  Keyboard LANG6
// 0x96  Keyboard LANG7
// 0x97  Keyboard LANG8
// 0x98  Keyboard LANG9
// 0x99  Keyboard Alternate Erase
// 0x9a  Keyboard SysReq/Attention
// 0x9b  Keyboard Cancel
// 0x9c  Keyboard Clear
// 0x9d  Keyboard Prior
// 0x9e  Keyboard Return
// 0x9f  Keyboard Separator
// 0xa0  Keyboard Out
// 0xa1  Keyboard Oper
// 0xa2  Keyboard Clear/Again
// 0xa3  Keyboard CrSel/Props
// 0xa4  Keyboard ExSel

// 0xb0  Keypad 00
// 0xb1  Keypad 000
// 0xb2  Thousands Separator
// 0xb3  Decimal Separator
// 0xb4  Currency Unit
// 0xb5  Currency Sub-unit
#define HID_KEY_KPLEFTPAREN 0xb6  // Keypad (
#define HID_KEY_KPRIGHTPAREN 0xb7 // Keypad )
// 0xb8  Keypad {
// 0xb9  Keypad }
// 0xba  Keypad Tab
// 0xbb  Keypad Backspace
// 0xbc  Keypad A
// 0xbd  Keypad B
// 0xbe  Keypad C
// 0xbf  Keypad D
// 0xc0  Keypad E
// 0xc1  Keypad F
// 0xc2  Keypad XOR
// 0xc3  Keypad ^
// 0xc4  Keypad %
// 0xc5  Keypad <
// 0xc6  Keypad >
// 0xc7  Keypad &
// 0xc8  Keypad &&
// 0xc9  Keypad |
// 0xca  Keypad ||
// 0xcb  Keypad :
// 0xcc  Keypad #
// 0xcd  Keypad Space
// 0xce  Keypad @
// 0xcf  Keypad !
// 0xd0  Keypad Memory Store
// 0xd1  Keypad Memory Recall
// 0xd2  Keypad Memory Clear
// 0xd3  Keypad Memory Add
// 0xd4  Keypad Memory Subtract
// 0xd5  Keypad Memory Multiply
// 0xd6  Keypad Memory Divide
// 0xd7  Keypad +/-
// 0xd8  Keypad Clear
// 0xd9  Keypad Clear Entry
// 0xda  Keypad Binary
// 0xdb  Keypad Octal
// 0xdc  Keypad Decimal
// 0xdd  Keypad Hexadecimal

#define HID_KEY_LEFTCTRL 0xe0   // Keyboard Left Control
#define HID_KEY_LEFTSHIFT 0xe1  // Keyboard Left Shift
#define HID_KEY_LEFTALT 0xe2    // Keyboard Left Alt
#define HID_KEY_LEFTMETA 0xe3   // Keyboard Left GUI
#define HID_KEY_RIGHTCTRL 0xe4  // Keyboard Right Control
#define HID_KEY_RIGHTSHIFT 0xe5 // Keyboard Right Shift
#define HID_KEY_RIGHTALT 0xe6   // Keyboard Right Alt
#define HID_KEY_RIGHTMETA 0xe7  // Keyboard Right GUI

#define HID_KEY_MEDIA_PLAYPAUSE 0xe8
#define HID_KEY_MEDIA_STOPCD 0xe9
#define HID_KEY_MEDIA_PREVIOUSSONG 0xea
#define HID_KEY_MEDIA_NEXTSONG 0xeb
#define HID_KEY_MEDIA_EJECTCD 0xec
#define HID_KEY_MEDIA_VOLUMEUP 0xed
#define HID_KEY_MEDIA_VOLUMEDOWN 0xee
#define HID_KEY_MEDIA_MUTE 0xef
#define HID_KEY_MEDIA_WWW 0xf0
#define HID_KEY_MEDIA_BACK 0xf1
#define HID_KEY_MEDIA_FORWARD 0xf2
#define HID_KEY_MEDIA_STOP 0xf3
#define HID_KEY_MEDIA_FIND 0xf4
#define HID_KEY_MEDIA_SCROLLUP 0xf5
#define HID_KEY_MEDIA_SCROLLDOWN 0xf6
#define HID_KEY_MEDIA_EDIT 0xf7
#define HID_KEY_MEDIA_SLEEP 0xf8
#define HID_KEY_MEDIA_COFFEE 0xf9
#define HID_KEY_MEDIA_REFRESH 0xfa
#define HID_KEY_MEDIA_CALC 0xfb

//  Low level key report: up to 6 keys and shift, ctrl etc at once
typedef struct {
  uint8_t modifiers;
  uint8_t reserved;
  uint8_t keys[6];
} KeyReport;

size_t HID_kbd_write(const uint8_t *buffer, size_t size);
size_t HID_kbd_print(char *str);
size_t HID_kbd_press(uint8_t k);
size_t HID_kbd_release(uint8_t k);
void HID_kbd_releaseAll(void);

#ifdef __cplusplus
}
#endif