/*
File: src/core/WallboxController.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Vendor-neutral polling, control translation and local control-lease failsafe.
*/
#pragma once

#include <Arduino.h>

#include "IWallbox.h"

struct ControlRequest {
  bool hasEnabled = false;
  bool enabled = false;

  bool hasPowerLimitW = false;
  float powerLimitW = 0.0f;

  bool hasCurrentLimitA = false;
  float currentLimitA = 0.0f;
};

class WallboxController {
 public:
  explicit WallboxController(IWallbox& wallbox);

  bool begin();
  void loop();

  bool applyControl(const ControlRequest& request, String& error);
  bool safeStop(const char* reason);

  const WallboxState& state() const;
  WallboxCapabilities capabilities() const;
  const char* driverName() const;

  uint32_t lastControlAgeMs() const;
  bool leaseExpired() const;

 private:
  float requestedCurrentFrom(const ControlRequest& request) const;

  IWallbox& wallbox_;
  uint32_t lastPollMs_ = 0;
  uint32_t lastControlMs_ = 0;
  bool hasControlLease_ = false;
  bool leaseStopApplied_ = false;
};
