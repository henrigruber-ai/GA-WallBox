# REST-Vertrag zum Fronius-Regler
Version: 0.1.0
Date: 2026-09-21
Purpose: Stabile HTTP-Schnittstelle zwischen Fronius-Regler und GA-WallBox.

## GET /api/device

Liefert Identität, Firmware, aktiven Treiber und Capabilities.

Beispiel:

~~~json
{
  "device": {
    "type": "GA-WallBox",
    "vendor": "Gruber Automation",
    "name": "GA-WallBox",
    "device_id": "GAWB-123456789ABC",
    "firmware": "0.1.0",
    "driver": "pulsares_easycharge_basic"
  },
  "capabilities": {
    "power_measurement": false,
    "energy_measurement": false,
    "current_measurement": false,
    "voltage_measurement": false,
    "phase_measurement": false,
    "switching": true,
    "power_control": true,
    "current_limit_control": true,
    "vehicle_state": true,
    "temperature_measurement": true,
    "min_current_a": 6,
    "max_current_a": 16
  }
}
~~~

## GET /api/status

Die bereits im Fronius-Regler vorbereiteten Feldnamen bleiben erhalten. Nicht vorhandene Messwerte sind `null`.

Zusätzliche Felder:
- `vehicle_connected`
- `charging`
- `requested_current_a`
- `effective_current_limit_a`
- `temperature_c`
- `apparent_power_va`
- `apparent_energy_vah`
- `control_age_ms`
- `control_lease_expired`

## POST /api/control

Bestehender Fronius-Vertrag:

~~~json
{
  "enabled": true,
  "power_limit_w": 4200
}
~~~

Erweiterung für direkte Stromvorgabe:

~~~json
{
  "enabled": true,
  "current_limit_a": 8.5
}
~~~

Mindestens eines der Felder muss gesetzt sein.

`power_limit_w` wird lokal mit 3 x 230 V in einen Stromsollwert umgesetzt. Werte unterhalb des minimal zulässigen Ladestroms führen zu 0 A statt zu einem unzulässigen Zwischenwert.

Der Fronius-Regler muss den Sollwert regelmäßig erneuern. Nach 15 s ohne gültigen Steuerbefehl stoppt die GA-WallBox.

## Authentifizierung

Wenn `GA_API_TOKEN` beim Build gesetzt ist:

~~~http
Authorization: Bearer <token>
~~~

Tokens werden nicht im Repository gespeichert und nicht in Fehlerantworten ausgegeben.

## Kompatibilitätsendpunkte

Für den früher vorbereiteten Heidelberg-Prototyp bleiben zusätzlich vorhanden:
- `POST /api/current` mit `{"ampere": 8}`
- `POST /api/enable`
- `POST /api/disable`
- `GET /health`
