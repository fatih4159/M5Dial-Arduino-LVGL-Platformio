//
// Created by Fatih  Tuluk on 12.02.24.
//

#include <Arduino.h>
#include "M5Dial-LVGL.h"
#include <M5Dial.h>
#include <lvgl.h>
#include "ui.h"


lv_group_t *group;
lv_obj_t *panel;
std::vector<std::string> labels = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"};
int radius;
int numButtons = labels.size();
int angleStep = 360 / numButtons;

void main_menu(void)
{

    // make the background black
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    panel = lv_obj_create(lv_screen_active());
    group = lv_group_create();

    lv_obj_set_size(panel, lv_obj_get_width(lv_screen_active()), lv_obj_get_height(lv_screen_active()));
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);

    lv_obj_set_style_border_color(panel, lv_color_black(), 0);
    // make the panel round
    lv_obj_set_style_radius(panel, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    // make it non scrollable
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(panel, lv_color_black(), LV_PART_MAIN);
    // make the border color black
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    // create a list of labels
    // std::vector<lv_img_dsc_t> images = {img_compass,img_compass,img_compass,img_compass,img_compass};

    // create_circular_buttons_with_images(panel, images);
    create_circular_buttons(panel, labels);
    lv_group_add_obj(group, panel);

    // lv_group_set_default(group);
    // lv_group_set_editing(group, true);

    for (lv_indev_t *indev = lv_indev_get_next(nullptr); indev != nullptr; indev = lv_indev_get_next(indev))
    {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER)
        {
            lv_indev_set_group(indev, group);
            break;
        }
    }
    // set the focus to the first button
    lv_group_focus_obj(lv_group_get_focused(group));

    lv_obj_add_event_cb(panel, item_select_cb, LV_EVENT_ALL, nullptr);
}
void create_circular_buttons(lv_obj_t *parent, std::vector<std::string> labels)
{
    int centerxy = 85;
    int radius = 89;

    for (int i = 0; i < numButtons; i++)
    {
        int angle = i * angleStep - 90; // Subtract 90 degrees to start at 12 o'clock
        int x = centerxy + radius * cos(angle * M_PI / 180);
        int y = centerxy + radius * sin(angle * M_PI / 180);

        // x position of the center from the button
        int centerx = (x - centerxy) / 1;
        // y position of the center from the button
        int centery = (y - centerxy) / 1;

        int padding = 12;

        // calculate button size from the angle step
        int size = (20 + angleStep) - padding;

        lv_obj_t *button = lv_btn_create(parent);
        lv_obj_set_pos(button, x, y);

        lv_obj_set_size(button, size, size);
        // make the button round
        lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);

        // change button color to dark green
        lv_obj_set_style_bg_color(button, lv_color_hex(0x00FF00), LV_PART_MAIN);

        // transition to the center of the screen when focused
        lv_obj_set_style_translate_x(button, -centerx, LV_STATE_FOCUSED);
        lv_obj_set_style_translate_y(button, -centery, LV_STATE_FOCUSED);
        lv_obj_set_style_transform_width(button,20,LV_STATE_FOCUSED);
        lv_obj_set_style_transform_height(button,20,LV_STATE_FOCUSED);

        lv_obj_t *label = lv_label_create(button);
        lv_label_set_text(label, labels[i].c_str());
        lv_group_add_obj(group, button);
        lv_obj_add_event_cb(button, button_press_cb, LV_EVENT_ALL, nullptr);
    }
}
void item_select_cb(lv_event_t *e)
{
    lv_indev_t *indev = lv_indev_active();
    if (indev == nullptr)
        return;
    lv_indev_type_t indev_type = lv_indev_get_type(indev);

    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *buttons = (lv_obj_t *)lv_event_get_target(e);

    if (code == LV_EVENT_VALUE_CHANGED)
    {

        // focus the next or the previous object according to the rotation
        if (indev_type == LV_INDEV_TYPE_ENCODER)
        {
            group = lv_obj_get_group(buttons);
            int diff = lv_indev_get_key(indev);

            if (diff > 0)
            {
                lv_group_focus_next(lv_obj_get_group(buttons));
            }
            else
            {
                lv_group_focus_prev(lv_obj_get_group(buttons));
            }
        }
    }
}
void button_press_cb(lv_event_t *e)
{

    // find out which button was pressed and do something in switch case
    lv_obj_t *obj = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(obj,0);
    const char *text = lv_label_get_text(label);

    Serial.println(text);

}