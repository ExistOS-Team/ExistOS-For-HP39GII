

static lv_group_t *g;
static lv_obj_t *tv;
static lv_obj_t *t1;
static lv_obj_t *t2;

static lv_obj_t *lab1;
static lv_obj_t *lab2;
static lv_obj_t *lab3;

static lv_obj_t *lab_cpu_freq;
static lv_obj_t *lab_hlck_freq;

static int cur_cpu_freq = 346;
static int cur_hclk_freq = 173;

int cur_cpu_div = 1;
int cur_cpu_frac = 25;
int cur_hclk_div = 2;

int new_cpu_div = 1;
int new_cpu_frac = 25;
int new_hlck_div = 2;

void get_cur_clk() {
    uint32_t tmp[3];
    ll_get_clkctrl_div(tmp);
    cur_cpu_div = tmp[0];
    cur_cpu_frac = tmp[1];
    cur_hclk_div = tmp[2];
}

void refresh_lable() {

    lv_label_set_text_fmt(lab_cpu_freq, "CPU: %d MHz -> %d MHz",
                          480 * 18 / cur_cpu_div / cur_cpu_frac,
                          480 * 18 / new_cpu_div / new_cpu_frac);
    lv_label_set_text_fmt(lab_hlck_freq, "HCLK: %d MHz -> %d MHz",
                          (480 * 18 / cur_cpu_div) / cur_cpu_frac / cur_hclk_div,
                          (480 * 18 / new_cpu_div) / new_cpu_frac / new_hlck_div);
}

void slider_cpu_div_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    char buf[8];
    int val = (int)lv_slider_get_value(slider);
    lv_snprintf(buf, sizeof(buf), "%d", val);
    lv_label_set_text(lab1, buf);

    new_cpu_div = val;
    refresh_lable();
}

void slider_cpu_frac_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    char buf[8];
    int val = (int)lv_slider_get_value(slider);
    lv_snprintf(buf, sizeof(buf), "%d", val - 17);
    lv_label_set_text(lab2, buf);

    new_cpu_frac = val;

    refresh_lable();
}

void slider_hclk_div_event_cb(lv_event_t *e) {
    lv_obj_t *slider = lv_event_get_target(e);
    char buf[8];
    int val = (int)lv_slider_get_value(slider);
    lv_snprintf(buf, sizeof(buf), "%d", val);
    lv_label_set_text(lab3, buf);

    new_hlck_div = val;

    refresh_lable();
}

void update_btn_cb(lv_event_t *e) {
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) {

        ll_set_clkctrl_div(new_cpu_div, new_cpu_frac, new_hlck_div);
        get_cur_clk();
        refresh_lable();
        // LV_LOG_USER("Clicked");
    }
}

void SystemTest() {

    static lv_coord_t column_dsc[] = {32, 32, 64, 64, 32, 16, 16, LV_GRID_TEMPLATE_LAST};
    static lv_coord_t row_dsc[] =    {12, 12, 12, 12, 12, 12, 20, 10, LV_GRID_TEMPLATE_LAST};

    tv = lv_tabview_create(lv_scr_act(), LV_DIR_TOP, LV_DPI_DEF / 4);

    t1 = lv_tabview_add_tab(tv, "Core Freq");
    t2 = lv_tabview_add_tab(tv, "Status");

    lv_obj_set_style_grid_column_dsc_array(t1, column_dsc, 0);
    lv_obj_set_style_grid_row_dsc_array(t1, row_dsc, 0);
    lv_obj_center(t1);
    lv_obj_set_size(t1, 256, 127);
    lv_obj_set_layout(t1, LV_LAYOUT_GRID);

    //===========================CPU DIV Slider================================
    lv_obj_t *obj;
    obj = lv_label_create(t1);
    lv_label_set_text(obj, "CPU DIV:");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 2,
                         LV_GRID_ALIGN_STRETCH, 0, 1);

    obj = lv_slider_create(t1);
    lv_slider_set_range(obj, 1, 16);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 2,
                         LV_GRID_ALIGN_STRETCH, 0, 1);
    lv_obj_add_event_cb(obj, slider_cpu_div_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lab1 = lv_label_create(t1);
    lv_label_set_text(lab1, "1");
    lv_obj_set_grid_cell(lab1, LV_GRID_ALIGN_STRETCH, 4, 1,
                         LV_GRID_ALIGN_STRETCH, 0, 1);

    //===========================CPU Frac Slider================================
    obj = lv_label_create(t1);
    lv_label_set_text(obj, "CPU Frac:");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 2,
                         LV_GRID_ALIGN_STRETCH, 1, 1);

    obj = lv_slider_create(t1);
    lv_slider_set_range(obj, 18, 35);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 2,
                         LV_GRID_ALIGN_STRETCH, 1, 1);
    lv_obj_add_event_cb(obj, slider_cpu_frac_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lab2 = lv_label_create(t1);
    lv_label_set_text(lab2, "7");
    lv_obj_set_grid_cell(lab2, LV_GRID_ALIGN_STRETCH, 4, 1,
                         LV_GRID_ALIGN_STRETCH, 1, 1);

    //===========================HCLK DIV Slider================================

    obj = lv_label_create(t1);
    lv_label_set_text(obj, "HCLK DIV:");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 2,
                         LV_GRID_ALIGN_STRETCH, 2, 1);

    obj = lv_slider_create(t1);
    lv_slider_set_range(obj, 1, 12);
    lv_obj_add_flag(obj, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 2,
                         LV_GRID_ALIGN_STRETCH, 2, 1);
    lv_obj_add_event_cb(obj, slider_hclk_div_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    lab3 = lv_label_create(t1);
    lv_label_set_text(lab3, "2");
    lv_obj_set_grid_cell(lab3, LV_GRID_ALIGN_STRETCH, 4, 1,
                         LV_GRID_ALIGN_STRETCH, 2, 1);

    // ===================

    obj = lv_label_create(t1);
    lv_label_set_text(obj, "Cur Freq:  ->  New Freq:");
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 0, 4,
                         LV_GRID_ALIGN_STRETCH, 3, 1);

    lab_cpu_freq = lv_label_create(t1);
    lv_label_set_text(lab_cpu_freq, "CPU: 346.5 MHz");
    lv_obj_set_grid_cell(lab_cpu_freq, LV_GRID_ALIGN_STRETCH, 0, 4,
                         LV_GRID_ALIGN_STRETCH, 4, 1);

    lab_hlck_freq = lv_label_create(t1);
    lv_label_set_text(lab_hlck_freq, "HLCK: 172.8 MHz");
    lv_obj_set_grid_cell(lab_hlck_freq, LV_GRID_ALIGN_STRETCH, 0, 4,
                         LV_GRID_ALIGN_STRETCH, 5, 1);

    obj = lv_btn_create(t1);
    lv_obj_add_event_cb(obj, update_btn_cb, LV_EVENT_ALL, NULL);
    lv_obj_set_grid_cell(obj, LV_GRID_ALIGN_STRETCH, 2, 1,
                         LV_GRID_ALIGN_STRETCH, 6, 1);
    obj = lv_label_create(obj);
    lv_label_set_text(obj, "Update");

    get_cur_clk();
    refresh_lable();
}

