//
// Created by Fatih  Tuluk on 12.02.24.
//


#include "keyborad/hidkeyboard_extension.h"
#include "vector"


extern HIDkeyboard keb;

void main_menu(void);
void create_circular_buttons(lv_obj_t *parent, std::vector<std::string> labels);
void item_select_cb(lv_event_t *e);
void button_press_cb(lv_event_t *e);
