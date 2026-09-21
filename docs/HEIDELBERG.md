# Heidelberg Wallbox Energy Control
Version: 0.1.0
Date: 2026-09-21
Purpose: Hardware- und Softwarevertrag des Heidelberg-Treibers.

## Default-Verbindung
- RS485 / Modbus RTU
- Slave-Adresse: 1
- 19200 baud
- 8E1
- ESP32 UART2: RX GPIO16, TX GPIO17
- DE/RE: GPIO4
- galvanisch getrennter RS485-Transceiver empfohlen

## Verwendete Register

Input-Register:
- 4: Registerlayout-Version
- 5: Charging State
- 6..8: Strom L1..L3 in 0,1 A
- 9: PCB-Temperatur in 0,1 °C
- 10..12: Spannung L1..L3
- 13: External Lock State
- 14: interne Gesamtleistung in VA
- 17..18: interne Energie seit Installation in VAh
- 100: Hardware-Maximalstrom
- 101: Hardware-Minimalstrom

Holding-Register:
- 257: Modbus-Master-Watchdog in ms
- 259: Remote Lock
- 261: maximaler Ladestrom, 0 oder 60..160 in 0,1-A-Schritten
- 262: Failsafe-Strom

## Failsafe

Beim Start setzt die Firmware:
1. Failsafe-Strom = 0 A
2. Modbus-Watchdog = 5000 ms
3. maximalen Ladestrom = 0 A
4. Remote Lock = locked

## Leistung und Energie

Die Herstellerdokumentation kennzeichnet die internen Messwerte ausdrücklich als nicht für genaue Abrechnung geeignet und beschreibt Register 14 als VA sowie 17/18 als VAh.

Zur Kompatibilität mit dem bereits vorbereiteten Fronius-Adapter werden diese Werte in V0.1 zusätzlich in `power_w` und `energy_wh` gespiegelt. Die semantisch korrekten Felder `apparent_power_va` und `apparent_energy_vah` werden parallel ausgegeben. Diese Kompatibilitätsfelder dürfen nicht als abrechnungsfähige Wirkleistung bzw. Wirkenergie interpretiert werden.

## Quellen
- Amperfied Knowledge Base: https://www.amperfied.de/support/wissensdatenbank/energy-control-plus/
- Modbus Register Table: https://www.amperfied.de/wp-content/uploads/2022/06/ModBus-Register-Tabelle.pdf
