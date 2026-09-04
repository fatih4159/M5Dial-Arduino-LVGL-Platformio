#include "app_config.h"
#include <ArduinoJson.h>

bool ConfigStore::begin() {
    if (!preferences_.begin("shellydial", false)) return false;
    config_.hostname = preferences_.getString("hostname", "m5dial-shelly");
    config_.wifiSsid = preferences_.getString("wifi_ssid", "");
    config_.wifiPassword = preferences_.getString("wifi_pass", "");
    config_.mqttEnabled = preferences_.getBool("mqtt_en", true);
    config_.mqttPort = preferences_.getUShort("mqtt_port", 1883);
    config_.refreshIntervalMs = preferences_.getULong("refresh_ms", 3000);
    config_.setupApPassword = preferences_.getString("ap_pass", "m5dial-setup");
    loadDevices(preferences_.getString("devices", "[]"));
    return true;
}

bool ConfigStore::save() {
    bool ok = true;
    ok &= preferences_.putString("hostname", config_.hostname) > 0;
    preferences_.putString("wifi_ssid", config_.wifiSsid);
    preferences_.putString("wifi_pass", config_.wifiPassword);
    preferences_.putBool("mqtt_en", config_.mqttEnabled);
    preferences_.putUShort("mqtt_port", config_.mqttPort);
    preferences_.putULong("refresh_ms", config_.refreshIntervalMs);
    preferences_.putString("ap_pass", config_.setupApPassword);
    preferences_.putString("devices", serializeDevices());
    return ok;
}

void ConfigStore::clearWifi() {
    config_.wifiSsid = "";
    config_.wifiPassword = "";
    save();
}

bool ConfigStore::upsertDevice(const SavedShellyDevice &device) {
    for (auto &item : config_.savedDevices) {
        if (item.host == device.host && item.channel == device.channel) {
            item = device;
            return save();
        }
    }
    config_.savedDevices.push_back(device);
    return save();
}

bool ConfigStore::removeDevice(const String &host, uint8_t channel) {
    for (auto it = config_.savedDevices.begin(); it != config_.savedDevices.end(); ++it) {
        if (it->host == host && it->channel == channel) {
            config_.savedDevices.erase(it);
            return save();
        }
    }
    return false;
}

bool ConfigStore::loadDevices(const String &json) {
    config_.savedDevices.clear();
    JsonDocument doc;
    if (deserializeJson(doc, json)) return false;
    for (JsonObject item : doc.as<JsonArray>()) {
        SavedShellyDevice d;
        d.host = item["host"] | "";
        d.name = item["name"] | "Shelly";
        d.generation = item["generation"] | 0;
        d.channel = item["channel"] | 0;
        d.enabled = item["enabled"] | true;
        if (!d.host.isEmpty()) config_.savedDevices.push_back(d);
    }
    return true;
}

String ConfigStore::serializeDevices() const {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (const auto &d : config_.savedDevices) {
        JsonObject item = arr.add<JsonObject>();
        item["host"] = d.host;
        item["name"] = d.name;
        item["generation"] = d.generation;
        item["channel"] = d.channel;
        item["enabled"] = d.enabled;
    }
    String out;
    serializeJson(doc, out);
    return out;
}
