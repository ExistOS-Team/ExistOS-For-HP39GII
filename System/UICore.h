#pragma once

#include "UI_Config.h"
#include "UI_Language.h"

extern const unsigned char VGA_Ascii_5x8[];
extern const unsigned char VGA_Ascii_6x12[];
extern const unsigned char VGA_Ascii_8x16[];

#ifdef __cplusplus

typedef struct UI_Rect {
    uint32_t x;
    uint32_t y;
    uint32_t w;
    uint32_t h;
} UI_Rect;

class UI_Display {
private:
    uint8_t *disp_buf;
    int disp_w, disp_h;
    void (*drawf)(uint8_t *buf, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1);
    inline void buf_set(uint32_t x, uint32_t y, uint8_t c) {
        if (disp_buf) {
            if ((x < this->disp_w) && (y < this->disp_h))
                this->disp_buf[x + y * this->disp_w] = c;
        }
    }

public:
    UI_Display(int display_width, int display_height, void (*drawf)(uint8_t *buf, uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1)) {
        printf("Create UI Display.\n");
        this->disp_buf = (uint8_t *)pvPortMalloc(display_width * display_height);
        this->drawf = drawf;
        this->disp_w = display_width;
        this->disp_h = display_height;
        memset(this->disp_buf, 0xff, display_width * display_height);
        this->drawf(this->disp_buf, 0, 0, this->disp_w - 1, this->disp_h - 1);
    }

    void emergencyBuffer() {
        disp_buf = (uint8_t *)(RAM_BASE + BASIC_RAM_SIZE - 33 * 1024);
    }

    void releaseBuffer() {
        if (disp_buf) {
            vPortFree(disp_buf);
            disp_buf = NULL;
        }
    }

    void restoreBuffer() {
        if (!disp_buf) {
            this->disp_buf = (uint8_t *)pvPortMalloc(disp_w * disp_h);
        }
    }

    void draw_point(uint32_t x, uint32_t y, uint8_t c) {
        buf_set(x, y, c);
        this->drawf(&this->disp_buf[y * this->disp_w], 0, y, this->disp_w - 1, y);
    }
    void draw_line(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, uint8_t c) {
        if (y0 == y1) {
            for (int x = x0; x <= x1; x++) {
                buf_set(x, y0, c);
            }
            goto draw_fin;
        }
        if (x0 == x1) {
            for (int y = y0; y <= y1; y++) {
                buf_set(x0, y, c);
            }
            goto draw_fin;
        }

        int x, y, dx, dy, e;
        dx = x1 - x0;
        dy = y1 - y0;
        e = -dx;
        x = x0;
        y = y0;
        for (int i = 0; i < dx; i++) {
            buf_set(x, y, c);
            x++;
            e += 2 * dy;
            if (e >= 0) {
                y++;
                e -= 2 * dx;
            }
        }
    draw_fin:
        this->drawf(&this->disp_buf[y0 * this->disp_w], 0, y0, this->disp_w - 1, y1);
    }
    void draw_box(uint32_t x0, uint32_t y0, uint32_t x1, uint32_t y1, int16_t borderColor, int16_t fillColor) {
        if (fillColor != -1) {
            for (int y = y0; y <= y1; y++) {
                draw_line(x0, y, x1, y, fillColor);
            }
        }
        if (borderColor != -1) {
            draw_line(x0, y0, x1, y0, borderColor);
            draw_line(x0, y1, x1, y1, borderColor);

            draw_line(x0, y0, x0, y1, borderColor);
            draw_line(x1, y0, x1, y1, borderColor);
        }
    }

    void draw_bmp(char *src, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h) {
        for (int y = 0; y < h; y++) {
            for (int x = 0; x < w; x++) {
                buf_set(x0 + x, y0 + y, src[x + y * w]);
            }
        }

        this->drawf(&this->disp_buf[y0 * this->disp_w], 0, y0, this->disp_w - 1, y0 + h);
    }

