#include "web_ui.h"

#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <WiFi.h>

namespace {
const char INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html><html lang="de"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1">
<title>M5 Dial Shelly Control</title><style>
:root{color-scheme:dark;--bg:#0b0d10;--card:#151920;--line:#262d38;--text:#f4f7fb;--muted:#9aa7b6;--accent:#56d364;--danger:#ff6b6b}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:15px system-ui,-apple-system,Segoe UI,sans-serif}.wrap{max-width:900px;margin:auto;padding:18px}.top{display:flex;justify-content:space-between;gap:12px;align-items:center;margin-bottom:14px}.badge{padding:6px 10px;border:1px solid var(--line);border-radius:999px;color:var(--muted)}h1{font-size:22px;margin:0}h2{font-size:16px;margin:0 0 14px}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:14px}.card{background:var(--card);border:1px solid var(--line);border-radius:18px;padding:16px;box-shadow:0 8px 28px #0004}.row{display:grid;grid-template-columns:1fr 1fr;gap:10px}.field{margin:9px 0}label{display:block;color:var(--muted);font-size:12px;margin-bottom:5px}input,select,button{width:100%;border-radius:11px;border:1px solid var(--line);background:#0f1318;color:var(--text);padding:10px 11px;font:inherit}button{cursor:pointer;font-weight:650}button.primary{background:var(--accent);color:#071108;border-color:transparent}button.danger{color:#ffd8d8;border-color:#5c2b2b}.inline{display:flex;gap:8px;align-items:center}.inline>*{flex:1}.device{padding:12px 0;border-top:1px solid var(--line)}.device:first-child{border-top:0}.deviceHead{display:flex;justify-content:space-between;gap:8px}.muted{color:var(--muted);font-size:12px}.on{color:var(--accent)}.off{color:var(--muted)}.actions{display:flex;gap:7px;margin-top:8px}.actions button{padding:8px}.statusDot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#666;margin-right:6px}.online{background:var(--accent)}.warn{font-size:12px;color:#d7b46a;margin-top:10px}.switch{display:flex;align-items:center;gap:8px}.switch input{width:auto}.footer{color:var(--muted);font-size:12px;margin:18px 2px}</style></head><body>
<div class="wrap"><div class="top"><div><h1>M5 Dial · Shelly Control</h1><div class="muted" id="addr">Lade Status…</div></div><span class="badge" id="mode">…</span></div>
<div class="grid">
<section class="card"><h2>WLAN</h2><div class="field"><label>SSID</label><div class="inline"><input id="ssid" placeholder="WLAN-Name"><button onclick="scanWifi()">Scannen</button></div></div><div id="nets" class="muted"></div><div class="field"><label>Passwort</label><input id="wifiPass" type="password" placeholder="Leer lassen = bestehendes Passwort behalten"></div><div class="field"><label>Hostname</label><input id="hostname"></div><button class="primary" onclick="saveConfig()">Speichern & neu starten</button></section>
<section class="card"><h2>MQTT Broker</h2><div class="field switch"><input id="mqttEnabled" type="checkbox"><label for="mqttEnabled" style="margin:0">Broker aktivieren</label></div><div class="row"><div class="field"><label>Port</label><input id="mqttPort" type="number" min="1" max="65535"></div><div class="field"><label>Statusintervall (ms)</label><input id="refreshMs" type="number" min="1000" max="60000"></div></div><div class="warn">PicoMQTT Broker: MQTT 3.1.1 / QoS 0. Keine retained Messages oder Broker-LWT.</div></section>
<section class="card" style="grid-column:1/-1"><div class="top" style="margin:0 0 8px"><h2 style="margin:0">Shelly Geräte</h2><button style="width:auto" onclick="discover()">Automatisch suchen</button></div><div id="devices">Keine Geräte geladen.</div><div class="device"><strong>Gerät manuell hinzufügen</strong><div class="row"><div class="field"><label>IP / Host</label><input id="devHost" placeholder="192.168.1.50"></div><div class="field"><label>Name</label><input id="devName" placeholder="Wohnzimmer"></div></div><div class="row"><div class="field"><label>Generation</label><select id="devGen"><option value="0">Automatisch</option><option value="1">Gen1</option><option value="2">Gen2/3</option></select></div><div class="field"><label>Kanal</label><input id="devChannel" type="number" min="0" max="15" value="0"></div></div><button onclick="addDevice()">Hinzufügen</button></div></section>
<section class="card"><h2>System</h2><div id="system" class="muted"></div><div class="actions"><button onclick="restart()">Neustart</button><button class="danger" onclick="clearWifi()">WLAN zurücksetzen</button></div></section>
</div><div class="footer">Lokale Konfiguration · keine Cloud erforderlich</div></div>
<script>
const $=id=>document.getElementById(id); async function api(url,opt){const r=await fetch(url,opt);if(!r.ok)throw new Error(await r.text());return r.headers.get('content-type')?.includes('json')?r.json():r.text()}
async function load(){try{const s=await api('/api/status');$('hostname').value=s.hostname;$('ssid').value=s.ssid||'';$('mqttEnabled').checked=s.mqttEnabled;$('mqttPort').value=s.mqttPort;$('refreshMs').value=s.refreshMs;$('mode').textContent=s.apMode?'Setup-AP':'WLAN';$('addr').textContent=s.address;$('system').innerHTML=`IP: ${s.ip}<br>RSSI: ${s.rssi} dBm<br>Uptime: ${Math.floor(s.uptime/1000)} s<br>Heap frei: ${s.freeHeap} B`;renderDevices(await api('/api/devices'))}catch(e){console.error(e)}}
function renderDevices(ds){$('devices').innerHTML=ds.length?ds.map((d,i)=>`<div class="device"><div class="deviceHead"><div><span class="statusDot ${d.online?'online':''}"></span><strong>${esc(d.name)}</strong><div class="muted">${esc(d.host)} · Gen ${d.generation||'?'} · Kanal ${d.channel}${d.model?' · '+esc(d.model):''}</div></div><div class="${d.on?'on':'off'}">${d.online?(d.on?'EIN':'AUS'):'OFFLINE'}${Number.isFinite(d.powerW)?` · ${d.powerW.toFixed(1)} W`:''}</div></div><div class="actions"><button onclick="toggleDevice(${i})">Umschalten</button><button onclick="renameDevice(${i})">Umbenennen</button><button class="danger" onclick="removeDevice(${i})">Entfernen</button></div></div>`).join(''):'<div class="muted">Noch keine Shelly Geräte. Starte die automatische Suche oder füge eine IP hinzu.</div>'}
function esc(v){return String(v??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#039;'}[c]))}
async function post(url,data){return api(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)})}
async function scanWifi(){const n=await api('/api/wifi/scan');$('nets').innerHTML=n.map(x=>`<button style="width:auto;margin:3px" onclick="$('ssid').value='${esc(x.ssid)}'">${esc(x.ssid)} (${x.rssi})</button>`).join('')||'Keine Netze gefunden'}
async function saveConfig(){await post('/api/config',{ssid:$('ssid').value,password:$('wifiPass').value,hostname:$('hostname').value,mqttEnabled:$('mqttEnabled').checked?'1':'0',mqttPort:$('mqttPort').value,refreshMs:$('refreshMs').value});alert('Gespeichert. M5 Dial startet neu.')}async function discover(){await post('/api/discover',{});await load()}async function addDevice(){await post('/api/device/add',{host:$('devHost').value,name:$('devName').value,generation:$('devGen').value,channel:$('devChannel').value});await load()}async function toggleDevice(i){await post('/api/device/action',{index:i,action:'toggle'});await load()}async function renameDevice(i){const n=prompt('Neuer Name');if(n){await post('/api/device/rename',{index:i,name:n});await load()}}async function removeDevice(i){if(confirm('Gerät entfernen?')){await post('/api/device/remove',{index:i});await load()}}async function restart(){await post('/api/restart',{})}async function clearWifi(){if(confirm('Gespeicherte WLAN-Daten löschen?')){await post('/api/wifi/clear',{});alert('WLAN gelöscht. Neustart…')}}load();setInterval(load,5000)
</script></body></html>
)HTML";

String jsonEscape(const String &value) {
    String out;
    out.reserve(value.length() + 8);
    for (size_t i = 0; i < value.length(); ++i) {
        char c = value[i];
        if (c == '"' || c == '\\') { out += '\\'; out += c; }
        else if (c == '\n') out += "\\n";
        else if (c != '\r') out += c;
    }
    return out;
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
        uint32_t start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 12000) delay(100);
    }

    if (WiFi.status() == WL_CONNECTED) startMdns();
    else startAccessPoint();

    setupRoutes();
    server_.begin();
}

