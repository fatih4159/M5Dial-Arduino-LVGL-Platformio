#include "ui.h"

#include <Arduino.h>
#include <WiFi.h>
#include <vector>

namespace {
ShellyManager *gShelly = nullptr;
WebUiService *gWebUi = nullptr;
MqttService *gMqtt = nullptr;

lv_group_t *gGroup = nullptr;
lv_obj_t *gHeader = nullptr;
lv_obj_t *gList = nullptr;
lv_obj_t *gFooter = nullptr;
std::vector<lv_obj_t *> gRows;
std::vector<lv_obj_t *> gStateLabels;

size_t gRenderedCount = static_cast<size_t>(-1);
bool gTogglePending = false;
size_t gToggleIndex = 0;
uint32_t gLastUiUpdate = 0;

void row_event_cb(lv_event_t *event) {
    lv_event_code_t code = lv_event_get_code(event);
    lv_obj_t *row = static_cast<lv_obj_t *>(lv_event_get_target(event));
    size_t index = reinterpret_cast<uintptr_t>(lv_event_get_user_data(event));

    if (code == LV_EVENT_CLICKED) {
        gToggleIndex = index;
        gTogglePending = true;
    } else if (code == LV_EVENT_FOCUSED) {
        lv_obj_scroll_to_view(row, LV_ANIM_ON);
    }
}

void attach_encoder_to_group() {
    for (lv_indev_t *indev = lv_indev_get_next(nullptr); indev != nullptr; indev = lv_indev_get_next(indev)) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_ENCODER) {
            lv_indev_set_group(indev, gGroup);
            return;
        }
    }
}

void rebuild_rows() {
    if (!gList || !gShelly) return;
    lv_obj_clean(gList);
    gRows.clear();
    gStateLabels.clear();
    lv_group_remove_all_objs(gGroup);

    if (gShelly->count() == 0) {
        lv_obj_t *empty = lv_label_create(gList);
        lv_label_set_text(empty, "Keine Shellys\n\nWebUI oeffnen und\nGeraete suchen");
        lv_obj_set_style_text_align(empty, LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_set_style_text_color(empty, lv_color_hex(0x9AA7B6), 0);
        lv_obj_center(empty);
        gRenderedCount = 0;
        return;
    }

    for (size_t i = 0; i < gShelly->count(); ++i) {
        const ShellyDevice *device = gShelly->get(i);
        lv_obj_t *row = lv_button_create(gList);
        lv_obj_set_size(row, 190, 58);
        lv_obj_set_style_radius(row, 14, 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x151920), 0);
        lv_obj_set_style_bg_color(row, lv_color_hex(0x1D2A20), LV_STATE_FOCUSED);
        lv_obj_set_style_border_width(row, 0, 0);
        lv_obj_set_style_border_width(row, 2, LV_STATE_FOCUSED);
        lv_obj_set_style_border_color(row, lv_color_hex(0x56D364), LV_STATE_FOCUSED);
        lv_obj_set_style_pad_left(row, 12, 0);
        lv_obj_set_style_pad_right(row, 12, 0);

        lv_obj_t *name = lv_label_create(row);
        lv_label_set_text(name, device->name.c_str());
        lv_obj_set_width(name, 120);
        lv_label_set_long_mode(name, LV_LABEL_LONG_DOT);
        lv_obj_align(name, LV_ALIGN_LEFT_MID, 0, -10);
        lv_obj_set_style_text_color(name, lv_color_hex(0xF4F7FB), 0);

        lv_obj_t *meta = lv_label_create(row);
        String info = device->host + " C" + String(device->channel);
        lv_label_set_text(meta, info.c_str());
        lv_obj_set_width(meta, 120);
        lv_label_set_long_mode(meta, LV_LABEL_LONG_DOT);
        lv_obj_align(meta, LV_ALIGN_LEFT_MID, 0, 11);
        lv_obj_set_style_text_color(meta, lv_color_hex(0x7E8998), 0);

        lv_obj_t *state = lv_label_create(row);
        lv_label_set_text(state, device->online ? (device->on ? "EIN" : "AUS") : "---");
        lv_obj_align(state, LV_ALIGN_RIGHT_MID, 0, 0);
        lv_obj_set_style_text_color(state, device->online && device->on ? lv_color_hex(0x56D364) : lv_color_hex(0x9AA7B6), 0);

        lv_obj_add_event_cb(row, row_event_cb, LV_EVENT_ALL, reinterpret_cast<void *>(i));
        lv_group_add_obj(gGroup, row);
        gRows.push_back(row);
        gStateLabels.push_back(state);
    }

    if (!gRows.empty()) lv_group_focus_obj(gRows.front());
    gRenderedCount = gShelly->count();
}

