/*
File: src/core/WallboxController.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Implements common wallbox control, periodic polling and safe fallback on stale commands.
*/
#include "WallboxController.h"

#include "../../include/config_defaults.h"
#include "PowerLimit.h"

WallboxController::WallboxController(IWallbox& wallbox) : wallbox_(wallbox) {}

bool WallboxController::begin() {
  const bool ok = wallbox_.begin();
  lastPollMs_ = millis();
  return ok;
}

void WallboxController::loop() {
  const uint32_t now = millis();

  if ((now - lastPollMs_) >= GA_STATUS_POLL_MS) {
    wallbox_.poll();
    lastPollMs_ = now;
  }

  if (hasControlLease_ && (now - lastControlMs_) > GA_CONTROL_LEASE_MS && !leaseStopApplied_) {
    safeStop("control lease expired");
    leaseStopApplied_ = true;
  }
}

float WallboxController::requestedCurrentFrom(const ControlRequest& request) const {
  const WallboxCapabilities caps = wallbox_.capabilities();

  if (request.hasCurrentLimitA) {
    return ga::normalizeCurrent(request.currentLimitA, caps.minCurrentA, caps.maxCurrentA);
  }

  if (request.hasPowerLimitW) {
    const float currentA = ga::currentFromPower(request.powerLimitW, 3, 230.0f);
    return ga::normalizeCurrent(currentA, caps.minCurrentA, caps.maxCurrentA);
  }

  return wallbox_.state().requestedCurrentA;
}

bool WallboxController::applyControl(const ControlRequest& request, String& error) {
  if (!request.hasEnabled && !request.hasPowerLimitW && !request.hasCurrentLimitA) {
    error = "control request is empty";
    return false;
  }

  if (request.hasPowerLimitW && request.powerLimitW < 0.0f) {
    error = "power_limit_w must be >= 0";
    return false;
  }

  if (request.hasCurrentLimitA && request.currentLimitA < 0.0f) {
    error = "current_limit_a must be >= 0";
    return false;
  }

  if (request.hasEnabled && !request.enabled) {
    if (!wallbox_.safeStop()) {
      error = wallbox_.state().error;
      return false;
    }
  } else {
    if (request.hasEnabled && request.enabled && !wallbox_.setEnabled(true)) {
      error = wallbox_.state().error;
      return false;
    }

    if (request.hasPowerLimitW || request.hasCurrentLimitA) {
      const float currentA = requestedCurrentFrom(request);
      if (!wallbox_.setCurrentLimitA(currentA)) {
        error = wallbox_.state().error;
        return false;
      }

      if (currentA > 0.0f && !wallbox_.state().enabled) {
        if (!wallbox_.setEnabled(true)) {
          error = wallbox_.state().error;
          return false;
        }
      }

      if (currentA <= 0.0f) {
        wallbox_.safeStop();
      }
    }
  }

  hasControlLease_ = true;
  leaseStopApplied_ = false;
  lastControlMs_ = millis();
  return true;
}

bool WallboxController::safeStop(const char* reason) {
  const bool ok = wallbox_.safeStop();
  if (!ok) {
    Serial.printf("[SAFETY] safeStop failed (%s): %s\n", reason, wallbox_.state().error.c_str());
  } else {
    Serial.printf("[SAFETY] safeStop applied: %s\n", reason);
  }
  return ok;
}

const WallboxState& WallboxController::state() const {
  return wallbox_.state();
}

WallboxCapabilities WallboxController::capabilities() const {
  return wallbox_.capabilities();
}

const char* WallboxController::driverName() const {
  return wallbox_.driverName();
}

uint32_t WallboxController::lastControlAgeMs() const {
  if (!hasControlLease_) {
    return UINT32_MAX;
  }
  return millis() - lastControlMs_;
}

bool WallboxController::leaseExpired() const {
  return hasControlLease_ && lastControlAgeMs() > GA_CONTROL_LEASE_MS;
}
