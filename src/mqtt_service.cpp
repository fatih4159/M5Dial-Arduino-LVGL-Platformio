#include "mqtt_service.h"
#include <WiFi.h>

MqttService::~MqttService() {
    delete broker_;
}

void MqttService::begin(ConfigStore *configStore, ShellyManager *shellyManager) {
    configStore_ = configStore;
    shellyManager_ = shellyManager;
    if (!configStore_ || !shellyManager_ || !configStore_->get().mqttEnabled) return;

    const AppConfig &cfg = configStore_->get();
    broker_ = new ConfiguredMqttBroker(cfg.mqttPort, cfg.mqttUsername, cfg.mqttPassword);
    broker_->subscribe("m5dial/shelly/+/set", [this](const char *topic, const char *payload) {
        handleCommand(topic, payload);
    });
    broker_->begin();
}

void MqttService::handleCommand(const char *topic, const char *payload) {
    // m5dial/shelly/<index>/set
    static const int prefixLength = 14;
    String t(topic);
    int p1 = t.indexOf('/', prefixLength);
    if (p1 < 0) return;
    String indexPart = t.substring(prefixLength, p1);
    if (indexPart.isEmpty()) return;
    int index = indexPart.toInt();
    if (index < 0 || static_cast<size_t>(index) >= shellyManager_->count()) return;

    String action(payload);
    action.trim();
    action.toLowerCase();
    if (action != "on" && action != "off" && action != "toggle") return;

    // Do not execute HTTP from inside the MQTT callback. PicoMQTT callbacks should return quickly.
    pendingIndex_ = index;
    pendingAction_ = action;
    commandPending_ = true;
}

void MqttService::processPendingCommand() {
    if (!commandPending_) return;
    commandPending_ = false;

    bool ok = false;
    if (pendingAction_ == "toggle") ok = shellyManager_->toggle(pendingIndex_);
    else ok = shellyManager_->setState(pendingIndex_, pendingAction_ == "on");

    if (broker_) {
        String topic = "m5dial/shelly/" + String(pendingIndex_) + "/command_result";
        broker_->publish(topic.c_str(), ok ? "ok" : "error");
    }
}

void MqttService::publishStatuses() {
    if (!broker_) return;
    uint32_t interval = configStore_ ? configStore_->get().refreshIntervalMs : 3000;
    if (interval < 2000) interval = 2000;
    if (millis() - lastPublishMs_ < interval) return;
    lastPublishMs_ = millis();

    broker_->publish("m5dial/system/status", "online");
    String ip = WiFi.status() == WL_CONNECTED ? WiFi.localIP().toString() : WiFi.softAPIP().toString();
    broker_->publish("m5dial/system/ip", ip.c_str());

    for (size_t i = 0; i < shellyManager_->count(); ++i) {
        const ShellyDevice *d = shellyManager_->get(i);
        String root = "m5dial/shelly/" + String(i);
        broker_->publish((root + "/name").c_str(), d->name.c_str());
        broker_->publish((root + "/host").c_str(), d->host.c_str());
        broker_->publish((root + "/online").c_str(), d->online ? "1" : "0");
        broker_->publish((root + "/state").c_str(), d->on ? "on" : "off");
        if (!isnan(d->powerW)) {
            String power = String(d->powerW, 2);
            broker_->publish((root + "/power_w").c_str(), power.c_str());
        }
    }
}

void MqttService::loop() {
    if (!broker_) return;
    broker_->loop();
    processPendingCommand();
    publishStatuses();
}
