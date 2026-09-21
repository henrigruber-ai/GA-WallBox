# GA-WallBox
Version: 0.1.0
Date: 2026-09-21
Purpose: Universelles ESP32-Gateway der Gruber Automation für AC-Wallboxen.

GA-WallBox trennt die PV-/Energiemanagement-Logik des Fronius-Reglers von der konkreten Wallbox-Hardware. Dieselbe REST-Schnittstelle kann dadurch mit unterschiedlichen Wallbox-Treibern verwendet werden.

## Unterstützt in V0.1

- PULSARES EV EasyCharge BASIC
- Heidelberg Wallbox Energy Control
- ESP32 + RS485/Modbus RTU
- WLAN-Provisionierung
- REST-API für den Fronius-Regler
- lokaler Control-Lease/Failsafe
- optionaler Bearer-Token
- getrennte Firmwareprofile aus einer gemeinsamen Codebasis

## Architektur

~~~text
Fronius-Regler / Raspberry Pi
          |
        HTTP
          |
          v
     GA-WallBox
        ESP32
          |
   IWallbox-Treiber
      /       \
 Pulsares   Heidelberg
      \       /
     RS485 / Modbus
~~~

Der Raspberry Pi entscheidet über PV-Überschuss, Ladeleistung und Energiestrategie. Der ESP32 setzt Sollwerte um, liest Hardwarezustände und bleibt auch ohne Raspberry Pi lokal sicher.

## Build

Voraussetzung: PlatformIO.

Pulsares:

~~~bash
pio run -e esp32-pulsares
~~~

Heidelberg:

~~~bash
pio run -e esp32-heidelberg
~~~

Tests:

~~~bash
pio test -e native
python scripts/check_repo.py
~~~

## Standard-Pins

| Signal | ESP32 |
|---|---:|
| RS485 RX | GPIO16 |
| RS485 TX | GPIO17 |
| RS485 DE/RE | GPIO4 |

Ein galvanisch getrennter RS485-Transceiver wird empfohlen.

## WLAN

Beim ersten Start bzw. ohne gespeicherte Zugangsdaten öffnet WiFiManager das temporäre Netz:

~~~text
GA-WallBox-Setup
~~~

Nach erfolgreicher Konfiguration verbindet sich der ESP32 mit dem WLAN und startet die REST-API auf Port 80.

## API

Wesentliche Endpunkte:

~~~text
GET  /api/device
GET  /api/status
POST /api/control
GET  /health
~~~

Beispiel:

~~~json
POST /api/control
{
  "enabled": true,
  "power_limit_w": 4200
}
~~~

Alternativ kann direkt ein Stromsollwert gesetzt werden:

~~~json
{
  "enabled": true,
  "current_limit_a": 8.0
}
~~~

Die bestehende Vorbereitung im Repository `fronius-regler` kann damit gegen denselben Gerätevertrag arbeiten.

## Failsafe

- Steuerbefehle müssen spätestens innerhalb von 15 s erneuert werden.
- Ohne Erneuerung setzt der ESP32 die Wallbox auf einen sicheren 0-A-Zustand.
- Pulsares: 5-s-Modbus-Watchdog, Backupstrom 0 A.
- Heidelberg: 5-s-Modbus-Watchdog, Failsafe-Strom 0 A.
- Nach Modbusfehlern wird standardmäßig 7 s pausiert.

Software ersetzt keine hardwareseitigen Schutzfunktionen wie RCD/Fehlerstromüberwachung, Schützüberwachung oder normativ erforderliche CP/PP-Sicherheitsfunktionen.

## Fahrzeug-SOC

Der Batterie-SOC ist weder bei Pulsares noch bei Heidelberg Bestandteil der hier verwendeten Modbus-/CP-/PP-Kommunikation. Die spätere ISO-15118-/PLC-Komponente wird deshalb separat vom Wallbox-Treiber aufgebaut. Siehe `docs/SOC_ROADMAP.md`.

## Dokumentation

- `docs/ARCHITECTURE.md`
- `docs/FRONIUS_API.md`
- `docs/PULSARES.md`
- `docs/HEIDELBERG.md`
- `docs/SOC_ROADMAP.md`
- `AGENTS.md`
- `PROJECT_RULES.md`

## Entwicklungsregeln

Die verbindlichen Regeln stehen zentral in `AGENTS.md` und `PROJECT_RULES.md`. Änderungen erfolgen ausschließlich auf Feature-/Fix-Branches und per Pull Request. Build, Tests, Versionskonsistenz und die 800-Zeilen-Regel werden in GitHub Actions geprüft.
