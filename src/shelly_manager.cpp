#include "shelly_manager.h"

#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#include <WiFi.h>

void ShellyManager::begin(ConfigStore *configStore) {
    configStore_ = configStore;
    devices_.clear();
    if (!configStore_) return;

    for (const auto &saved : configStore_->get().savedDevices) {
        addOrUpdate(saved, false);
    }
}

const ShellyDevice *ShellyManager::get(size_t index) const {
    return index < devices_.size() ? &devices_[index] : nullptr;
}

ShellyDevice *ShellyManager::get(size_t index) {
    return index < devices_.size() ? &devices_[index] : nullptr;
}

int ShellyManager::find(const String &host, uint8_t channel) const {
    for (size_t i = 0; i < devices_.size(); ++i) {
        if (devices_[i].host == host && devices_[i].channel == channel) return static_cast<int>(i);
    }
    return -1;
}

bool ShellyManager::addOrUpdate(const SavedShellyDevice &saved, bool persist) {
    if (saved.host.isEmpty()) return false;
    int existing = find(saved.host, saved.channel);
    ShellyDevice *device = nullptr;

    if (existing >= 0) {
        device = &devices_[existing];
    } else {
        devices_.push_back(ShellyDevice{});
        device = &devices_.back();
        device->host = saved.host;
        device->channel = saved.channel;
    }

    if (!saved.name.isEmpty()) device->name = saved.name;
    if (device->name.isEmpty()) device->name = defaultName(saved.host, saved.channel);
    device->generation = saved.generation;
    device->enabled = saved.enabled;

    if (persist && configStore_) configStore_->upsertDevice(saved);
    return true;
}

bool ShellyManager::remove(size_t index) {
    if (index >= devices_.size()) return false;
    ShellyDevice d = devices_[index];
    devices_.erase(devices_.begin() + index);
    if (configStore_) configStore_->removeDevice(d.host, d.channel);
    if (refreshCursor_ >= devices_.size()) refreshCursor_ = 0;
    return true;
}

size_t ShellyManager::discoverMdns() {
    if (WiFi.status() != WL_CONNECTED) return 0;
    size_t added = 0;

    int count = MDNS.queryService("shelly", "tcp");
    for (int i = 0; i < count; ++i) {
        String host = MDNS.IP(i).toString();
        if (host.isEmpty() || find(host, 0) >= 0) continue;

        SavedShellyDevice saved;
        saved.host = host;
        saved.name = MDNS.hostname(i);
        if (saved.name.isEmpty()) saved.name = defaultName(host, 0);
        saved.generation = 2;
        saved.channel = 0;
        saved.enabled = true;
        addOrUpdate(saved, true);
        ++added;
    }

    // Gen1 devices advertise _http._tcp. Only accept service names/hosts that look like Shelly.
    count = MDNS.queryService("http", "tcp");
    for (int i = 0; i < count; ++i) {
        String mdnsHost = MDNS.hostname(i);
        String lower = mdnsHost;
        lower.toLowerCase();
        if (!lower.startsWith("shelly")) continue;

        String host = MDNS.IP(i).toString();
        if (host.isEmpty() || find(host, 0) >= 0) continue;

        SavedShellyDevice saved;
        saved.host = host;
        saved.name = mdnsHost;
        saved.generation = 1;
        saved.channel = 0;
        saved.enabled = true;
        addOrUpdate(saved, true);
        ++added;
    }

    return added;
}

void ShellyManager::loop() {
    if (WiFi.status() != WL_CONNECTED || devices_.empty() || !configStore_) return;
    const uint32_t interval = max<uint32_t>(1000, configStore_->get().refreshIntervalMs);
    const uint32_t perDevice = max<uint32_t>(350, interval / max<size_t>(1, devices_.size()));
    if (millis() - lastRefreshMs_ < perDevice) return;

    lastRefreshMs_ = millis();
    if (refreshCursor_ >= devices_.size()) refreshCursor_ = 0;
    if (devices_[refreshCursor_].enabled) refresh(refreshCursor_);
    refreshCursor_ = (refreshCursor_ + 1) % devices_.size();
}

void ShellyManager::refreshAll() {
    for (size_t i = 0; i < devices_.size(); ++i) {
        if (devices_[i].enabled) refresh(i);
    }
}

