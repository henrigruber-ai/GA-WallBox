/*
File: src/core/WallboxTypes.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Vendor-neutral wallbox state and capability model.
*/
#pragma once

#include <Arduino.h>
#include <math.h>

struct WallboxCapabilities {
  bool powerMeasurement = false;
  bool energyMeasurement = false;
  bool currentMeasurement = false;
  bool voltageMeasurement = false;
  bool frequencyMeasurement = false;
  bool phaseMeasurement = false;
  bool switching = true;
  bool powerControl = true;
  bool currentLimitControl = true;
  bool vehicleState = true;
  bool temperatureMeasurement = false;

  float minCurrentA = 6.0f;
  float maxCurrentA = 16.0f;
};

struct WallboxState {
  bool online = false;
  bool vehicleConnected = false;
  bool charging = false;
  bool enabled = false;

  float requestedCurrentA = 0.0f;
  float effectiveCurrentLimitA = NAN;

  float currentL1A = NAN;
  float currentL2A = NAN;
  float currentL3A = NAN;

  float voltageL1V = NAN;
  float voltageL2V = NAN;
  float voltageL3V = NAN;

  float powerW = NAN;
  float energyWh = NAN;
  float apparentPowerVA = NAN;
  float apparentEnergyVAh = NAN;
  float frequencyHz = NAN;
  float temperatureC = NAN;

  int errorCode = 0;
  String state = "unknown";
  String error;
};