    void draw_char_ascii(uint32_t x0, uint32_t y0, char ch, uint8_t fontSize, uint8_t fg, int16_t bg) {
        int font_w;
        int font_h;
        const unsigned char *pCh;
        unsigned int x = 0, y = 0, i = 0, j = 0;

        if ((ch < ' ') || (ch > '~' + 1)) {
            return;
        }

        switch (fontSize) {
        case 8:
            font_w = 8;
            font_h = 8;
            pCh = VGA_Ascii_5x8 + (ch - ' ') * font_h;
            break;

        case 12:
            font_w = 8;
            font_h = 12;
            pCh = VGA_Ascii_6x12 + (ch - ' ') * font_h;
            break;

        case 16:
            font_w = 8;
            font_h = 16;
            pCh = VGA_Ascii_8x16 + (ch - ' ') * font_h;
            break;

        default:
            return;
        }
        char pix;
        while (y < font_h) {
            while (x < font_w) {
                pix = ((*pCh << x) & 0x80U);
                if (pix) {
                    buf_set(x0 + x, y0 + y, fg);
                } else {
                    if (bg != -1) {
                        buf_set(x0 + x, y0 + y, bg);
                    }
                }
                x++;
            }
            x = 0;
            y++;
            pCh++;
        }

        this->drawf(&this->disp_buf[y0 * this->disp_w], 0, y0, this->disp_w - 1, y0 + font_h - 1);
    }
    void draw_char_GBK16(uint32_t x0, uint32_t y0, uint16_t c, uint8_t fg, int16_t bg) {
        extern uint32_t fonts_hzk_start;
        extern uint32_t fonts_hzk_end;
        int lv = (c & 0xFF) - 0xa1;
        int hv = (c >> 8) - 0xa1;
        uint32_t offset = (uint32_t)(94 * hv + lv) * 32;
        uint8_t *font_data = (uint8_t *)(((uint32_t)&fonts_hzk_start) + offset);
        if ((uint32_t)font_data > (uint32_t)&fonts_hzk_end) {
            return;
        }

        int x = x0, y = y0;

        for (int i = 0; i < 32; i += 2) {
            uint8_t pix;
            for (int t = 0, pix = font_data[i]; t < 8; t++) {
                if (pix & 0x80) {
                    buf_set(x, y, fg);
                } else {
                    if (bg != -1)
                        buf_set(x, y, bg);
                }
                ++x;
                pix <<= 1;
            }

            for (int t = 0, pix = font_data[i + 1]; t < 8; t++) {
                if (pix & 0x80) {
                    buf_set(x, y, fg);
                } else {
                    if (bg != -1)
                        buf_set(x, y, bg);
                }
                ++x;
                pix <<= 1;
            }

            x = x0;
            y++;
        }
        // printf("GBK PRINT:%02x\n", c);
        this->drawf(&this->disp_buf[y0 * this->disp_w], 0, y0, this->disp_w - 1, y0 + 16);
    }
    int draw_printf(uint32_t x0, uint32_t y0, uint8_t fontSize, uint8_t fg, int16_t bg, const char *format, ...) {
        va_list aptr;
        int ret;

        char buffer[256];

        va_start(aptr, format);
        ret = vsprintf(buffer, format, aptr);
        va_end(aptr);

        for (int i = 0, x = x0; (i < sizeof(buffer)) && (buffer[i]); i++) {
            if (buffer[i] < 0x80) {
                draw_char_ascii(x, y0, buffer[i], fontSize, fg, bg);
                x += 8;
                if (x > disp_w) {
                    break;
                }
            } else {
                draw_char_GBK16(x, y0, buffer[i + 1] | (buffer[i] << 8), fg, bg);
                x += 16;
                if (x > disp_w) {
                    break;
                }
                i++;
            }
        }
        return (ret);
    }
    ~UI_Display() {
        vPortFree(this->disp_buf);
    }
};
#if 0

class UI_Surface {
private:
    UI_Surface *parSurfNext = NULL;
    UI_Surface *parSurfPrev = NULL;

    UI_Surface *subSurfNext = NULL;

    UI_Surface *subFocus = NULL;

    int16_t bg;
    bool visualable;
    bool active = false;



public:
    UI_Display *disp;
    UI_Rect loc;

