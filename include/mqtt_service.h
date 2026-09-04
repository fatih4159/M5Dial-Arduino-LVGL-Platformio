#pragma once

#include <Arduino.h>
#include <PicoMQTT.h>
#include "app_config.h"
#include "shelly_manager.h"

class ConfiguredMqttBroker : public PicoMQTT::Server {
public:
    ConfiguredMqttBroker(uint16_t port, const String &username, const String &password)
        : PicoMQTT::Server(port), username_(username), password_(password) {}

protected:
    PicoMQTT::ConnectReturnCode auth(const char *clientId, const char *username, const char *password) override {
        (void)clientId;
        if (username_.isEmpty()) return PicoMQTT::CRC_ACCEPTED;
        if (!username || !password) return PicoMQTT::CRC_BAD_USERNAME_OR_PASSWORD;
        return (username_ == username && password_ == password)
                   ? PicoMQTT::CRC_ACCEPTED
                   : PicoMQTT::CRC_BAD_USERNAME_OR_PASSWORD;
    }

private:
    String username_;
    String password_;
};

class MqttService {
public:
    ~MqttService();
    void begin(ConfigStore *configStore, ShellyManager *shellyManager);
    void loop();
    bool running() const { return broker_ != nullptr; }

private:
    void handleCommand(const char *topic, const char *payload);
    void processPendingCommand();
    void publishStatuses();

    ConfigStore *configStore_ = nullptr;
    ShellyManager *shellyManager_ = nullptr;
    ConfiguredMqttBroker *broker_ = nullptr;

    bool commandPending_ = false;
    int pendingIndex_ = -1;
    String pendingAction_;
    uint32_t lastPublishMs_ = 0;
};
