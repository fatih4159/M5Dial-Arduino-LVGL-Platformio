#include "roller_menu.h"

void showFunctionRoller(std::vector<std::string> options, lv_event_cb_t function_roller_cb, lv_group_t *main_group)
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

    // fill the whole screen with a roller
    lv_obj_t *roller = lv_roller_create(lv_scr_act());
    lv_roller_set_options(roller, optionsString.c_str(), LV_ROLLER_MODE_INFINITE);
    lv_obj_set_size(roller, lv_obj_get_width(lv_scr_act()), lv_obj_get_height(lv_scr_act()));
    lv_obj_align(roller, LV_ALIGN_CENTER, 0, 0);
    lv_group_add_obj(main_group, roller);
    lv_group_focus_obj(roller);

    // set the focus to the first button
    lv_group_focus_obj(lv_group_get_focused(main_group));

}