    UI_Surface(UI_Display *disp, UI_Rect *loc, int16_t bg = -1, bool visualable = true) {
        this->disp = disp;
        this->visualable = visualable;
        this->bg = bg;
        memcpy(&this->loc, loc, sizeof(UI_Rect));
        refresh(false);
    }

    UI_Surface(UI_Surface *parPrev, UI_Display *disp, UI_Rect *loc, int16_t bg = -1, bool visualable = true) {
        this->disp = disp;
        this->visualable = visualable;
        this->bg = bg;
        this->parSurfPrev = parPrev;
        memcpy(&this->loc, loc, sizeof(UI_Rect));
        refresh(false);
    }

    virtual void refreshExt(bool focus) {}

    virtual int keyMsgExt(uint32_t key, int state) {
        return 0;
    }

    void setActive(bool active)
    {
        this->active = active;
    }

    int keyMsg(uint32_t key, int state) {
        int ret = 0;

        if (active) {
            ret = keyMsgExt(key, state);
            if (ret) {
                return ret;
            }

            if((key == KEY_DEFAULT_NEXT) && (state == KEY_TRIG))
            {
                if((subFocus) && (subFocus->parSurfNext) && (subFocus->active))
                {
                    subFocus = subFocus->parSurfNext;
                    return KEY_MESSAGE_RET_CAPTURE;
                }
            }

            if((key == KEY_DEFAULT_PREV) && (state == KEY_TRIG) && (subFocus->active))
            {
                if((subFocus) && (subFocus->parSurfPrev))
                {
                    subFocus = subFocus->parSurfPrev;
                    return KEY_MESSAGE_RET_CAPTURE;
                }
            }

            if (subSurfNext) {
                ret = subSurfNext->keyMsg(key, state);
                if (ret) {
                    return ret;
                }
            }
        }
        if (parSurfNext) {
            ret = parSurfNext->keyMsg(key, state);
            if (ret) {
                return ret;
            }
        }
    }

    void refresh(bool focus) {
        if (visualable) {
            if (bg >= 0)
                disp->draw_box(loc.x, loc.y, loc.x + loc.w, loc.y + loc.h, -1, bg);
            refreshExt(focus);
            if(subSurfNext)
            {
                subSurfNext->refresh(subFocus == subSurfNext);
            }
        }
    }

    UI_Surface *newSubSurf(UI_Rect *loc, int16_t bg = -1, bool visualable = true) {
        UI_Surface *ptr = subSurfNext;
        if (ptr) {
            while (ptr->subSurfNext) {
                ptr = ptr->subSurfNext;
            }
            ptr->subSurfNext = new UI_Surface(disp, loc, bg, visualable);
            ptr = ptr->subSurfNext;

        } else {
            ptr = new UI_Surface(disp, loc, bg, visualable);
            subFocus = ptr;
        }
        return ptr;
    }

    UI_Surface *newParSurf(UI_Rect *loc, int16_t bg = -1, bool visualable = true) {
        UI_Surface *ptr = parSurfNext;
        if (ptr) {
            while (ptr->parSurfNext) {
                ptr = ptr->parSurfNext;
            }
            ptr->parSurfNext = new UI_Surface(ptr, disp, loc, bg, visualable);
            ptr = ptr->parSurfNext;

        } else {
            ptr = new UI_Surface(this, disp, loc, bg, visualable);
        }
        return ptr;
    }

    ~UI_Surface();
};

#endif

class UI_Window {
private:
    /* data */

    uint32_t width;
    uint32_t height;
    uint32_t x0;
    uint32_t y0;

    char *title = NULL;
    char *sub_title = NULL;
    UI_Window *parent = NULL;
    UI_Window *child = NULL;

    //    class UI_Widget *widget_chains = NULL;

    bool funcKey_enable;
    char funcKey[6][FUNCKEY_ITEM_MAX_CHAR + 1];

public:
    class UI_Widget *widget_focus = NULL;

    uint32_t content_x0;
    uint32_t content_y0;
    uint32_t content_width;
    uint32_t content_height;

    UI_Display *disp;

