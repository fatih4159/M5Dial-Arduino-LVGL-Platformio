#include "main_menu.h"
#include "../helper/KebCom.h"

LV_IMAGE_DECLARE(chrome);
LV_IMAGE_DECLARE(img_apple);
LV_IMAGE_DECLARE(img_windows);

lv_obj_t *panel;
lv_obj_t *roller;
lv_obj_t *roller_panel;
lv_obj_t *selectionCircle;
lv_obj_t *selectionLabel;
lv_group_t *main_group;
lv_obj_t *icon;

std::vector<std::string> app_index = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L"};

std::map<std::string, lv_img_dsc_t> app_imageMap;

std::map<std::string, std::string> app_description;
HIDkeyboard keb_main;

void main_menu()
{
    app_imageMap["A"] = img_apple;
    app_imageMap["B"] = img_windows;
    app_imageMap["C"] = img_windows;
    app_imageMap["L"] = img_apple;

    app_description["A"] = "Apple Macros";
    app_description["B"] = "Windows Macros";
    app_description["C"] = "Teams Macros";
    app_description["L"] = "Download Mode";

    keb_main.begin();

    //  make the background black
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

    // // create a circle from the center of the screen
    // selectionCircle = lv_obj_create(panel);
    // lv_obj_set_size(selectionCircle, lv_pct(50), lv_pct(50));
    // lv_obj_set_style_radius(selectionCircle, LV_RADIUS_CIRCLE, 0);
    // lv_obj_remove_flag(panel, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_bg_color(selectionCircle, lv_color_hex(0x00FF00), LV_PART_MAIN);
    // lv_obj_align(selectionCircle, LV_ALIGN_CENTER, 0, 0);

    selectionLabel = lv_label_create(panel);
    lv_label_set_text(selectionLabel, "A");
    lv_obj_set_style_text_color(selectionLabel, lv_color_white(), 0);
    // make the selection label scroll automatically if it's too long
    lv_label_set_long_mode(selectionLabel, LV_LABEL_LONG_SCROLL_CIRCULAR);

    lv_obj_set_style_text_font(selectionLabel, &lv_font_montserrat_24, 0);
    lv_obj_set_width(selectionLabel, LV_PCT(60));

    lv_obj_align(selectionLabel, LV_ALIGN_CENTER, 0, 0);

    // create_circular_buttons(panel, labels);
    create_circular_buttons(panel);
    lv_group_add_obj(main_group, panel);

    for (lv_indev_t *indev = lv_indev_get_next(nullptr); indev != nullptr; indev = lv_indev_get_next(indev))
    {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER)
        {
            lv_indev_set_group(indev, main_group);
            break;
        }
    }

    lv_group_focus_obj(lv_group_get_focused(main_group));

    lv_obj_add_event_cb(panel, item_select_cb, LV_EVENT_ALL, nullptr);
}

