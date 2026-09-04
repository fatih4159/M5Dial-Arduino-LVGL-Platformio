#include <Arduino.h>
#include <WiFi.h>
#include <M5Dial.h>
#include <lvgl.h>

#include "../lib/M5Dial-LVGL-Arduino/M5Dial-LVGL.h"
#include "app_config.h"
#include "mqtt_service.h"
#include "shelly_manager.h"
#include "ui.h"
#include "web_ui.h"

ConfigStore configStore;
ShellyManager shellyManager;
WebUiService webUi;
MqttService mqttService;

void setup() {
    Serial.begin(115200);
    m5dial_lvgl_init();

    configStore.begin();
    shellyManager.begin(&configStore);
    webUi.begin(&configStore, &shellyManager);

    if (WiFi.status() == WL_CONNECTED) {
        shellyManager.discoverMdns();
        shellyManager.refreshAll();
    }

    mqttService.begin(&configStore, &shellyManager);
    ui_init(&shellyManager, &webUi, &mqttService);

    Serial.println("M5 Dial Shelly Control ready");
    Serial.println("Web UI: " + webUi.accessAddress());
    if (webUi.isApMode()) {
        Serial.println("Setup AP: " + webUi.apSsid());
        Serial.println("Setup password: m5dial-setup");
    }
}

void loop() {
    webUi.loop();
    mqttService.loop();
    shellyManager.loop();

    size_t toggleIndex = 0;
    if (ui_take_toggle_request(toggleIndex)) {
        shellyManager.toggle(toggleIndex);
    }

    ui_loop();
    m5dial_lvgl_next();
}
