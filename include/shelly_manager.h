#pragma once

#include <Arduino.h>
#include <vector>
#include "app_config.h"

struct ShellyDevice {
    String host;
    String name;
    String model;
    uint8_t generation = 0;
    uint8_t channel = 0;
    bool enabled = true;
    bool online = false;
    bool on = false;
    float powerW = NAN;
    uint32_t lastSeenMs = 0;
};

class ShellyManager {
public:
    void begin(ConfigStore *configStore);
    void loop();
    size_t count() const { return devices_.size(); }
    const ShellyDevice *get(size_t index) const;
    ShellyDevice *get(size_t index);

    int find(const String &host, uint8_t channel) const;
    bool addOrUpdate(const SavedShellyDevice &device, bool persist = true);
    bool remove(size_t index);
    size_t discoverMdns();
    bool refresh(size_t index);
    bool setState(size_t index, bool on);
    bool toggle(size_t index);
    bool rename(size_t index, const String &name);
    void refreshAll();

private:
    bool detect(ShellyDevice &device);
    bool refreshGen1(ShellyDevice &device);
    bool refreshGen2(ShellyDevice &device);
    bool commandGen1(ShellyDevice &device, const char *command);
    bool commandGen2(ShellyDevice &device, bool on);
    bool getJson(const String &url, String &payload, uint16_t timeoutMs = 1200);
    String defaultName(const String &host, uint8_t channel) const;

    ConfigStore *configStore_ = nullptr;
    std::vector<ShellyDevice> devices_;
    uint32_t lastRefreshMs_ = 0;
    size_t refreshCursor_ = 0;
};
