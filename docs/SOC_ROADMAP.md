# Fahrzeug-SOC / ISO 15118 Roadmap
Version: 0.1.0
Date: 2026-09-21
Purpose: Trennt die spätere Fahrzeugkommunikation sauber vom AC-Wallbox-Treiber.

## Ausgangspunkt

Weder Pulsares EasyCharge BASIC noch Heidelberg Energy Control liefern über ihre hier genutzte Modbus-Schnittstelle den Batterie-SOC des Fahrzeugs.

CP/PP reichen für klassische AC-Ladezustände und Stromfreigabe, aber nicht für den gewünschten SOC-Datenaustausch.

## Geplante Komponente

~~~text
Heidelberg ----\
                +--> GA-WallBox Core --> Fronius-Regler
Pulsares ------/
                    ^
                    |
            VehicleCommunication
                    |
              ISO 15118
                    |
               PLC / SLAC
                    |
                   EV
~~~

## Prototyp-Hardware

Als bevorzugte Entwicklungsplattform wurde das DB2605-AC HAT betrachtet. Es soll nicht direkt in einen Pulsares- oder Heidelberg-Treiber eingebaut werden, sondern als separate VehicleCommunication-Komponente dienen.

## Ziel für spätere API

Mögliche additive Statusfelder:
- `vehicle_soc_percent`
- `vehicle_target_soc_percent`
- `vehicle_battery_capacity_kwh`
- `vehicle_energy_request_kwh`
- `vehicle_communication_state`

Fehlen diese Informationen, bleiben die Felder `null`.
