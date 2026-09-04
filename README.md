# M5 Dial Shelly Control

Lokale Smart-Home-Firmware für den **M5Stack M5 Dial** auf Basis von **LVGL 9**, mit direkter Shelly-Steuerung, integrierter WebUI und lokalem MQTT-Broker.

## Funktionen

- M5-Dial-LVGL-Oberfläche für mehrere Shelly-Geräte
- Drehencoder: Gerät auswählen
- Encoder drücken / Touch: ausgewähltes Gerät umschalten
- Shelly Gen2/Gen3 Discovery via mDNS (`_shelly._tcp`)
- Shelly Gen1 Discovery via `_http._tcp`, sofern der Host als Shelly erkennbar ist
- Gen1-Steuerung über `/relay/{channel}`
- Gen2/Gen3-Steuerung über Shelly RPC (`Switch.GetStatus`, `Switch.Set`)
- Leistungsanzeige für Gen2/Gen3 Geräte, wenn `apower` verfügbar ist
- Persistente Konfiguration in ESP32 Preferences/NVS
- Responsive lokale WebUI
- WLAN-Scan und WLAN-Konfiguration
- Shelly-Geräte automatisch suchen oder manuell per IP/Hostname hinzufügen
- Geräte umbenennen und entfernen
- MQTT-Broker auf dem M5 Dial via PicoMQTT
- Captive Setup-Access-Point bei fehlender/fehlerhafter WLAN-Konfiguration

## Erstinstallation / Setup

Wenn noch kein funktionierendes WLAN gespeichert ist, startet der M5 Dial automatisch:

- **SSID:** `M5Dial-Setup-XXXXXX`
- **Passwort:** `m5dial-setup`
- **WebUI:** `http://192.168.4.1`

Die Zugangsdaten werden auch auf dem M5 Dial angezeigt.

Nach erfolgreicher WLAN-Konfiguration ist die WebUI normalerweise erreichbar unter:

- `http://m5dial-shelly.local`
- alternativ über die im Display bzw. Router angezeigte IP-Adresse

Hostname, MQTT-Port und Polling-Intervall können über die WebUI geändert werden.

## MQTT

Standardmäßig läuft der Broker auf Port **1883**. Er ist für kleine lokale Installationen gedacht.

### Status-Topics

```text
m5dial/system/status
m5dial/system/ip
m5dial/shelly/<index>/name
m5dial/shelly/<index>/host
m5dial/shelly/<index>/online
m5dial/shelly/<index>/state
m5dial/shelly/<index>/power_w
m5dial/shelly/<index>/command_result
```

### Shelly steuern

Publish auf:

```text
m5dial/shelly/<index>/set
```

Payload:

```text
on
off
toggle
```

Beispiel:

```bash
mosquitto_pub -h <M5-DIAL-IP> -p 1883 -t m5dial/shelly/0/set -m toggle
```

### PicoMQTT-Einschränkungen

Der integrierte PicoMQTT-Broker implementiert MQTT 3.1.1. Der Broker unterstützt QoS 0 und ignoriert retained Messages sowie Last-Will-Nachrichten. Für umfangreiche MQTT-Installationen sollte weiterhin ein dedizierter Broker wie Mosquitto verwendet werden; der M5 Dial kann dann primär als lokales Bediengerät dienen.

## Mehrkanal-Shellys

Automatische Discovery legt zunächst Kanal `0` an. Weitere Relais eines Mehrkanal-Geräts können in der WebUI mit derselben IP und einem anderen Kanal (z. B. 1, 2, 3) hinzugefügt werden.

## Build

Voraussetzung: PlatformIO.

```bash
pio run
```

Upload:

```bash
pio run -t upload
```

Serieller Monitor:

```bash
pio device monitor
```

## Sicherheit

Die Firmware ist für ein vertrauenswürdiges lokales LAN vorgesehen. Die Konfigurations-WebUI und der PicoMQTT-Broker stellen derzeit keine Benutzeranmeldung bereit. Den Broker nicht direkt aus dem Internet erreichbar machen und den M5 Dial in einem geeigneten IoT-/LAN-Segment betreiben.
