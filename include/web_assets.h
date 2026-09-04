#pragma once

#include <Arduino.h>

static const char WEB_INDEX_HTML[] PROGMEM = R"HTML(
<!doctype html>
<html lang="de">
<head>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1,viewport-fit=cover">
<meta name="theme-color" content="#0b0d10">
<title>M5 Dial Shelly Control</title>
<style>
:root{color-scheme:dark;--bg:#0b0d10;--card:#151920;--card2:#10141a;--line:#29313d;--text:#f4f7fb;--muted:#93a0b0;--accent:#56d364;--danger:#ff7070;--warn:#e0b968}*{box-sizing:border-box}body{margin:0;background:var(--bg);color:var(--text);font:15px/1.45 system-ui,-apple-system,Segoe UI,sans-serif}.wrap{max-width:960px;margin:auto;padding:20px}.header{display:flex;align-items:center;justify-content:space-between;gap:16px;margin:4px 0 18px}.header h1{font-size:23px;line-height:1.15;margin:0}.muted{color:var(--muted);font-size:12px}.badge{white-space:nowrap;padding:7px 11px;border-radius:999px;border:1px solid var(--line);background:var(--card2);color:var(--muted)}.grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(290px,1fr));gap:14px}.card{background:var(--card);border:1px solid var(--line);border-radius:18px;padding:17px;box-shadow:0 10px 30px #0004}.wide{grid-column:1/-1}.card h2{font-size:16px;margin:0 0 13px}.row{display:grid;grid-template-columns:1fr 1fr;gap:10px}.field{margin:9px 0}label{display:block;font-size:12px;color:var(--muted);margin-bottom:5px}input,select,button{width:100%;border:1px solid var(--line);border-radius:11px;background:#0e1217;color:var(--text);padding:10px 11px;font:inherit;outline:none}input:focus,select:focus{border-color:#4c6a55;box-shadow:0 0 0 3px #56d36414}button{cursor:pointer;font-weight:650}button:hover{border-color:#526072}.primary{background:var(--accent);color:#071108;border-color:transparent}.danger{color:#ffd7d7;border-color:#5c3030}.inline{display:flex;gap:8px;align-items:center}.inline>*:first-child{flex:1}.inline button{width:auto}.switch{display:flex;align-items:center;gap:9px}.switch input{width:auto}.switch label{margin:0}.hint{color:var(--warn);font-size:12px;margin-top:9px}.device{padding:13px 0;border-top:1px solid var(--line)}.device:first-child{border-top:0}.deviceHead{display:flex;align-items:flex-start;justify-content:space-between;gap:12px}.state{text-align:right;white-space:nowrap}.on{color:var(--accent)}.off{color:var(--muted)}.dot{display:inline-block;width:8px;height:8px;border-radius:50%;background:#606874;margin-right:7px}.dot.online{background:var(--accent)}.actions{display:flex;gap:7px;margin-top:9px}.actions button{padding:8px}.networks{display:flex;flex-wrap:wrap;gap:6px;margin-top:7px}.networks button{width:auto;padding:6px 9px;font-size:12px}.statusGrid{display:grid;grid-template-columns:1fr 1fr;gap:8px 14px}.statusItem{padding:8px 0;border-bottom:1px solid var(--line)}.statusItem b{display:block;font-size:13px}.footer{margin:18px 2px;color:var(--muted);font-size:12px}@media(max-width:560px){.wrap{padding:14px}.row{grid-template-columns:1fr}.header{align-items:flex-start}.actions{flex-wrap:wrap}.actions button{min-width:120px;flex:1}.statusGrid{grid-template-columns:1fr 1fr}}
</style>
</head>
<body>
<div class="wrap">
  <header class="header">
    <div><h1>M5 Dial · Shelly Control</h1><div id="address" class="muted">Status wird geladen …</div></div>
    <span id="mode" class="badge">…</span>
  </header>

  <main class="grid">
    <section class="card">
      <h2>WLAN</h2>
      <div class="field"><label>SSID</label><div class="inline"><input id="ssid" autocomplete="off"><button type="button" onclick="scanWifi()">Scannen</button></div><div id="networks" class="networks"></div></div>
      <div class="field"><label>WLAN-Passwort</label><input id="wifiPass" type="password" autocomplete="new-password" placeholder="Leer = bestehendes Passwort behalten"></div>
      <div class="field"><label>Hostname</label><input id="hostname" autocomplete="off" placeholder="m5dial-shelly"></div>
    </section>

    <section class="card">
      <h2>MQTT Broker</h2>
      <div class="field switch"><input id="mqttEnabled" type="checkbox"><label for="mqttEnabled">Broker aktivieren</label></div>
      <div class="row"><div class="field"><label>Port</label><input id="mqttPort" type="number" min="1" max="65535"></div><div class="field"><label>Statusintervall (ms)</label><input id="refreshMs" type="number" min="1000" max="60000"></div></div>
      <div class="field"><label>Benutzername (optional)</label><input id="mqttUser" autocomplete="off" placeholder="Leer = keine MQTT-Anmeldung"></div>
      <div class="field"><label>Passwort</label><input id="mqttPass" type="password" autocomplete="new-password" placeholder="Leer = bestehendes Passwort behalten"></div>
      <div class="hint">MQTT 3.1.1 / QoS 0. PicoMQTT unterstützt keine retained Messages oder Broker-LWT.</div>
    </section>

    <section class="card">
      <h2>Setup Access Point</h2>
      <div class="field"><label>AP-Passwort</label><input id="apPass" type="password" minlength="8" autocomplete="new-password" placeholder="Leer = bestehendes Passwort behalten"></div>
      <div class="muted">Mindestens 8 Zeichen. Wird verwendet, falls das gespeicherte WLAN nicht erreichbar ist.</div>
    </section>

    <section class="card">
      <h2>System</h2>
      <div id="system" class="statusGrid"></div>
      <div class="actions"><button type="button" onclick="restart()">Neustart</button><button type="button" class="danger" onclick="clearWifi()">WLAN zurücksetzen</button></div>
    </section>

    <section class="card wide">
      <div class="header" style="margin:0 0 6px"><h2 style="margin:0">Shelly Geräte</h2><button type="button" style="width:auto" onclick="discover()">Automatisch suchen</button></div>
      <div id="devices"><div class="muted">Geräte werden geladen …</div></div>
      <div class="device">
        <strong>Gerät manuell hinzufügen</strong>
        <div class="row"><div class="field"><label>IP / Hostname</label><input id="devHost" placeholder="192.168.1.50"></div><div class="field"><label>Anzeigename</label><input id="devName" placeholder="Wohnzimmer"></div></div>
        <div class="row"><div class="field"><label>Generation</label><select id="devGen"><option value="0">Automatisch</option><option value="1">Gen1</option><option value="2">Gen2/Gen3</option></select></div><div class="field"><label>Kanal</label><input id="devChannel" type="number" min="0" max="15" value="0"></div></div>
        <button type="button" onclick="addDevice()">Hinzufügen</button>
      </div>
    </section>
  </main>

  <button class="primary" style="margin-top:14px" type="button" onclick="saveConfig()">Konfiguration speichern & neu starten</button>
  <div class="footer">Lokale Konfiguration · keine Cloud erforderlich</div>
</div>
<script>
const $=id=>document.getElementById(id);
const esc=v=>String(v??'').replace(/[&<>"']/g,c=>({'&':'&amp;','<':'&lt;','>':'&gt;','"':'&quot;',"'":'&#039;'}[c]));
async function api(url,opt){const r=await fetch(url,opt);if(!r.ok)throw new Error(await r.text());const ct=r.headers.get('content-type')||'';return ct.includes('json')?r.json():r.text()}
async function post(url,data){return api(url,{method:'POST',headers:{'Content-Type':'application/x-www-form-urlencoded'},body:new URLSearchParams(data)})}
function renderSystem(s){$('system').innerHTML=`<div class="statusItem"><span class="muted">IP</span><b>${esc(s.ip)}</b></div><div class="statusItem"><span class="muted">RSSI</span><b>${s.rssi} dBm</b></div><div class="statusItem"><span class="muted">Uptime</span><b>${Math.floor(s.uptime/1000)} s</b></div><div class="statusItem"><span class="muted">Heap frei</span><b>${s.freeHeap} B</b></div>`}
function renderDevices(ds){$('devices').innerHTML=ds.length?ds.map((d,i)=>`<div class="device"><div class="deviceHead"><div><span class="dot ${d.online?'online':''}"></span><strong>${esc(d.name)}</strong><div class="muted">${esc(d.host)} · Gen ${d.generation||'?'} · Kanal ${d.channel}${d.model?' · '+esc(d.model):''}</div></div><div class="state ${d.on?'on':'off'}">${d.online?(d.on?'EIN':'AUS'):'OFFLINE'}${Number.isFinite(d.powerW)?`<div class="muted">${d.powerW.toFixed(1)} W</div>`:''}</div></div><div class="actions"><button onclick="deviceAction(${i},'toggle')">Umschalten</button><button onclick="renameDevice(${i})">Umbenennen</button><button class="danger" onclick="removeDevice(${i})">Entfernen</button></div></div>`).join(''):'<div class="muted">Noch keine Shelly-Geräte. Automatische Suche starten oder ein Gerät manuell hinzufügen.</div>'}
async function load(initial=false){try{const s=await api('/api/status');$('mode').textContent=s.apMode?'Setup-AP':'WLAN';$('address').textContent=s.address;renderSystem(s);if(initial){$('hostname').value=s.hostname||'';$('ssid').value=s.ssid||'';$('mqttEnabled').checked=!!s.mqttEnabled;$('mqttPort').value=s.mqttPort;$('refreshMs').value=s.refreshMs;$('mqttUser').value=s.mqttUsername||''}renderDevices(await api('/api/devices'))}catch(e){console.error(e)}}
async function scanWifi(){try{const nets=await api('/api/wifi/scan');const box=$('networks');box.innerHTML='';nets.forEach(n=>{const b=document.createElement('button');b.type='button';b.textContent=`${n.ssid} (${n.rssi})`;b.onclick=()=>{$('ssid').value=n.ssid};box.appendChild(b)});if(!nets.length)box.textContent='Keine WLANs gefunden'}catch(e){alert('WLAN-Scan fehlgeschlagen')}}
async function saveConfig(){const ap=$('apPass').value;if(ap&&ap.length<8){alert('Das Setup-AP-Passwort muss mindestens 8 Zeichen lang sein.');return}try{await post('/api/config',{ssid:$('ssid').value,password:$('wifiPass').value,hostname:$('hostname').value,mqttEnabled:$('mqttEnabled').checked?'1':'0',mqttPort:$('mqttPort').value,mqttUsername:$('mqttUser').value,mqttPassword:$('mqttPass').value,refreshMs:$('refreshMs').value,setupApPassword:ap});alert('Gespeichert. Der M5 Dial startet neu.')}catch(e){alert('Speichern fehlgeschlagen: '+e.message)}}
async function discover(){try{const r=await post('/api/discover',{});await load(false);alert(`${r.added} neue Geräte hinzugefügt.`)}catch(e){alert('Suche fehlgeschlagen')}}
async function addDevice(){try{await post('/api/device/add',{host:$('devHost').value,name:$('devName').value,generation:$('devGen').value,channel:$('devChannel').value});$('devHost').value='';$('devName').value='';await load(false)}catch(e){alert('Gerät konnte nicht hinzugefügt werden: '+e.message)}}
async function deviceAction(i,action){try{await post('/api/device/action',{index:i,action});await load(false)}catch(e){alert('Shelly antwortet nicht.')}}
async function renameDevice(i){const n=prompt('Neuer Anzeigename:');if(!n)return;await post('/api/device/rename',{index:i,name:n});await load(false)}
async function removeDevice(i){if(!confirm('Gerät wirklich entfernen?'))return;await post('/api/device/remove',{index:i});await load(false)}
async function restart(){await post('/api/restart',{})}
async function clearWifi(){if(!confirm('Gespeicherte WLAN-Zugangsdaten löschen und Setup-AP starten?'))return;await post('/api/wifi/clear',{});alert('WLAN-Daten gelöscht. Neustart …')}
load(true);setInterval(()=>load(false),5000);
</script>
</body>
</html>
)HTML";