bool ShellyManager::detect(ShellyDevice &device) {
    String payload;
    if (getJson("http://" + device.host + "/rpc/Shelly.GetDeviceInfo", payload)) {
        JsonDocument doc;
        if (!deserializeJson(doc, payload)) {
            device.generation = 2;
            device.model = String(doc["model"] | "Shelly Gen2+");
            const char *name = doc["name"] | "";
            if (device.name.isEmpty() && strlen(name)) device.name = name;
            return true;
        }
    }

    if (getJson("http://" + device.host + "/shelly", payload)) {
        JsonDocument doc;
        if (!deserializeJson(doc, payload)) {
            device.generation = 1;
            device.model = String(doc["type"] | "Shelly Gen1");
            return true;
        }
    }
    return false;
}

bool ShellyManager::refresh(size_t index) {
    ShellyDevice *device = get(index);
    if (!device || !device->enabled) return false;
    if (device->generation == 0 && !detect(*device)) {
        device->online = false;
        return false;
    }

    bool ok = device->generation >= 2 ? refreshGen2(*device) : refreshGen1(*device);
    device->online = ok;
    if (ok) device->lastSeenMs = millis();
    return ok;
}

bool ShellyManager::refreshGen2(ShellyDevice &device) {
    String payload;
    String url = "http://" + device.host + "/rpc/Switch.GetStatus?id=" + String(device.channel);
    if (!getJson(url, payload)) return false;

    JsonDocument doc;
    if (deserializeJson(doc, payload)) return false;
    if (!doc["output"].is<bool>()) return false;
    device.on = doc["output"].as<bool>();
    device.powerW = doc["apower"].is<float>() || doc["apower"].is<int>() ? doc["apower"].as<float>() : NAN;
    return true;
}

bool ShellyManager::refreshGen1(ShellyDevice &device) {
    String payload;
    String url = "http://" + device.host + "/relay/" + String(device.channel);
    if (!getJson(url, payload)) return false;

    JsonDocument doc;
    if (deserializeJson(doc, payload)) return false;
    if (!doc["ison"].is<bool>()) return false;
    device.on = doc["ison"].as<bool>();
    device.powerW = NAN;
    return true;
}

bool ShellyManager::setState(size_t index, bool on) {
    ShellyDevice *device = get(index);
    if (!device || !device->enabled) return false;
    if (device->generation == 0 && !detect(*device)) return false;

    bool ok = device->generation >= 2 ? commandGen2(*device, on) : commandGen1(*device, on ? "on" : "off");
    if (ok) refresh(index);
    return ok;
}

bool ShellyManager::toggle(size_t index) {
    ShellyDevice *device = get(index);
    if (!device || !device->enabled) return false;
    if (device->generation == 0 && !detect(*device)) return false;

    bool ok = device->generation >= 2 ? commandGen2(*device, !device->on) : commandGen1(*device, "toggle");
    if (ok) refresh(index);
    return ok;
}

bool ShellyManager::rename(size_t index, const String &name) {
    ShellyDevice *device = get(index);
    if (!device || name.isEmpty()) return false;
    device->name = name;
    if (configStore_) {
        SavedShellyDevice saved{device->host, device->name, device->generation, device->channel, device->enabled};
        return configStore_->upsertDevice(saved);
    }
    return true;
}

bool ShellyManager::commandGen2(ShellyDevice &device, bool on) {
    String payload;
    String url = "http://" + device.host + "/rpc/Switch.Set?id=" + String(device.channel) + "&on=" + (on ? "true" : "false");
    return getJson(url, payload);
}

bool ShellyManager::commandGen1(ShellyDevice &device, const char *command) {
    String payload;
    String url = "http://" + device.host + "/relay/" + String(device.channel) + "?turn=" + command;
    return getJson(url, payload);
}

bool ShellyManager::getJson(const String &url, String &payload, uint16_t timeoutMs) {
    WiFiClient client;
    HTTPClient http;
    http.setConnectTimeout(timeoutMs);
    http.setTimeout(timeoutMs);
    if (!http.begin(client, url)) return false;
    int code = http.GET();
    if (code < 200 || code >= 300) {
        http.end();
        return false;
    }
    payload = http.getString();
    http.end();
    return true;
}

String ShellyManager::defaultName(const String &host, uint8_t channel) const {
    return "Shelly " + host + (channel ? (" #" + String(channel)) : "");
}
