#include <lvgl.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "roller_menu.h"
#include "../keyborad/hidkeyboard_extension.h"
#include <M5Dial.h>



void main_menu();
void create_circular_buttons(lv_obj_t *parent, std::vector<std::string> labels);
void item_select_cb(lv_event_t *e);
static void button_press_cb(lv_event_t *e);