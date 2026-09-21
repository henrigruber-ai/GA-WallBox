# PULSARES EV EasyCharge BASIC
Version: 0.1.0
Date: 2026-09-21
Purpose: Hardware- und Softwarevertrag des Pulsares-Treibers.

## Default-Verbindung
- RS485 / Modbus RTU
- Slave-Adresse: 2
- 9600 baud
- 8E1
- ESP32 UART2: RX GPIO16, TX GPIO17
- DE/RE: GPIO4
- galvanisch getrennter RS485-Transceiver empfohlen

## Verwendete Register

| Funktion | Dokumentregister | Startadresse |
|---|---:|---:|
| Fehler | 40025 | 0x0019 |
| Ladestecker | 40027 | 0x001B |
| Ladevorgang | 40031 | 0x001F |
| Temperatur | 40035 | 0x0023 |
| gültiger Ladestrom | 40045 | 0x002D |
| Modbus-Ladestrom | 40093 | 0x005D |
| Backup-Ladestrom | 40095 | 0x005F |
| Backup-Modus/Watchdog | 40097 | 0x0061 |
| Hardware-Stromgrenze | 40099 | 0x0063 |

## Failsafe

Beim Start setzt die Firmware:
1. Backup-Ladestrom = 0 mA
2. Backup-Modus = 5-s-Modbus-Watchdog
3. Modbus-Ladestrom = 0 mA

Damit stoppt die Ladefreigabe auch dann, wenn der ESP32 selbst ausfällt.

## Messgrenze

Der Treiber kennt den vom EVSE freigegebenen Strom, aber keinen tatsächlichen L1/L2/L3-Strom, keine Phasenspannungen und keine Energie. Solche Werte bleiben in der gemeinsamen API `null`.

## Quellen
- PULSARES EV EasyCharge BASIC Produktseite: https://www.pulsares.shop/easycharge-basic.html
- Aktuelle Modbus-Map: https://pulsares.shop/medien/dokumente/ev-easycharge-basic-modbus-map.pdf