void update_rows() {
    for (size_t i = 0; i < gStateLabels.size() && i < gShelly->count(); ++i) {
        const ShellyDevice *device = gShelly->get(i);
        String state = device->online ? (device->on ? "EIN" : "AUS") : "---";
        if (device->online && !isnan(device->powerW)) state += "\n" + String(device->powerW, 0) + "W";
        lv_label_set_text(gStateLabels[i], state.c_str());
        lv_obj_set_style_text_align(gStateLabels[i], LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_color(gStateLabels[i], device->online && device->on ? lv_color_hex(0x56D364) : lv_color_hex(0x9AA7B6), 0);
    }
}
}

void ui_init(ShellyManager *shellyManager, WebUiService *webUi, MqttService *mqttService) {
    gShelly = shellyManager;
    gWebUi = webUi;
    gMqtt = mqttService;

    lv_obj_t *screen = lv_screen_active();
    lv_obj_set_style_bg_color(screen, lv_color_hex(0x0B0D10), 0);
    lv_obj_remove_flag(screen, LV_OBJ_FLAG_SCROLLABLE);

    gHeader = lv_label_create(screen);
    lv_obj_set_width(gHeader, 190);
    lv_label_set_long_mode(gHeader, LV_LABEL_LONG_DOT);
    lv_obj_align(gHeader, LV_ALIGN_TOP_MID, 0, 17);
    lv_obj_set_style_text_align(gHeader, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(gHeader, lv_color_hex(0xF4F7FB), 0);

    gList = lv_obj_create(screen);
    lv_obj_set_size(gList, 214, 150);
    lv_obj_align(gList, LV_ALIGN_CENTER, 0, 3);
    lv_obj_set_flex_flow(gList, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(gList, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_row(gList, 7, 0);
    lv_obj_set_style_pad_top(gList, 9, 0);
    lv_obj_set_style_pad_bottom(gList, 9, 0);
    lv_obj_set_style_bg_opa(gList, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(gList, 0, 0);
    lv_obj_set_scrollbar_mode(gList, LV_SCROLLBAR_MODE_OFF);

    gFooter = lv_label_create(screen);
    lv_obj_set_width(gFooter, 190);
    lv_obj_align(gFooter, LV_ALIGN_BOTTOM_MID, 0, -14);
    lv_label_set_long_mode(gFooter, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(gFooter, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(gFooter, lv_color_hex(0x7E8998), 0);

    gGroup = lv_group_create();
    lv_group_set_default(gGroup);
    attach_encoder_to_group();
    rebuild_rows();
}

void ui_loop() {
    if (!gShelly || !gWebUi || millis() - gLastUiUpdate < 350) return;
    gLastUiUpdate = millis();

    if (gRenderedCount != gShelly->count()) rebuild_rows();
    update_rows();

    String header;
    if (gWebUi->isApMode()) header = "SETUP AP";
    else header = WiFi.status() == WL_CONNECTED ? "Shelly Control  WiFi" : "Shelly Control  offline";
    if (gMqtt && gMqtt->running()) header += "  MQTT";
    lv_label_set_text(gHeader, header.c_str());

    String footer;
    if (gWebUi->isApMode()) footer = gWebUi->apSsid() + " | PW: m5dial-setup | 192.168.4.1";
    else footer = gWebUi->accessAddress();
    lv_label_set_text(gFooter, footer.c_str());
}

bool ui_take_toggle_request(size_t &deviceIndex) {
    if (!gTogglePending) return false;
    gTogglePending = false;
    deviceIndex = gToggleIndex;
    return true;
}
