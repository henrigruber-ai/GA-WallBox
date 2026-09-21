# GA-WallBox – Projektregeln
Version: 1.0.0
Date: 2026-09-21
Purpose: Technische Leitplanken und Produktgrenzen.

## Produktziel
GA-WallBox ist ein universelles ESP32-Gateway zwischen dem Fronius-Regler und unterschiedlichen AC-Wallbox-Hardwareplattformen.

Unterstützte Treiber in V0.1:
- PULSARES EV EasyCharge BASIC
- Heidelberg Wallbox Energy Control

## Schichten
1. REST/API
2. WallboxController und Sicherheitslogik
3. IWallbox-Abstraktion
4. Hersteller-Treiber
5. RS485/Modbus RTU
6. Wallbox-Hardware

## Öffentliche API
Vertrag zum Fronius-Regler:
- `GET /api/device`
- `GET /api/status`
- `POST /api/control`
- `GET /health`

Kompatibilitätsendpunkte dürfen ergänzt werden, der oben genannte Vertrag bleibt stabil.

## Treiberprinzip
Jeder Treiber implementiert dieselbe Schnittstelle:
- `begin()`
- `poll()`
- `setEnabled()`
- `setCurrentLimitA()`
- `state()`
- `capabilities()`

Herstellerspezifische Besonderheiten werden über optionale/erweiterte Statusfelder dokumentiert.

## Regelungsgrenze
Die PV-Überschussregelung gehört in den Fronius-Regler. Der ESP32 berechnet keine PV-Strategie. Er darf `power_limit_w` lediglich in einen lokalen Stromsollwert umrechnen und sicher begrenzen.

## Hardwareprofile
PULSARES:
- Modbus RTU / RS485
- Default: Adresse 2, 9600 baud, 8E1
- Stromvorgabe 0..32 A in mA
- Watchdog: 5 s
- Backupstrom: 0 A

Heidelberg Energy Control:
- Modbus RTU / RS485
- Default: Adresse 1, 19200 baud, 8E1
- Stromvorgabe: 0 oder 6..16 A, 0.1-A-Schritte
- Messwerte: Phasenströme, Phasenspannungen, interne Leistung/Energie, Temperatur, Ladezustand
- Failsafe-Strom: 0 A

## SOC
SOC wird nicht aus der normalen AC-Wallbox-Modbus-Schnittstelle abgeleitet.
Spätere Erweiterung:
EV <-> PLC/SLAC <-> ISO 15118 <-> VehicleCommunication-Komponente <-> GA-WallBox API.

Bevorzugte Prototyp-Hardware für diese spätere Schicht: DB2605-AC HAT am Raspberry Pi. Diese Komponente bleibt architektonisch vom Wallbox-Treiber getrennt.