    UI_Window(UI_Window *parent, UI_Window *child, const char *title, UI_Display *disp, uint32_t x0, uint32_t y0, uint32_t w, uint32_t h) {

        this->x0 = x0;
        this->y0 = y0;
        this->width = w;
        this->height = h;
        this->parent = parent;
        this->child = child;
        this->disp = disp;
        memset(this->funcKey, 0, sizeof(this->funcKey));
        if (title) {
            this->title = (char *)pvPortMalloc(strlen(title));
            strcpy(this->title, title);
            this->content_x0 = x0 + 1;
            this->content_y0 = y0 + WIN_DEFAULT_FONTSIZE - 3;
            this->content_width = w - 1;
            this->content_height = h - 1;
        } else {
            this->content_x0 = x0 + 1;
            this->content_y0 = y0 + 1;
            this->content_width = w - 1;
            this->content_height = h - 1;
        }

        refreshWindow();
    };

    void refreshTitle() {
        disp->draw_box(x0 + 1, y0, x0 + width, y0 + WIN_DEFAULT_FONTSIZE - 3, WIN_DEFAULT_BORDER_COLOR, WIN_DEFAULT_TITLE_BG_COLOR);
        disp->draw_printf(x0 + 1, y0, WIN_DEFAULT_FONTSIZE, WIN_DEFAULT_TITLE_FONT_COLOR, WIN_DEFAULT_TITLE_BG_COLOR, this->title);
        // disp->draw_line(x0, WIN_DEFAULT_FONTSIZE, x0 + width, WIN_DEFAULT_FONTSIZE, WIN_DEFAULT_BORDER_COLOR);
    }

    void refreshFuncKeyBar() {
        if (funcKey_enable) {
            uint32_t item_w = FUNCKEY_BAR_WITDH / 6;

            disp->draw_box(FUNCKEY_BAR_X, FUNCKEY_BAR_Y, FUNCKEY_BAR_WITDH, FUNCKEY_BAR_Y + FUNCKEY_FONTSIZE, FUNCKEY_BAR_BG_COLOR, FUNCKEY_BAR_BG_COLOR);

            for (int i = 1; i < 6; i++) {
                disp->draw_box(i * item_w, FUNCKEY_BAR_Y, i * item_w, FUNCKEY_BAR_Y + FUNCKEY_FONTSIZE - 1, 255 - FUNCKEY_BAR_BG_COLOR, 255 - FUNCKEY_BAR_BG_COLOR);
            }

            for (int i = 0; i < 6; i++) {
                if (this->funcKey[i][0]) {
                    disp->draw_printf(i * item_w, FUNCKEY_BAR_Y, FUNCKEY_FONTSIZE, 255 - FUNCKEY_BAR_BG_COLOR, -1,
                                      "%*s",
                                      3 + strlen(this->funcKey[i]) / 2, this->funcKey[i], 3 - strlen(this->funcKey[i]) / 2, "");
                }
            }
        }
    }

    void refreshWindow() {
        disp->draw_box(0, 0, width - 2, height - 1, WIN_DEFAULT_BORDER_COLOR, WIN_DEFAULT_BG_COLOR);
        refreshTitle();
        refreshFuncKeyBar();
    }

    /*
        virtual int winSelfKeyMessage(uint32_t key, int state) {
            return 0;
        }

        int winKeyMessage(uint32_t key, int state);
        */

    void enableFuncKey(bool enable) {
        if (this->content_y0 + this->content_height >= FUNCKEY_BAR_Y) {
            this->content_height = FUNCKEY_BAR_Y - WIN_DEFAULT_FONTSIZE;
            this->height = this->content_height + 1;
            printf("Decrease H:%d\n", this->content_height);
        }
        funcKey_enable = enable;
        refreshFuncKeyBar();
    }

