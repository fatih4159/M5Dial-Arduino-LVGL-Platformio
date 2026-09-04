#pragma once

#include <Arduino.h>
#include <DNSServer.h>
#include <WebServer.h>
#include "app_config.h"
#include "shelly_manager.h"

class WebUiService {
public:
    WebUiService();
    void begin(ConfigStore *configStore, ShellyManager *shellyManager);
    void loop();

    bool isApMode() const { return apMode_; }
    String apSsid() const { return apSsid_; }
    String apPassword() const;
    String accessAddress() const;

private:
    void setupRoutes();
    void startAccessPoint();
    void startMdns();
    void scheduleRestart(uint32_t delayMs = 700);
    void sendJson(int code, const String &json);

    String statusJson() const;
    String devicesJson() const;
    String wifiScanJson();

    WebServer server_;
    DNSServer dns_;
    ConfigStore *configStore_ = nullptr;
    ShellyManager *shellyManager_ = nullptr;
    bool apMode_ = false;
    bool restartRequested_ = false;
    uint32_t restartAtMs_ = 0;
    String apSsid_;
};
