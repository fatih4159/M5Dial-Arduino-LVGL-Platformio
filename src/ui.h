//
// Created by Fatih  Tuluk on 12.02.24.
//


#include "ui/main_menu.h"
#include <stdlib.h>
#include <string>
#include <lvgl.h>
#include "keyborad/hidkeyboard_extension.h"



void ui_init();
void create_circular_buttons(lv_obj_t *parent, std::vector<std::string> labels);
void item_select_cb(lv_event_t *e);
static void button_press_cb(lv_event_t *e);