    void setFuncKeys(const char *list) {
        int i = 0, j = 0, ind = 0;
        memset(this->funcKey, 0, sizeof(this->funcKey));
        while (list[i]) {
            if (list[i] != '|') {
                this->funcKey[ind][j++] = list[i++];
                if (j >= FUNCKEY_ITEM_MAX_CHAR) {
                    this->funcKey[ind][j] = '\0';
                    while ((list[i]) && (list[i] != '|')) {
                        i++;
                    }
                }
            } else {
                this->funcKey[ind][j] = '\0';
                ind++;
                j = 0;
                i++;
            }

            if (ind >= 6) {
                break;
            }
        }
        refreshFuncKeyBar();

        // for (int i = 0; i < 6; i++) {
        //     printf("bar[%d]:%s\n", i, funcKey[i]);
        // }
    }

    // void addWidget(class UI_Widget *widget);
    // void setFocusWidget(class UI_Widget *widget);
    //  void widgetFocusNext();
    //  void widgetFocusPrev();

    ~UI_Window() {
        vPortFree(this->title);
    };
};

class UI_Msgbox {
private:
    uint32_t x0;
    uint32_t y0;
    uint32_t width;
    uint32_t height;
    uint32_t text_x0;
    uint32_t text_y0;

    char *title;
    char *text;

    UI_Display *disp;

public:
    UI_Msgbox(UI_Display *_disp, uint32_t _x0, uint32_t _y0, uint32_t _width, uint32_t _height, const char *_title, const char *_text) {
        this->disp = _disp;

        this->x0 = _x0;
        this->y0 = _y0;
        this->width = _width;
        this->height = _height;

        this->title = (char *)calloc(strlen(_title) + 1, sizeof(char));
        strcpy(this->title, _title);
        this->text = (char *)calloc(strlen(_text) + 1, sizeof(char));
        strcpy(this->text, _text);

        this->text_x0 = this->x0 + (this->width - (strlen(this->text) * 8)) / 2;
        this->text_y0 = this->y0 + this->height / 2;
    }

    ~UI_Msgbox() {
        free(this->title);
        free(this->text);
    }

    void refresh() {
        disp->draw_box(this->x0, this->y0, this->x0 + this->width, this->y0 + this->height, 0, 224);
        disp->draw_line(this->x0, this->y0 + 16, this->x0 + this->width, this->y0 + 16, 0);
        disp->draw_printf(this->x0 + 4, this->y0 + 4, 12, 0, 224, "%s", this->title);
        disp->draw_printf(this->text_x0, this->text_y0, 12, 0, 224, "%s", this->text);
    }

    bool show() {
        uint32_t key;
        uint32_t keyVal = 0;
        uint32_t press = 0;
        vTaskDelay(20);
        do {
            key = ll_vm_check_key();
            press = key >> 16;
            keyVal = key & 0xFFFF;
        } while (!press);
        if (keyVal == KEY_ENTER)
            return true;

        return false;
    }

    void setText(const char *_text) {
        strcpy(this->text, _text);
        this->text_x0 = this->x0 + (this->width - (strlen(this->text) * 8)) / 2;
        this->text_y0 = this->y0 + this->height / 2;
    }
};

/*
void UI_Window::addWidget(UI_Widget *widget) {
    class UI_Widget *ptr = widget_chains;
    if (ptr) {
        while (ptr->next) {
            ptr = ptr->next;
        }
        ptr->next = widget;
        widget->prev = ptr;
    } else {
        widget_chains = widget;
        widget_focus = widget_chains;
        widget->prev = NULL;
        widget->next = NULL;
    }
    widget->win = this;
}

void UI_Window::setFocusWidget(UI_Widget *widget) {
    this->widget_focus = widget;
}

void UI_Window::refreshWindow(void) {
    disp->draw_box(0, 0, width - 2, height - 1, WIN_DEFAULT_BORDER_COLOR, WIN_DEFAULT_BG_COLOR);
    refreshTitle();
    refreshFuncKeyBar();

    class UI_Widget *widgetptr = widget_chains;
    while (widgetptr) {
        widgetptr->drawContent();
        widgetptr = widgetptr->next;
    }

    if (this->child) {
        this->child->refreshWindow();
    }
}

int UI_Window::winKeyMessage(uint32_t key, int state) {
    int ret = winSelfKeyMessage(key, state);

    if (ret) {
        return ret;
    }

    if (child) {
        if (ret = child->winKeyMessage(key, state)) {
            return ret;
        }
    }
    if (widget_focus) {
        ret = this->widget_focus->widgetkeyMessage(key, state);
    } else {
        widget_focus = widget_chains;
        if(widget_focus)
        {
            ret = this->widget_focus->widgetkeyMessage(key, state);
        }
    }
    if (ret) {
        return ret;
    }

    return 0;
}
*/

