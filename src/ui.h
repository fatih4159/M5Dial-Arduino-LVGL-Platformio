#pragma once

#include <lvgl.h>
#include "mqtt_service.h"
#include "shelly_manager.h"
#include "web_ui.h"

void ui_init(ShellyManager *shellyManager, WebUiService *webUi, MqttService *mqttService);
void ui_loop();
bool ui_take_toggle_request(size_t &deviceIndex);
