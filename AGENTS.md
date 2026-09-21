# AGENTS.md
Version: 1.0.0
Date: 2026-09-21
Purpose: Verbindliche Arbeitsregeln für Menschen und KI-Agenten im Repository GA-WallBox.

## Geltungsbereich
Diese Regeln gelten für das gesamte Repository. Bereichsspezifische AGENTS.md dürfen Regeln verschärfen, aber nicht abschwächen.

## Branching und Pull Requests
- Niemals direkt auf `main` entwickeln.
- Jede Änderung erfolgt auf einem eigenen Branch, z. B. `feat/...`, `fix/...`, `docs/...` oder `chore/...`.
- Änderungen gelangen ausschließlich per Pull Request nach `main`.
- Agenten dürfen Pull Requests erstellen, aber nicht selbst mergen.
- Kein Force-Push auf `main`; `main` darf nicht gelöscht werden.

## Qualität
- Build, Tests und Repository-Checks müssen vor einem Merge erfolgreich sein.
- Keine Datei soll mehr als 800 Zeilen enthalten. Bei Überschreitung ist zu modularisieren.
- Neue oder geänderte Quellcodedateien tragen einen Versionsheader mit Datei, Version, Datum und Zweck.
- Komplexe oder sicherheitsrelevante Logik muss nachvollziehbar kommentiert sein.
- Keine Secrets, Tokens, WLAN-Zugangsdaten oder produktiven Schlüssel in Git einchecken.
- Fehlerfälle müssen explizit behandelt werden; stilles Ignorieren ist nicht zulässig.

## Versionsführung
- Die kanonische Projektversion steht in `VERSION`.
- Firmware-Antworten und Build-Metadaten müssen dieselbe Version verwenden.
- Jede fachliche Änderung wird in `CHANGELOG.md` dokumentiert.
- SemVer verwenden: MAJOR.MINOR.PATCH.

## Architektur
- Der Fronius-Regler/Raspberry Pi enthält die PV- und Energiemanagement-Logik.
- Der ESP32 ist Hardware-Gateway, normalisiert Zustände, setzt lokale Sicherheitsgrenzen durch und stellt eine stabile REST-API bereit.
- Wallbox-spezifische Modbus-Logik bleibt ausschließlich in einem Treiber.
- Gemeinsamer Code darf keine Pulsares- oder Heidelberg-Register direkt kennen.
- Nicht unterstützte Messwerte werden als `null`/nicht verfügbar gemeldet und nicht erfunden.
- Fahrzeug-SOC ist kein Bestandteil von CP/PP oder der normalen AC-Modbus-Kommunikation. ISO-15118/PLC wird als unabhängige spätere Komponente integriert.

## Sicherheit
- REST/API-Kommandos dürfen hardwareseitige Schutzfunktionen niemals umgehen.
- Ein Kommunikationsverlust muss in einen sicheren Zustand führen.
- Pulsares: Hardware-Watchdog 5 s, Backup-Ladestrom 0 A.
- Der übergeordnete Controller muss seinen Sollwert regelmäßig erneuern; Standard-Control-Lease: 15 s.
- Nach einem Modbus-Kommunikationsfehler gilt standardmäßig eine 7-s-Cooldown-Phase.
- Schutzorgane, CP/PP, RCD/Fehlerstromüberwachung und Schützlogik bleiben Aufgabe der dafür vorgesehenen Hardware.

## Definition of Done
Eine Änderung ist fertig, wenn:
1. Architekturgrenzen eingehalten sind.
2. Build und Tests grün sind.
3. Quality-Gate grün ist.
4. Dokumentation und CHANGELOG aktualisiert sind.
5. Keine Secrets oder Debug-Reste enthalten sind.
6. PR-Beschreibung Testweg, Risiken und betroffene Hardware nennt.