#if 0

class UI_Widget {
private:
public:
    UI_Window *win = NULL;
    UI_Widget *next = NULL;
    UI_Widget *prev = NULL;

    UI_Widget *focus_next = NULL;

    uint32_t x, y, w, h;

    UI_Widget(uint32_t x, uint32_t y, uint32_t width, uint32_t height) {
        this->x = x;
        this->y = y;
        this->w = width;
        this->h = height;
    }

    void addSubWidget(UI_Widget *widget) {
        // printf("add next:%p", widget);
        UI_Widget *ptr = this->next;
        if (ptr) {
            while (ptr->next) {
                ptr = ptr->next;
            }
            ptr->next = widget;
            widget->prev = ptr;
        } else {
            this->next = widget;
            widget->prev = this;
            focus_next = widget;
        }
        widget->win = this->win;
    }

    virtual int widgetSelfKeyMessage(uint32_t key, int state) {
        return 0;
    }

    int widgetkeyMessage(uint32_t key, int state) {
        int ret = 0;
        //printf("[%p]rev:%d,%d\n",this,key,state);
        if (next) {
            if (ret = next->widgetkeyMessage(key, state)) {
                return ret;
            }
        }

        if (ret = widgetSelfKeyMessage(key, state)) {
            return ret;
        }

        if ((key == KEY_DEFAULT_NEXT) && (state == KEY_TRIG)) {
            if (win->widget_focus == this) {
                if (next) {
                    printf("focus next:%p\n", next);
                    win->widget_focus = next;
                }
            }
            return KEY_MESSAGE_RET_CAPTURE;
        }

        if ((key == KEY_DEFAULT_PREV) && (state == KEY_TRIG)) {
            if (win->widget_focus == this) {
                if (prev) {
                    printf("focus prev:%p\n", prev);
                    win->widget_focus = prev;
                }
            }
            return KEY_MESSAGE_RET_CAPTURE;
        }

        return 0;
    };

    virtual void drawContent() {
        if (next) {
            next->drawContent();
        }
    };

    ~UI_Widget() {
    }
};

class UI_WidgetImgButton : public UI_Widget {
private:
    uint8_t *pic = NULL;
    char title[12];

public:
    using UI_Widget::UI_Widget;

    void setPic(uint8_t *dat) {
        pic = dat;
    }

    void setTitle(const char *title) {
        int len = sizeof(this->title);
        int i = 0;
        while ((len > 1) && (title[i])) {
            this->title[i] = title[i];
            i++;
            len--;
        }
        this->title[i] = 0;
    }

    void drawContent() {
        if (pic) {
            win->disp->draw_bmp((char *)pic, x, y, w, h);
        }

        if (win->widget_focus == this) {
            win->disp->draw_box(x, y, x + w, y + h, 0xA0, -1);
        } else {
            win->disp->draw_box(x, y, x + w, y + h, 0xFF, -1);
        }

        if (title[0]) {
            win->disp->draw_printf(x, y + h + 1, 16, 0, -1,
                                   "%s",
                                   this->title);
        }
        if (next) {
            next->drawContent();
        }
    };
};

class UI_WidgetPage : public UI_Widget {
private:
    UI_Widget *page_slot[6] = {NULL};
    UI_Widget *curDispPage = NULL;

    uint32_t page_key[6];
    int page_keystate[6];

public:
    using UI_Widget::UI_Widget;

    UI_Widget *newPage(int pageid) {
        if (pageid >= 6) {
            return NULL;
        }

        page_slot[pageid] = new UI_Widget(x, y, w, h);
        page_slot[pageid]->win = this->win;

        page_key[pageid] = 0xFFFFFFFF;
        page_keystate[pageid] = -1;

        return page_slot[pageid];
    };

