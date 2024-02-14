#include <lvgl.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "../keyborad/hidkeyboard_extension.h"


lv_obj_t *showFunctionRoller(std::vector<std::string> options, lv_event_cb_t function_roller_cb, lv_group_t *main_group);