# Changelog
Version: 0.1.0
Date: 2026-09-21

Alle wesentlichen Änderungen dieses Projekts werden hier dokumentiert.

## [0.1.0] - 2026-09-21
### Added
- Gemeinsame ESP32-Codebasis für PULSARES EV EasyCharge BASIC und Heidelberg Energy Control.
- Abstrakte `IWallbox`-Treibergrenze.
- RS485/Modbus-Transport mit Fehler-Cooldown.
- REST-Vertrag passend zur vorbereiteten GA-WallBox-Integration im Fronius-Regler.
- Lokaler 15-s-Control-Lease als Failsafe.
- Pulsares-Watchdog 5 s und Backupstrom 0 A.
- Heidelberg-Failsafe-Strom 0 A und Modbus-Watchdog.
- WLAN-Provisionierung über WiFiManager.
- Optionaler Bearer-Token über einen Build-Parameter; keine Secrets im Repository.
- Zwei PlatformIO-Firmwareprofile: Pulsares und Heidelberg.
- Dokumentation für Architektur, Modbus-Treiber, Fronius-API und SOC-Roadmap.
- GitHub Actions Quality Gate und 800-Zeilen-Regel.
