#include "lvgl.h"

struct app_container_t {
    int index;
    const char* label;
    lv_img_dsc_t image;
    void (*app_func)(lv_event_t *e);
    
    app_container_t(int i = 0, const char* lbl = nullptr, lv_img_dsc_t img = {}, void (*func)(lv_event_t *e) = nullptr)
        : index(i), label(lbl), image(img), app_func(func) {}
};
