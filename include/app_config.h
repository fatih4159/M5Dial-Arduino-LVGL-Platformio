#pragma once

#include <Arduino.h>
#include <Preferences.h>
#include <vector>

struct SavedShellyDevice {
    String host;
    String name;
    uint8_t generation = 0; // 0 = auto, 1 = Gen1, 2 = Gen2+
    uint8_t channel = 0;
    bool enabled = true;
};

struct AppConfig {
    String hostname = "m5dial-shelly";
    String wifiSsid;
    String wifiPassword;
    bool mqttEnabled = true;
    uint16_t mqttPort = 1883;
    uint32_t refreshIntervalMs = 3000;
    String setupApPassword = "m5dial-setup";
    std::vector<SavedShellyDevice> savedDevices;
};

class ConfigStore {
public:
    bool begin();
    const AppConfig &get() const { return config_; }
    AppConfig &edit() { return config_; }
    bool save();
    void clearWifi();
    bool upsertDevice(const SavedShellyDevice &device);
    bool removeDevice(const String &host, uint8_t channel);

private:
    bool loadDevices(const String &json);
    String serializeDevices() const;

    Preferences preferences_;
    AppConfig config_;
};
