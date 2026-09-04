#include "web_ui.h"

#include <ESPmDNS.h>
#include <WiFi.h>
#include "web_assets.h"

namespace {
String jsonEscape(const String &value) {
    String out;
    out.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); ++i) {
        const char c = value[i];
        if (c == '"' || c == '\\') {
            out += '\\';
            out += c;
        } else if (c == '\n') {
            out += "\\n";
        } else if (c != '\r') {
            out += c;
        }
    }
    return out;
}

String normalizeHostname(String hostname) {
    hostname.trim();
    hostname.toLowerCase();
    String result;
    result.reserve(hostname.length());
    for (size_t i = 0; i < hostname.length(); ++i) {
        char c = hostname[i];
        const bool valid = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9') || c == '-';
        if (valid) result += c;
    }
    while (result.startsWith("-")) result.remove(0, 1);
    while (result.endsWith("-")) result.remove(result.length() - 1);
    if (result.isEmpty()) result = "m5dial-shelly";
    if (result.length() > 32) result = result.substring(0, 32);
    return result;
}
}

WebUiService::WebUiService() : server_(80) {}

void WebUiService::begin(ConfigStore *configStore, ShellyManager *shellyManager) {
    configStore_ = configStore;
    shellyManager_ = shellyManager;
    const AppConfig &cfg = configStore_->get();

    WiFi.persistent(false);
    WiFi.mode(WIFI_AP_STA);
    if (!cfg.hostname.isEmpty()) WiFi.setHostname(cfg.hostname.c_str());

    if (!cfg.wifiSsid.isEmpty()) {
        WiFi.begin(cfg.wifiSsid.c_str(), cfg.wifiPassword.c_str());
        const uint32_t start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) {
            delay(100);
        }
    }

    if (WiFi.status() == WL_CONNECTED) startMdns();
    else startAccessPoint();

    setupRoutes();
    server_.begin();
}

void WebUiService::startAccessPoint() {
    apMode_ = true;
    const uint64_t mac = ESP.getEfuseMac();
    char suffix[7];
    snprintf(suffix, sizeof(suffix), "%06llX", static_cast<unsigned long long>(mac & 0xFFFFFF));
    apSsid_ = "M5Dial-Setup-" + String(suffix);

    String password = configStore_->get().setupApPassword;
    if (password.length() < 8) password = "m5dial-setup";
    WiFi.softAP(apSsid_.c_str(), password.c_str());
    dns_.start(53, "*", WiFi.softAPIP());
}

void WebUiService::startMdns() {
    const AppConfig &cfg = configStore_->get();
    if (!MDNS.begin(cfg.hostname.c_str())) return;

    MDNS.addService("http", "tcp", 80);
    MDNS.addServiceTxt("http", "tcp", "product", "M5Dial-Shelly-Control");
    if (cfg.mqttEnabled) {
        MDNS.addService("mqtt", "tcp", cfg.mqttPort);
        MDNS.addServiceTxt("mqtt", "tcp", "product", "M5Dial-PicoMQTT");
        MDNS.addServiceTxt("mqtt", "tcp", "auth", cfg.mqttUsername.isEmpty() ? "none" : "username-password");
    }
}

String WebUiService::apPassword() const {
    if (!configStore_) return "m5dial-setup";
    const String password = configStore_->get().setupApPassword;
    return password.length() >= 8 ? password : String("m5dial-setup");
}

String WebUiService::accessAddress() const {
    if (apMode_) return "http://" + WiFi.softAPIP().toString();
    return "http://" + configStore_->get().hostname + ".local";
}

