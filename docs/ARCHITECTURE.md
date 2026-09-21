# Architektur
Version: 0.1.0
Date: 2026-09-21
Purpose: Zielarchitektur der gemeinsamen GA-WallBox-Plattform.

## Zielbild

~~~text
Fronius-Regler / Raspberry Pi
          |
          | HTTP / REST
          v
+---------------------------+
|        GA-WallBox         |
|           ESP32           |
|                           |
|  REST API                 |
|      |                    |
|  WallboxController        |
|      |                    |
|    IWallbox               |
|     /     \               |
| Pulsares  Heidelberg      |
|     \     /               |
|     RS485 / Modbus RTU    |
+-------------|-------------+
              |
              v
       Wallbox-Hardware
~~~

Die PV-Strategie bleibt vollständig im Fronius-Regler. Der ESP32 setzt Sollwerte um, normalisiert Messwerte und stellt lokale Failsafes sicher.

## Zwei Firmwareprofile, eine Codebasis

V0.1 baut zwei Firmwarevarianten aus demselben Repository:

~~~bash
pio run -e esp32-pulsares
pio run -e esp32-heidelberg
~~~

Damit bleibt die gemeinsame API identisch, während serielle Defaults und Treiber getrennt bleiben. Eine spätere Laufzeit-Autoerkennung kann ergänzt werden, ohne den REST-Vertrag zu ändern.

## Sicherheitskette

### Raspberry Pi fällt aus
Der ESP32 verlangt eine regelmäßige Erneuerung des Sollwerts. Nach 15 s ohne Steuerbefehl wird lokal `safeStop()` ausgelöst.

### ESP32 fällt aus
Pulsares wird mit 5-s-Modbus-Watchdog und 0-A-Backup konfiguriert. Heidelberg wird mit 5-s-Modbus-Watchdog und 0-A-Failsafe konfiguriert.

### Modbusfehler
Nach einem fehlgeschlagenen Modbus-Telegramm pausiert der Transport standardmäßig 7 s. Das verhindert Request-Stürme und lässt den Hardware-Watchdog in den sicheren Zustand laufen.

## Erweiterung SOC

Die SOC-Kommunikation wird bewusst nicht in einen Hersteller-Treiber eingebaut:

~~~text
Auto
 |
PLC / SLAC
 |
ISO 15118
 |
VehicleCommunication
 |
GA-WallBox API
~~~

Dadurch kann dieselbe ISO-15118-Komponente später mit Pulsares oder Heidelberg kombiniert werden.