void create_circular_buttons(lv_obj_t *parent)
{
    int numButtons = app_index.size();
    int angleStep = 360 / numButtons;
    int centerxy = 85;
    int radius = 98;
    int movement = 7;
    int transformSize = 2;
    int i = 0;
    for (auto key : app_index)
    {

        int angle = i * angleStep - 90; // Subtract 90 degrees to start at 12 o'clock
        int x = centerxy + radius * cos(angle * M_PI / 180);
        int y = centerxy + radius * sin(angle * M_PI / 180);

        int centerx = (x - centerxy) / movement;
        int centery = (y - centerxy) / movement;

        int padding = 12;
        int size = (20 + angleStep) - padding;

        lv_obj_t *button = lv_button_create(parent);
        lv_obj_set_pos(button, x, y);
        lv_obj_set_size(button, size, size);
        lv_obj_set_style_radius(button, LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_bg_color(button, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_color(button, lv_color_hex(0x00FF00), LV_PART_MAIN);
        lv_obj_set_style_translate_x(button, -centerx, LV_STATE_FOCUSED);
        lv_obj_set_style_translate_y(button, -centery, LV_STATE_FOCUSED);
        lv_obj_set_style_transform_width(button, transformSize, LV_STATE_FOCUSED);
        lv_obj_set_style_transform_height(button, transformSize, LV_STATE_FOCUSED);

        lv_obj_t *icn_cont = lv_obj_create(button);
        lv_obj_remove_flag(icn_cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_remove_flag(icn_cont, LV_OBJ_FLAG_CLICK_FOCUSABLE);
        lv_obj_set_style_radius(icn_cont, LV_RADIUS_CIRCLE, 0);

        lv_obj_set_style_bg_color(icn_cont, lv_color_hex(0x008000), LV_PART_MAIN);

        lv_obj_set_style_border_color(icn_cont, lv_color_hex(0x005300), 0);

        lv_obj_set_size(icn_cont, size, size);
        lv_obj_align(icn_cont, LV_ALIGN_CENTER, 0, 0);
        lv_obj_set_style_bg_color(button, lv_color_black(), LV_PART_MAIN);
        // set the background color to green
        lv_obj_set_style_bg_color(icn_cont, lv_color_black(), LV_PART_MAIN);

        icon = lv_image_create(icn_cont);
        lv_image_set_src(icon, &app_imageMap[key]);
        lv_obj_align(icon, LV_ALIGN_CENTER, 0, 0);

        // change the border color to green if is focused

        lv_obj_t *label = lv_label_create(button);
        lv_obj_set_style_text_color(label, lv_color_black(), 0);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
        // lv_label_set_text(label, labels[i].c_str());
        lv_label_set_text(label, key.c_str());

        lv_group_add_obj(main_group, button);
        // lv_obj_add_event_cb(button, button_press_cb, LV_EVENT_ALL, nullptr);
        lv_obj_add_event_cb(button, button_press_cb, LV_EVENT_ALL, nullptr);
        i++;
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
        // handle touch
        if (indev_type == LV_INDEV_TYPE_POINTER)
        {
            lv_group_focus_obj(lv_group_get_focused(main_group));
        }
    }
}

void showButtonCarousel(std::vector<std::string> options, lv_group_t *main_group)
{
    // clear the screen
    // lv_obj_clean(lv_scr_act());
    //  get the first child from the screen
    lv_obj_t *child = lv_obj_get_child(lv_scr_act(), 0);
    // hide the child
    lv_obj_add_flag(child, LV_OBJ_FLAG_HIDDEN);

    roller_panel = lv_obj_create(lv_screen_active());

    

    lv_obj_set_size(roller_panel, lv_obj_get_width(lv_screen_active()), lv_obj_get_height(lv_screen_active()));
    lv_obj_align(roller_panel, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_border_color(roller_panel, lv_color_black(), 0);
    lv_obj_set_style_radius(roller_panel, LV_RADIUS_CIRCLE, LV_PART_MAIN);

    lv_obj_set_style_bg_color(roller_panel, lv_color_black(), LV_PART_MAIN);

    lv_obj_set_scroll_snap_y(roller_panel, LV_SCROLL_SNAP_CENTER);

    lv_obj_set_flex_flow(roller_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(roller_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    // add a back button to the list 
    options.insert(options.begin(), "back");

 
    for (const std::string &option : options)
    {
        lv_obj_t *button = lv_btn_create(roller_panel);
        lv_obj_set_size(button, lv_pct(80), LV_SIZE_CONTENT);

        lv_obj_t *label = lv_label_create(button);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
        lv_label_set_text(label, option.c_str());
        lv_obj_set_style_text_font(label, &lv_font_montserrat_24, 0);
        lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);


        lv_group_add_obj(main_group, button);
        lv_obj_add_event_cb(button, button_carousel_cb, LV_EVENT_ALL, nullptr);
    }
    //focus on the second button
    lv_group_focus_obj(lv_obj_get_child(roller_panel, 0));

}
static void button_carousel_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *obj = lv_event_get_target_obj(e);
    if (code == LV_EVENT_CLICKED)
    {
        lv_obj_t *obj = static_cast<lv_obj_t *>(lv_event_get_target(e));
        const char *label = lv_label_get_text(lv_obj_get_child(obj, 0));
        if (strcmp(label, "back") == 0)
        {
            lv_obj_remove_flag(panel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_add_flag(roller_panel, LV_OBJ_FLAG_HIDDEN);
            lv_obj_clean(roller_panel);
        }
        if (strcmp(label, "Toggle Mic") == 0)
        {
            keb_main.sendPress(HID_KEY_CONTROL_LEFT);
            keb_main.sendPress(HID_KEY_SHIFT_LEFT);
            delay(800);
            keb_main.sendPress(HID_KEY_M);
            delay(100);
            keb_main.sendRelease();
        }
        if (strcmp(label, "Toggle Cam") == 0)
        {
            keb_main.sendPress(HID_KEY_CONTROL_LEFT);
            keb_main.sendPress(HID_KEY_SHIFT_LEFT);
            delay(800);
            keb_main.sendPress(HID_KEY_O);
            delay(100);
            keb_main.sendRelease();
        }
        if (strcmp(label, "Toggle Hand") == 0)
        {
            // keb_main.sendPress(HID_KEY_CONTROL_LEFT);
            // delay(100);
            // keb_main.sendPress(HID_KEY_SHIFT_LEFT);
            // delay(500);
            // keb_main.sendPress(HID_KEY_K);
            // delay(100);
            // keb_main.sendRelease();
        }
        if (strcmp(label, "Launch Intellij MAC") == 0)
        {
            KebCom::mac_launch("intellij", keb_main);
        }
        if (strcmp(label, "Launch Teams MAC") == 0)
        {
            KebCom::mac_launch("teams", keb_main);
        }
        if (strcmp(label, "Launch Teams WIN") == 0)
        {
            KebCom::win_launch("teams", keb_main);
        }
        if (strcmp(label, "Launch Intellij WIN") == 0)
        {
            KebCom::win_launch("intellij", keb_main);
        }

        
    }
}

void navigateBack()
{
    lv_obj_remove_flag(panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(roller_panel, LV_OBJ_FLAG_HIDDEN);
    lv_obj_clean(roller_panel);
}
void showDialog()
{
    lv_obj_t *mbox1 = lv_msgbox_create(lv_scr_act());
    lv_msgbox_add_text(mbox1, "Hello world!");
    lv_obj_align(mbox1, LV_ALIGN_CENTER, 0, 0);
}
static void button_press_cb(lv_event_t *e)
{
    if (lv_event_get_code(e) == LV_EVENT_FOCUSED)
    {
        lv_obj_t *obj = static_cast<lv_obj_t *>(lv_event_get_target(e));
        const char *label = lv_label_get_text(lv_obj_get_child(obj, 1));
        if (app_description.find(label) != app_description.end())
        {
            lv_label_set_text(selectionLabel, app_description[label].c_str());
        }
        else
        {
            lv_label_set_text(selectionLabel, label);
        }
    }

    if (lv_event_get_code(e) == LV_EVENT_CLICKED)
    {
        lv_obj_t *obj = static_cast<lv_obj_t *>(lv_event_get_target(e));
        const char *label = lv_label_get_text(lv_obj_get_child(obj, 1));

        if (label[0] == 'A')
        {
            std::vector<std::string> options = {"Launch Intellij MAC", "Launch Teams MAC"};
            showButtonCarousel(options, main_group);   
        }
        if (label[0] == 'B')
        {
            std::vector<std::string> options = {"Launch Teams WIN", "Launch Intellij WIN"};
            showButtonCarousel(options, main_group);        
        }
        if (label[0] == 'C')
        {
            std::vector<std::string> options = {"Toggle Mic", "Toggle Cam", "Toggle Hand", "Launch Teams"};
            showButtonCarousel(options, main_group);  
        }
        if (label[0] == 'L')
        {
            // lv_obj_clean(panel);
            //M5Dial.Display.sleep();
            REG_WRITE(RTC_CNTL_OPTION1_REG, RTC_CNTL_FORCE_DOWNLOAD_BOOT);
            esp_restart();
            
        }
    }
}