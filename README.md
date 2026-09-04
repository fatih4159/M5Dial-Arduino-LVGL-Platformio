# M5 Dial Shelly Control

Lokale Smart-Home-Firmware für den **M5Stack M5 Dial** auf Basis von **LVGL 9** mit direkter Shelly-Steuerung, integrierter WebUI und lokalem MQTT-Broker.

## Funktionen

- LVGL-Oberfläche für mehrere Shelly-Geräte
- Drehencoder: Gerät auswählen
- Encoder drücken oder Touch: ausgewähltes Gerät umschalten
- Shelly Gen2/Gen3 Discovery via mDNS (`_shelly._tcp`)
- Shelly Gen1 Discovery via `_http._tcp`, sofern der Host als Shelly erkennbar ist
- Gen1-Steuerung über `/relay/{channel}`
- Gen2/Gen3-Steuerung über Shelly RPC (`Switch.GetStatus`, `Switch.Set`)
- Leistungsanzeige, wenn der Shelly `apower` bereitstellt
- persistente Konfiguration in ESP32 Preferences/NVS
- responsive lokale WebUI ohne Cloud-Abhängigkeit
- WLAN-Scan und WLAN-Konfiguration
- Shelly-Geräte automatisch suchen oder manuell per IP/Hostname hinzufügen
- Geräte umbenennen und entfernen
- MQTT-Broker auf dem M5 Dial via PicoMQTT
- optionale MQTT-Benutzername/Passwort-Authentifizierung
- Captive Setup-Access-Point bei fehlender oder fehlerhafter WLAN-Konfiguration
- Setup-AP-Passwort über die WebUI änderbar

## Erstinstallation / Setup

Wenn noch kein funktionierendes WLAN gespeichert ist, startet der M5 Dial automatisch:

- **SSID:** `M5Dial-Setup-XXXXXX`
- **Standardpasswort:** `m5dial-setup`
- **WebUI:** `http://192.168.4.1`

SSID und Setup-Adresse werden auf dem M5 Dial angezeigt. Das AP-Passwort wird nach einer Änderung aus Sicherheitsgründen nicht dauerhaft im Display eingeblendet.

Nach erfolgreicher WLAN-Konfiguration ist die WebUI normalerweise erreichbar unter:

- `http://m5dial-shelly.local`
- alternativ über die im Router angezeigte IP-Adresse

Über die WebUI können WLAN, Hostname, Polling-Intervall, Setup-AP-Passwort, MQTT-Broker und Shelly-Geräte verwaltet werden.

## MQTT

Standardmäßig läuft der Broker auf Port **1883**. Bleibt der MQTT-Benutzername leer, akzeptiert der Broker Clients ohne Anmeldung. Sobald ein Benutzername gesetzt ist, sind Benutzername und Passwort erforderlich.

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

Ohne Authentifizierung:

```bash
mosquitto_pub -h <M5-DIAL-IP> -p 1883 -t m5dial/shelly/0/set -m toggle
```

Mit Authentifizierung:

```bash
mosquitto_pub -h <M5-DIAL-IP> -p 1883 -u <USER> -P <PASSWORT> -t m5dial/shelly/0/set -m toggle
```

### PicoMQTT-Einschränkungen

Der integrierte PicoMQTT-Broker implementiert MQTT 3.1.1. Der Broker unterstützt QoS 0 und ignoriert retained Messages sowie Last-Will-Nachrichten. Für große oder kritische MQTT-Installationen ist ein dedizierter Broker wie Mosquitto sinnvoller.

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

Die PlatformIO-Plattform und LVGL-Version sind im Projekt gepinnt, um reproduzierbare Builds zu erhalten.

## Sicherheit

Die Firmware ist für ein vertrauenswürdiges lokales LAN bzw. IoT-VLAN gedacht. MQTT kann mit Benutzername und Passwort abgesichert werden. Die Konfigurations-WebUI besitzt aktuell keine eigene Anmeldung und sollte deshalb nicht aus dem Internet erreichbar sein.
