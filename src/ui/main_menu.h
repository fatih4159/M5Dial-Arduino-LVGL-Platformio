#include <lvgl.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "../keyborad/hidkeyboard_extension.h"
#include <M5Dial.h>
#include <map>
#include "../../include/images/chrome.c"
#include "../../include/images/img_apple.c"
#include "../../include/images/img_windows.c"


void main_menu();
void create_circular_buttons(lv_obj_t *parent);
void item_select_cb(lv_event_t *e);
static void button_press_cb(lv_event_t *e);
void showDialog();
static void button_carousel_cb(lv_event_t *e);
void showButtonCarousel(std::vector<std::string> options, lv_group_t *main_group);
