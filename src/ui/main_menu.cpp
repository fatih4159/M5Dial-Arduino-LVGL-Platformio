#include "main_menu.h"
#include "../../include/images/chrome.c"
#include "../../include/images/img_apple.c"

LV_IMAGE_DECLARE(chrome);
LV_IMAGE_DECLARE(img_apple);

lv_obj_t *panel;
lv_obj_t *selectionCircle;
lv_obj_t *selectionLabel;
lv_group_t *main_group;
lv_obj_t *roller1;
lv_obj_t *apple_icon;



std::vector<std::string> labels = {"A", "B", "C", "D", "G", "H", "I", "J"};
HIDkeyboard keb_main;
void main_menu()
{
    keb_main.begin();
    // make the background black
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(lv_screen_active(), LV_SCROLLBAR_MODE_OFF);

    main_group = lv_group_create();

    panel = lv_obj_create(lv_screen_active());

    lv_obj_set_size(panel, lv_obj_get_width(lv_screen_active()), lv_obj_get_height(lv_screen_active()));
    lv_obj_align(panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_color(panel, lv_color_black(), 0);
    lv_obj_set_style_radius(panel, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_scrollbar_mode(panel, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(panel, lv_color_black(), LV_PART_MAIN);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_CLICK_FOCUSABLE);

    // create a circle form the center of the screen
    selectionCircle = lv_obj_create(panel);
    lv_obj_set_size(selectionCircle, lv_pct(50), lv_pct(50));
    lv_obj_set_style_radius(selectionCircle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(selectionCircle, lv_color_hex(0x00FF00), LV_PART_MAIN);
    lv_obj_align(selectionCircle, LV_ALIGN_CENTER, 0, 0);
    selectionLabel = lv_label_create(selectionCircle);
    lv_label_set_text(selectionLabel, "A");
    lv_obj_set_style_text_color(selectionLabel, lv_color_black(), 0);
    lv_obj_align(selectionLabel, LV_ALIGN_CENTER, 0, 0);

    // create_circular_buttons_with_images(panel, images);
    create_circular_buttons(panel, labels);
    lv_group_add_obj(main_group, panel);

    // lv_group_set_default(group);
    // lv_group_set_editing(group, true);

    for (lv_indev_t *indev = lv_indev_get_next(nullptr); indev != nullptr; indev = lv_indev_get_next(indev))
    {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER)
        {
            lv_indev_set_group(indev, main_group);
            break;
        }
    }
    // set the focus to the first button
    lv_group_focus_obj(lv_group_get_focused(main_group));

    lv_obj_add_event_cb(panel, item_select_cb, LV_EVENT_ALL, nullptr);
}

void create_circular_buttons(lv_obj_t *parent, std::vector<std::string> labels)
{
    int numButtons = labels.size();
    int angleStep = 360 / numButtons;
    int centerxy = 85;
    int radius = 89;
    int movement = 7;
    int transformSize = 2;

    for (int i = 0; i < numButtons; i++)
    {
        int angle = i * angleStep - 90; // Subtract 90 degrees to start at 12 o'clock
        int x = centerxy + radius * cos(angle * M_PI / 180);
        int y = centerxy + radius * sin(angle * M_PI / 180);

        // x position of the center from the button
        int centerx = (x - centerxy) / movement;
        // y position of the center from the button
        int centery = (y - centerxy) / movement;

        int padding = 12;

        // calculate button size from the angle step
        int size = (20 + angleStep) - padding;

        lv_obj_t *button = lv_imagebutton_create(parent);
        lv_obj_t *icn_cont = lv_obj_create(button);
        lv_obj_remove_flag(icn_cont, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_set_size(icn_cont, size, size);
        apple_icon = lv_image_create(icn_cont);
        lv_image_set_src(apple_icon, &img_apple);
        lv_obj_align(apple_icon,LV_ALIGN_CENTER,0,0);


        //set image opacity



        lv_obj_set_pos(button, x, y);

        lv_obj_set_size(button, size, size);
        // make the button round
        lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_radius(icn_cont, LV_RADIUS_CIRCLE, 0);

        // change button color to dark green
        lv_obj_set_style_bg_color(icn_cont,  lv_color_black(), LV_PART_MAIN);
        //lv_obj_set_style_border_color(button,lv_color_white(),LV_STATE_ANY);
        //lv_obj_set_style_border_width(button,2,LV_STATE_ANY);

        // transition to the center of the screen when focused
        lv_obj_set_style_translate_x(button, -centerx, LV_STATE_FOCUSED);
        lv_obj_set_style_translate_y(button, -centery, LV_STATE_FOCUSED);
        lv_obj_set_style_transform_width(button, transformSize, LV_STATE_FOCUSED);
        lv_obj_set_style_transform_height(button, transformSize, LV_STATE_FOCUSED);

        lv_obj_t *label = lv_label_create(button);
        // set label text color to black
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_align(label,LV_ALIGN_CENTER,0,0);
        lv_obj_add_flag(label,LV_OBJ_FLAG_HIDDEN);

        lv_label_set_text(label, labels[i].c_str());
        lv_group_add_obj(main_group, button);
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

    // get the selected button label and set it to the selection label

    if (code == LV_EVENT_VALUE_CHANGED)
    {

        if (indev_type == LV_INDEV_TYPE_ENCODER)
        {
            main_group = lv_obj_get_group(buttons);
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

static void function_roller_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        lv_obj_t *roller = static_cast<lv_obj_t *>(lv_event_get_target(e));

        // set the focus to the first button

        char *buff;
        lv_roller_get_selected_str(roller, buff, 0);

        switch (buff[0])
        {
        case 'back':
            lv_obj_clean(lv_scr_act());
            main_menu();
            break;
        }
    }
}

static void button_press_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_FOCUSED)
    {
        lv_obj_t *obj = static_cast<lv_obj_t *>(lv_event_get_target(e));
        const char *label = lv_label_get_text(lv_obj_get_child(obj, 1));
        if(label[0] == 'J')
        {
            lv_label_set_text(selectionLabel, "Download Mode");
        }
        else
        {
            lv_label_set_text(selectionLabel, "");
        }
    }
    // check if the event is a button press
    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        // get the label of the button
        lv_obj_t *obj = static_cast<lv_obj_t *>(lv_event_get_target(e));
        const char *label = lv_label_get_text(lv_obj_get_child(obj, 1));

        if (label[0] == 'A')
        {
            // showDialog();
            keb_main.sendString("Hello World");
        }
        if (label[0] == 'B')
        {
            roller1 = showFunctionRoller(labels, function_roller_cb, main_group);
        }
        if(label[0] == 'J')
        {
            lv_obj_clean(panel);
            M5Dial.Display.sleep();
            REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
            delay(100);
            esp_restart();
        }
    }
}