void WebUiService::setupRoutes() {
    server_.on("/", HTTP_GET, [this] {
        server_.send_P(200, "text/html; charset=utf-8", WEB_INDEX_HTML);
    });

    server_.on("/api/status", HTTP_GET, [this] { sendJson(200, statusJson()); });
    server_.on("/api/devices", HTTP_GET, [this] { sendJson(200, devicesJson()); });
    server_.on("/api/wifi/scan", HTTP_GET, [this] { sendJson(200, wifiScanJson()); });

    server_.on("/api/config", HTTP_POST, [this] {
        AppConfig &cfg = configStore_->edit();

        if (server_.hasArg("ssid")) cfg.wifiSsid = server_.arg("ssid");
        if (server_.hasArg("password") && !server_.arg("password").isEmpty()) {
            cfg.wifiPassword = server_.arg("password");
        }
        if (server_.hasArg("hostname")) cfg.hostname = normalizeHostname(server_.arg("hostname"));

        cfg.mqttEnabled = server_.arg("mqttEnabled") == "1";
        if (server_.hasArg("mqttPort")) {
            const long value = server_.arg("mqttPort").toInt();
            if (value < 1 || value > 65535) {
                sendJson(400, "{\"error\":\"invalid mqtt port\"}");
                return;
            }
            cfg.mqttPort = static_cast<uint16_t>(value);
        }
        if (server_.hasArg("mqttUsername")) cfg.mqttUsername = server_.arg("mqttUsername");
        if (server_.hasArg("mqttPassword") && !server_.arg("mqttPassword").isEmpty()) {
            cfg.mqttPassword = server_.arg("mqttPassword");
        }

        if (server_.hasArg("refreshMs")) {
            long value = server_.arg("refreshMs").toInt();
            if (value < 1000) value = 1000;
            if (value > 60000) value = 60000;
            cfg.refreshIntervalMs = static_cast<uint32_t>(value);
        }

        if (server_.hasArg("setupApPassword") && !server_.arg("setupApPassword").isEmpty()) {
            const String password = server_.arg("setupApPassword");
            if (password.length() < 8 || password.length() > 63) {
                sendJson(400, "{\"error\":\"setup AP password must be 8-63 characters\"}");
                return;
            }
            cfg.setupApPassword = password;
        }

        configStore_->save();
        sendJson(200, "{\"ok\":true,\"restart\":true}");
        scheduleRestart();
    });

    server_.on("/api/discover", HTTP_POST, [this] {
        if (WiFi.status() != WL_CONNECTED) {
            sendJson(409, "{\"error\":\"not connected to WiFi\"}");
            return;
        }
        const size_t found = shellyManager_->discoverMdns();
        shellyManager_->refreshAll();
        sendJson(200, "{\"ok\":true,\"added\":" + String(found) + "}");
    });

    server_.on("/api/device/add", HTTP_POST, [this] {
        SavedShellyDevice d;
        d.host = server_.arg("host");
        d.host.trim();
        d.name = server_.arg("name");
        d.name.trim();
        d.generation = static_cast<uint8_t>(server_.arg("generation").toInt());
        d.channel = static_cast<uint8_t>(constrain(server_.arg("channel").toInt(), 0, 15));
        d.enabled = true;

        if (d.host.isEmpty()) {
            sendJson(400, "{\"error\":\"host required\"}");
            return;
        }
        if (d.generation > 2) d.generation = 0;
        if (d.name.isEmpty()) d.name = "Shelly " + d.host;

        shellyManager_->addOrUpdate(d, true);
        const int index = shellyManager_->find(d.host, d.channel);
        if (index >= 0 && WiFi.status() == WL_CONNECTED) shellyManager_->refresh(index);
        sendJson(200, "{\"ok\":true}");
    });

    server_.on("/api/device/action", HTTP_POST, [this] {
        const int index = server_.arg("index").toInt();
        const String action = server_.arg("action");
        bool ok = false;
        if (action == "toggle") ok = shellyManager_->toggle(index);
        else if (action == "on") ok = shellyManager_->setState(index, true);
        else if (action == "off") ok = shellyManager_->setState(index, false);
        else {
            sendJson(400, "{\"error\":\"invalid action\"}");
            return;
        }
        sendJson(ok ? 200 : 502, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server_.on("/api/device/rename", HTTP_POST, [this] {
        const bool ok = shellyManager_->rename(server_.arg("index").toInt(), server_.arg("name"));
        sendJson(ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server_.on("/api/device/remove", HTTP_POST, [this] {
        const bool ok = shellyManager_->remove(server_.arg("index").toInt());
        sendJson(ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server_.on("/api/wifi/clear", HTTP_POST, [this] {
        configStore_->clearWifi();
        sendJson(200, "{\"ok\":true}");
        scheduleRestart();
    });

    server_.on("/api/restart", HTTP_POST, [this] {
        sendJson(200, "{\"ok\":true}");
        scheduleRestart();
    });

    server_.onNotFound([this] {
        if (apMode_) {
            server_.sendHeader("Location", "http://" + WiFi.softAPIP().toString(), true);
            server_.send(302, "text/plain", "");
        } else {
            server_.send(404, "text/plain", "Not found");
        }
    });
}

String WebUiService::statusJson() const {
    const AppConfig &cfg = configStore_->get();
    const String ip = apMode_ ? WiFi.softAPIP().toString() : WiFi.localIP().toString();

    String out = "{";
    out += "\"hostname\":\"" + jsonEscape(cfg.hostname) + "\",";
    out += "\"ssid\":\"" + jsonEscape(cfg.wifiSsid) + "\",";
    out += "\"mqttEnabled\":" + String(cfg.mqttEnabled ? "true" : "false") + ",";
    out += "\"mqttPort\":" + String(cfg.mqttPort) + ",";
    out += "\"mqttUsername\":\"" + jsonEscape(cfg.mqttUsername) + "\",";
    out += "\"mqttPasswordSet\":" + String(cfg.mqttPassword.isEmpty() ? "false" : "true") + ",";
    out += "\"refreshMs\":" + String(cfg.refreshIntervalMs) + ",";
    out += "\"apMode\":" + String(apMode_ ? "true" : "false") + ",";
    out += "\"address\":\"" + jsonEscape(accessAddress()) + "\",";
    out += "\"ip\":\"" + ip + "\",";
    out += "\"rssi\":" + String(WiFi.status() == WL_CONNECTED ? WiFi.RSSI() : 0) + ",";
    out += "\"uptime\":" + String(millis()) + ",";
    out += "\"freeHeap\":" + String(ESP.getFreeHeap()) + "}";
    return out;
}

String WebUiService::devicesJson() const {
    String out = "[";
    for (size_t i = 0; i < shellyManager_->count(); ++i) {
        if (i) out += ',';
        const ShellyDevice *d = shellyManager_->get(i);
        out += "{\"host\":\"" + jsonEscape(d->host) + "\",";
        out += "\"name\":\"" + jsonEscape(d->name) + "\",";
        out += "\"model\":\"" + jsonEscape(d->model) + "\",";
        out += "\"generation\":" + String(d->generation) + ",";
        out += "\"channel\":" + String(d->channel) + ",";
        out += "\"online\":" + String(d->online ? "true" : "false") + ",";
        out += "\"on\":" + String(d->on ? "true" : "false") + ",";
        out += "\"powerW\":" + (isnan(d->powerW) ? String("null") : String(d->powerW, 2)) + "}";
    }
    out += ']';
    return out;
}

String WebUiService::wifiScanJson() {
    const int count = WiFi.scanNetworks(false, true);
    String out = "[";
    for (int i = 0; i < count; ++i) {
        if (i) out += ',';
        out += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    out += ']';
    WiFi.scanDelete();
    return out;
}

void WebUiService::sendJson(int code, const String &json) {
    server_.send(code, "application/json; charset=utf-8", json);
}

void WebUiService::scheduleRestart(uint32_t delayMs) {
    restartRequested_ = true;
    restartAtMs_ = millis() + delayMs;
}

void WebUiService::loop() {
    server_.handleClient();
    if (apMode_) dns_.processNextRequest();
    if (restartRequested_ && static_cast<int32_t>(millis() - restartAtMs_) >= 0) {
        ESP.restart();
    }
}