    void setDisplayPage(int page) {
        if (page >= 6) {
            return;
        }
        curDispPage = page_slot[page];
        //if (curDispPage)
        //    win->setFocusWidget(curDispPage);
    }

    void bindPageSwitchWithKey(int page, uint32_t key, int state) {
        if (page >= 6) {
            return;
        }

        // printf("bind page:%d, %d, %d\n",page, key, state);
        page_key[page] = key;
        page_keystate[page] = state;
    }

    int widgetSelfKeyMessage(uint32_t key, int state) {
        for (int i = 0; i < 6; i++) {
            // intf("i:%d,%08x,%08x\n", i, pppage_key][[pppage_keystate][[
            if ((page_key[i] == key) && (page_keystate[i] == state)) {
                if (page_slot[i]) {
                    // printf("set page:%d\n",i);
                    setDisplayPage(i);
                    drawContent();
                    return KEY_MESSAGE_RET_CAPTURE;
                }
            }
        }

        if(curDispPage)
        {
            //int ret;
            return curDispPage->widgetkeyMessage(key, state);
            //return ret;
        }

        return 0;
    }

    UI_Widget *getPageContent(int pageid) {
        int id = 0;
        if (pageid >= 6) {
            return NULL;
        }

        return page_slot[pageid];
    }

    void drawContent() {
        printf("draw page:%p\n", curDispPage);
        if (curDispPage) {
            printf("mw:%d,%d,%d,%d\n", x, y, x + w, y + h);
            this->win->disp->draw_box(x, y, x + w, y + h, -1, WIN_DEFAULT_BG_COLOR);
            curDispPage->drawContent();
        }
    }

    ~UI_WidgetPage(){
        /*
        UI_Widget *ptr = page;
        UI_Widget *ptr_next;
        do {
            ptr_next = ptr->next;
            delete ptr;
            ptr = ptr_next;
        } while (ptr_next);
        */
    };
};

#endif

#define DISPX 0
#define DISPY 12
#define DISPH 96
#define DISPW 256
#define CONSH (DISPH / 8) /* 12 */
#define CONSW (DISPW / 8) /* 32 */
#define FONTS 8
struct SimpShell {
    struct ShellDispLine {
        char col[CONSW];
    } lin[CONSH];
    uint32_t cx, cy;
    UI_Display *uidisp;

    SimpShell(UI_Display *a_uidisp){
        this->clear();
        this->uidisp = a_uidisp;
    }

    void refresh() {
        uidisp->draw_box(DISPX, DISPY, DISPX + DISPW, DISPY + DISPH, -1, 255);
        for (int i = 0; i < CONSH; i++) {
            for (int j = 0; j < CONSW; j++) {
                uidisp->draw_char_ascii(DISPX + FONTS * j, DISPY + FONTS * i, lin[i].col[j], FONTS, 0, 255);
            }
        }
    }

    void clear() {
        cx = cy = 0;
        memset(lin, 0, sizeof(lin));
    }

    int scroll() {
        if (cy >= CONSH) {
            for (int i = 0; i < CONSH - 1; i++) {
                lin[i] = lin[i + 1];
            }
            memset(&lin[CONSH - 1], 0, sizeof(lin[CONSH - 1]));
            this->refresh();
            cy = CONSH - 1;
            return 1;
        }
        return 0;
    }

    void put(const char c) {
        lin[cy].col[cx] = c;
        cx++;
        if (cx >= CONSW) {
            cx = 0;
            cy++;
            this->scroll();
        }
    }

    void puts(const char *s) {
        while (s[0]) {
            if(s[0]=='\n'){
                cy++;
                cx=0;
                this->scroll();
            }else if(s[0]=='\b'){
                cx--;
                if(cx<0){
                    cx=CONSW-1;
                    cy--;
                    if(cy<0) {cy=0; cx=0;}
                }
                lin[cy].col[cx]=0;
            }else{
                put(s[0]);
            }
            s++;
        }
        this->refresh();
    }
};
#undef DISPX
#undef DISPY
#undef DISPH
#undef DISPW
#undef CONSH
#undef CONSW
#undef FONTS

#endif