void WebUiService::startAccessPoint() {
    apMode_ = true;
    uint64_t mac = ESP.getEfuseMac();
    char suffix[7];
    snprintf(suffix, sizeof(suffix), "%06llX", static_cast<unsigned long long>(mac & 0xFFFFFF));
    apSsid_ = "M5Dial-Setup-" + String(suffix);
    String password = configStore_->get().setupApPassword;
    if (password.length() < 8) password = "m5dial-setup";
    WiFi.softAP(apSsid_.c_str(), password.c_str());
    dns_.start(53, "*", WiFi.softAPIP());
}

void WebUiService::startMdns() {
    if (MDNS.begin(configStore_->get().hostname.c_str())) {
        MDNS.addService("http", "tcp", 80);
        MDNS.addServiceTxt("http", "tcp", "product", "M5Dial-Shelly-Control");
    }
}

String WebUiService::accessAddress() const {
    if (apMode_) return "http://" + WiFi.softAPIP().toString();
    return "http://" + configStore_->get().hostname + ".local";
}

void WebUiService::setupRoutes() {
    server_.on("/", HTTP_GET, [this] { server_.send_P(200, "text/html; charset=utf-8", INDEX_HTML); });
    server_.on("/api/status", HTTP_GET, [this] { sendJson(200, statusJson()); });
    server_.on("/api/devices", HTTP_GET, [this] { sendJson(200, devicesJson()); });
    server_.on("/api/wifi/scan", HTTP_GET, [this] { sendJson(200, wifiScanJson()); });

    server_.on("/api/config", HTTP_POST, [this] {
        AppConfig &cfg = configStore_->edit();
        if (server_.hasArg("ssid")) cfg.wifiSsid = server_.arg("ssid");
        if (server_.hasArg("password") && !server_.arg("password").isEmpty()) cfg.wifiPassword = server_.arg("password");
        if (server_.hasArg("hostname") && !server_.arg("hostname").isEmpty()) cfg.hostname = server_.arg("hostname");
        cfg.mqttEnabled = server_.arg("mqttEnabled") == "1";
        if (server_.hasArg("mqttPort")) cfg.mqttPort = constrain(server_.arg("mqttPort").toInt(), 1, 65535);
        if (server_.hasArg("refreshMs")) cfg.refreshIntervalMs = constrain(server_.arg("refreshMs").toInt(), 1000, 60000);
        configStore_->save();
        sendJson(200, "{\"ok\":true,\"restart\":true}");
        scheduleRestart();
    });

    server_.on("/api/discover", HTTP_POST, [this] {
        size_t found = shellyManager_->discoverMdns();
        shellyManager_->refreshAll();
        sendJson(200, "{\"ok\":true,\"added\":" + String(found) + "}");
    });

    server_.on("/api/device/add", HTTP_POST, [this] {
        SavedShellyDevice d;
        d.host = server_.arg("host"); d.name = server_.arg("name");
        d.generation = static_cast<uint8_t>(server_.arg("generation").toInt());
        d.channel = static_cast<uint8_t>(constrain(server_.arg("channel").toInt(), 0, 15));
        d.enabled = true;
        if (d.host.isEmpty()) { sendJson(400, "{\"error\":\"host required\"}"); return; }
        shellyManager_->addOrUpdate(d, true);
        int index = shellyManager_->find(d.host, d.channel);
        if (index >= 0) shellyManager_->refresh(index);
        sendJson(200, "{\"ok\":true}");
    });

    server_.on("/api/device/action", HTTP_POST, [this] {
        int index = server_.arg("index").toInt(); String action = server_.arg("action");
        bool ok = action == "toggle" ? shellyManager_->toggle(index) : (action == "on" ? shellyManager_->setState(index, true) : shellyManager_->setState(index, false));
        sendJson(ok ? 200 : 502, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server_.on("/api/device/rename", HTTP_POST, [this] {
        bool ok = shellyManager_->rename(server_.arg("index").toInt(), server_.arg("name"));
        sendJson(ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server_.on("/api/device/remove", HTTP_POST, [this] {
        bool ok = shellyManager_->remove(server_.arg("index").toInt());
        sendJson(ok ? 200 : 400, ok ? "{\"ok\":true}" : "{\"ok\":false}");
    });

    server_.on("/api/wifi/clear", HTTP_POST, [this] {
        configStore_->clearWifi(); sendJson(200, "{\"ok\":true}"); scheduleRestart();
    });
    server_.on("/api/restart", HTTP_POST, [this] { sendJson(200, "{\"ok\":true}"); scheduleRestart(); });

    server_.onNotFound([this] {
        if (apMode_) {
            server_.sendHeader("Location", "http://" + WiFi.softAPIP().toString(), true);
            server_.send(302, "text/plain", "");
        } else server_.send(404, "text/plain", "Not found");
    });
}

String WebUiService::statusJson() const {
    const AppConfig &cfg = configStore_->get();
    String ip = apMode_ ? WiFi.softAPIP().toString() : WiFi.localIP().toString();
    String out = "{";
    out += "\"hostname\":\"" + jsonEscape(cfg.hostname) + "\",";
    out += "\"ssid\":\"" + jsonEscape(cfg.wifiSsid) + "\",";
    out += "\"mqttEnabled\":" + String(cfg.mqttEnabled ? "true" : "false") + ",";
    out += "\"mqttPort\":" + String(cfg.mqttPort) + ",";
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
        out += "{\"host\":\"" + jsonEscape(d->host) + "\",\"name\":\"" + jsonEscape(d->name) + "\",\"model\":\"" + jsonEscape(d->model) + "\",";
        out += "\"generation\":" + String(d->generation) + ",\"channel\":" + String(d->channel) + ",\"online\":" + String(d->online ? "true" : "false") + ",\"on\":" + String(d->on ? "true" : "false") + ',';
        out += "\"powerW\":" + (isnan(d->powerW) ? String("null") : String(d->powerW, 2)) + "}";
    }
    out += ']'; return out;
}

String WebUiService::wifiScanJson() {
    int n = WiFi.scanNetworks(false, true);
    String out = "[";
    for (int i = 0; i < n; ++i) {
        if (i) out += ',';
        out += "{\"ssid\":\"" + jsonEscape(WiFi.SSID(i)) + "\",\"rssi\":" + String(WiFi.RSSI(i)) + "}";
    }
    out += ']'; WiFi.scanDelete(); return out;
}

void WebUiService::sendJson(int code, const String &json) { server_.send(code, "application/json; charset=utf-8", json); }
void WebUiService::scheduleRestart(uint32_t delayMs) { restartRequested_ = true; restartAtMs_ = millis() + delayMs; }

void WebUiService::loop() {
    server_.handleClient();
    if (apMode_) dns_.processNextRequest();
    if (restartRequested_ && static_cast<int32_t>(millis() - restartAtMs_) >= 0) ESP.restart();
}
