#include "./app_type.hpp"
#include "../../include/images/img_apple.c"
#include "helper/KebCom.h"

extern app_container_t launch_intellij_app(
    2,
    "hello",
    img_apple,
    [](lv_event_t *e)
    {
            KebCom::mac_launch("intellij");
    });