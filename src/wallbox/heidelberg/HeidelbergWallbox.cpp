/*
File: src/wallbox/heidelberg/HeidelbergWallbox.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Implements Heidelberg Energy Control telemetry, current control and failsafe setup.
*/
#include "HeidelbergWallbox.h"

#include "../../core/PowerLimit.h"

namespace {
constexpr uint16_t REG_INPUT_START = 4;
constexpr uint16_t REG_INPUT_COUNT = 15;  // 4..18

constexpr uint16_t REG_HW_MAX_CURRENT = 100;
constexpr uint16_t REG_HW_MIN_CURRENT = 101;

constexpr uint16_t REG_WATCHDOG_MS = 257;
constexpr uint16_t REG_REMOTE_LOCK = 259;
constexpr uint16_t REG_MAX_CURRENT = 261;
constexpr uint16_t REG_FAILSAFE_CURRENT = 262;

constexpr uint16_t WATCHDOG_MS = 5000;
}

HeidelbergWallbox::HeidelbergWallbox(Rs485Bus& bus, uint8_t slaveId)
    : bus_(bus), slaveId_(slaveId) {}

bool HeidelbergWallbox::begin() {
  // Local safety first: communication loss results in 0 A.
  if (!bus_.writeHolding(slaveId_, REG_FAILSAFE_CURRENT, 0)) {
    setCommError("failsafe-current");
    return false;
  }
  if (!bus_.writeHolding(slaveId_, REG_WATCHDOG_MS, WATCHDOG_MS)) {
    setCommError("watchdog");
    return false;
  }
  if (!safeStop()) {
    return false;
  }

  uint16_t limits[2] = {0, 0};
  if (bus_.readInput(slaveId_, REG_HW_MAX_CURRENT, limits, 2)) {
    if (limits[0] >= 6 && limits[0] <= 32) {
      maxCurrentA_ = static_cast<float>(limits[0]);
    }
    if (limits[1] >= 6 && limits[1] <= maxCurrentA_) {
      minCurrentA_ = static_cast<float>(limits[1]);
    }
  }

  return poll();
}

void HeidelbergWallbox::setCommError(const char* context) {
  state_.online = false;
  state_.error = String(context) + ": Modbus 0x" + String(bus_.lastResultCode(), HEX);
}

String HeidelbergWallbox::chargingStateText(uint16_t value) const {
  switch (value) {
    case 2: return "A1_no_vehicle";
    case 3: return "A2_no_vehicle";
    case 4: return "B1_vehicle_connected";
    case 5: return "B2_charge_requested";
    case 6: return "C1_charging_allowed";
    case 7: return "C2_charging_allowed";
    case 8: return "derating";
    case 9: return "E_error";
    case 10: return "F_error";
    case 11: return "error";
    default: return "unknown";
  }
}

bool HeidelbergWallbox::poll() {
  uint16_t regs[REG_INPUT_COUNT] = {0};

  if (!bus_.readInput(slaveId_, REG_INPUT_START, regs, REG_INPUT_COUNT)) {
    setCommError("poll");
    return false;
  }

  const uint16_t chargingState = regs[5 - REG_INPUT_START];
  const uint16_t currentL1 = regs[6 - REG_INPUT_START];
  const uint16_t currentL2 = regs[7 - REG_INPUT_START];
  const uint16_t currentL3 = regs[8 - REG_INPUT_START];
  const int16_t temperature = static_cast<int16_t>(regs[9 - REG_INPUT_START]);
  const uint16_t voltageL1 = regs[10 - REG_INPUT_START];
  const uint16_t voltageL2 = regs[11 - REG_INPUT_START];
  const uint16_t voltageL3 = regs[12 - REG_INPUT_START];
  const uint16_t externalLock = regs[13 - REG_INPUT_START];
  const uint16_t apparentPower = regs[14 - REG_INPUT_START];
  const uint16_t energyInstallHigh = regs[17 - REG_INPUT_START];
  const uint16_t energyInstallLow = regs[18 - REG_INPUT_START];

  const float energyVAh =
      static_cast<float>((static_cast<uint32_t>(energyInstallHigh) << 16) | energyInstallLow);

  state_.online = true;
  state_.errorCode = (chargingState >= 9) ? static_cast<int>(chargingState) : 0;
  state_.error = state_.errorCode == 0 ? "" : "Heidelberg charging state " + String(chargingState);
  state_.vehicleConnected = chargingState >= 4;
  state_.charging = chargingState == 6 || chargingState == 7 || chargingState == 8;
  state_.enabled = externalLock == 1;
  state_.state = chargingStateText(chargingState);

  state_.currentL1A = static_cast<float>(currentL1) / 10.0f;
  state_.currentL2A = static_cast<float>(currentL2) / 10.0f;
  state_.currentL3A = static_cast<float>(currentL3) / 10.0f;

  state_.voltageL1V = static_cast<float>(voltageL1);
  state_.voltageL2V = static_cast<float>(voltageL2);
  state_.voltageL3V = static_cast<float>(voltageL3);

  state_.temperatureC = static_cast<float>(temperature) / 10.0f;

  // Heidelberg documents these as internal VA / VAh values, not billing-grade W / Wh.
  // The compatibility fields remain populated for the existing Fronius adapter contract;
  // the semantically correct VA/VAh fields are exposed in parallel.
  state_.apparentPowerVA = static_cast<float>(apparentPower);
  state_.apparentEnergyVAh = energyVAh;
  state_.powerW = state_.apparentPowerVA;
  state_.energyWh = state_.apparentEnergyVAh;

  return true;
}

bool HeidelbergWallbox::setEnabled(bool enabled) {
  if (!enabled) {
    return safeStop();
  }

  if (!bus_.writeHolding(slaveId_, REG_REMOTE_LOCK, 1)) {
    setCommError("unlock");
    return false;
  }

  state_.enabled = true;
  return true;
}

bool HeidelbergWallbox::setCurrentLimitA(float ampere) {
  const float normalized = ga::normalizeCurrent(ampere, minCurrentA_, maxCurrentA_);
  const uint16_t tenthsAmp = static_cast<uint16_t>(lroundf(normalized * 10.0f));

  if (!bus_.writeHolding(slaveId_, REG_MAX_CURRENT, tenthsAmp)) {
    setCommError("set-current");
    return false;
  }

  state_.requestedCurrentA = normalized;
  return true;
}

bool HeidelbergWallbox::safeStop() {
  bool ok = true;

  if (!bus_.writeHolding(slaveId_, REG_MAX_CURRENT, 0)) {
    setCommError("safe-stop-current");
    ok = false;
  }

  if (ok && !bus_.writeHolding(slaveId_, REG_REMOTE_LOCK, 0)) {
    setCommError("safe-stop-lock");
    ok = false;
  }

  if (ok) {
    state_.requestedCurrentA = 0.0f;
    state_.enabled = false;
  }
  return ok;
}

const WallboxState& HeidelbergWallbox::state() const {
  return state_;
}

WallboxCapabilities HeidelbergWallbox::capabilities() const {
  WallboxCapabilities caps;
  caps.powerMeasurement = true;
  caps.energyMeasurement = true;
  caps.currentMeasurement = true;
  caps.voltageMeasurement = true;
  caps.frequencyMeasurement = false;
  caps.phaseMeasurement = true;
  caps.temperatureMeasurement = true;
  caps.minCurrentA = minCurrentA_;
  caps.maxCurrentA = maxCurrentA_;
  return caps;
}

const char* HeidelbergWallbox::driverName() const {
  return "heidelberg_energy_control";
}
