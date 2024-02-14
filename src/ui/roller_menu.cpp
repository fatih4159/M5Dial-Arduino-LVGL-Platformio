#include "roller_menu.h"

lv_obj_t *showFunctionRoller(std::vector<std::string> options, lv_event_cb_t function_roller_cb, lv_group_t *main_group)
{
    //convert vector to string
    std::string optionsString;
    for (int i = 0; i < options.size(); i++)
    {
        optionsString += options[i];
        if (i != options.size() - 1)
        {
            optionsString += "\n";
        }
    }
    // add also back to the first option to make it infinite
    optionsString += "\nback\n";

    //clear the screen
    lv_obj_clean(lv_scr_act());

    lv_obj_t *panel = lv_obj_create(lv_screen_active());

    lv_obj_set_size(panel, lv_obj_get_width(lv_screen_active()), lv_obj_get_height(lv_screen_active()));
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_color(panel, lv_color_black(), 0);
    lv_obj_set_style_radius(panel, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(panel, lv_color_black(), LV_PART_MAIN);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    // fill the whole screen with a roller
    lv_obj_t *roller = lv_roller_create(lv_scr_act());
    lv_roller_set_options(roller, optionsString.c_str(), LV_ROLLER_MODE_INFINITE);
    lv_obj_set_size(roller, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_align(roller, LV_ALIGN_CENTER, 0, 0);
    lv_group_add_obj(main_group, roller);
    lv_group_focus_obj(roller);

    // set the focus to the first button
    lv_group_focus_obj(lv_group_get_focused(main_group));
    lv_obj_add_event_cb(roller, function_roller_cb, LV_EVENT_ALL, nullptr);
    return panel;
}
