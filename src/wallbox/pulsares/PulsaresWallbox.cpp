/*
File: src/wallbox/pulsares/PulsaresWallbox.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Implements PULSARES status polling, current control and hardware failsafe setup.
*/
#include "PulsaresWallbox.h"

#include "../../core/PowerLimit.h"

namespace {
constexpr uint16_t REG_ERROR = 0x0019;          // 40025
constexpr uint16_t REG_PLUG_STATE = 0x001B;     // 40027
constexpr uint16_t REG_CHARGING_STATE = 0x001F; // 40031
constexpr uint16_t REG_TEMPERATURE = 0x0023;    // 40035
constexpr uint16_t REG_VALID_CURRENT = 0x002D;  // 40045
constexpr uint16_t REG_MODBUS_CURRENT = 0x005D; // 40093
constexpr uint16_t REG_BACKUP_CURRENT = 0x005F; // 40095
constexpr uint16_t REG_BACKUP_MODE = 0x0061;    // 40097
constexpr uint16_t REG_HW_CURRENT_LIMIT = 0x0063; // 40099

constexpr uint16_t BACKUP_MODE_WATCHDOG_5S = 3;
}

PulsaresWallbox::PulsaresWallbox(Rs485Bus& bus, uint8_t slaveId)
    : bus_(bus), slaveId_(slaveId) {}

bool PulsaresWallbox::begin() {
  // Hardware failsafe: if ESP32 communication disappears, PULSARES falls back to 0 A after 5 s.
  if (!bus_.writeHolding(slaveId_, REG_BACKUP_CURRENT, 0)) {
    setCommError("backup-current");
    return false;
  }
  if (!bus_.writeHolding(slaveId_, REG_BACKUP_MODE, BACKUP_MODE_WATCHDOG_5S)) {
    setCommError("watchdog");
    return false;
  }
  if (!bus_.writeHolding(slaveId_, REG_MODBUS_CURRENT, 0)) {
    setCommError("startup-stop");
    return false;
  }

  uint16_t hardwareLimit = 0;
  if (readRegister(REG_HW_CURRENT_LIMIT, hardwareLimit)) {
    maxCurrentA_ = hardwareLimit == 1 ? 32.0f : 16.0f;
  }

  state_.enabled = false;
  state_.requestedCurrentA = 0.0f;
  return poll();
}

bool PulsaresWallbox::readRegister(uint16_t address, uint16_t& value) {
  return bus_.readHolding(slaveId_, address, &value, 1);
}

void PulsaresWallbox::setCommError(const char* context) {
  state_.online = false;
  state_.error = String(context) + ": Modbus 0x" + String(bus_.lastResultCode(), HEX);
}

String PulsaresWallbox::chargingStateText(uint16_t value) const {
  switch (value) {
    case 0: return "ready";
    case 1: return "blocked";
    case 2: return "vehicle_paused";
    case 3: return "charging";
    case 4: return "stopping";
    case 5: return "cp_error";
    case 6: return "resetting_error";
    case 7: return "wake_sequence";
    default: return "unknown";
  }
}

bool PulsaresWallbox::poll() {
  uint16_t error = 0;
  uint16_t plug = 0;
  uint16_t charging = 0;
  uint16_t temperature = 0;
  uint16_t validCurrent = 0;

  if (!readRegister(REG_ERROR, error) ||
      !readRegister(REG_PLUG_STATE, plug) ||
      !readRegister(REG_CHARGING_STATE, charging) ||
      !readRegister(REG_TEMPERATURE, temperature) ||
      !readRegister(REG_VALID_CURRENT, validCurrent)) {
    setCommError("poll");
    return false;
  }

  state_.online = true;
  state_.errorCode = static_cast<int>(error);
  state_.error = error == 0 ? "" : "PULSARES error " + String(error);
  state_.vehicleConnected = plug == 1;
  state_.charging = charging == 3;
  state_.state = chargingStateText(charging);
  state_.temperatureC = temperature == 255 ? NAN : static_cast<float>(temperature);
  state_.effectiveCurrentLimitA = static_cast<float>(validCurrent) / 1000.0f;
  return true;
}

bool PulsaresWallbox::setEnabled(bool enabled) {
  if (!enabled) {
    return safeStop();
  }

  // PULSARES has no separate remote-lock register in the public map.
  // Enabling therefore means allowing the next non-zero current command.
  state_.enabled = true;
  return true;
}

bool PulsaresWallbox::setCurrentLimitA(float ampere) {
  const float normalized = ga::normalizeCurrent(ampere, 6.0f, maxCurrentA_);
  const uint16_t milliamps = static_cast<uint16_t>(lroundf(normalized * 1000.0f));

  if (!bus_.writeHolding(slaveId_, REG_MODBUS_CURRENT, milliamps)) {
    setCommError("set-current");
    return false;
  }

  state_.requestedCurrentA = normalized;
  state_.enabled = normalized > 0.0f;
  return true;
}

bool PulsaresWallbox::safeStop() {
  if (!bus_.writeHolding(slaveId_, REG_MODBUS_CURRENT, 0)) {
    setCommError("safe-stop");
    return false;
  }

  state_.requestedCurrentA = 0.0f;
  state_.enabled = false;
  return true;
}

const WallboxState& PulsaresWallbox::state() const {
  return state_;
}

WallboxCapabilities PulsaresWallbox::capabilities() const {
  WallboxCapabilities caps;
  caps.powerMeasurement = false;
  caps.energyMeasurement = false;
  caps.currentMeasurement = false;
  caps.voltageMeasurement = false;
  caps.frequencyMeasurement = false;
  caps.phaseMeasurement = false;
  caps.temperatureMeasurement = true;
  caps.minCurrentA = 6.0f;
  caps.maxCurrentA = maxCurrentA_;
  return caps;
}

const char* PulsaresWallbox::driverName() const {
  return "pulsares_easycharge_basic";
}
