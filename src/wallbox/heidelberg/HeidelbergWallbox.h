/*
File: src/wallbox/heidelberg/HeidelbergWallbox.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Driver for Heidelberg Wallbox Energy Control over Modbus RTU.
*/
#pragma once

#include "../../communication/Rs485Bus.h"
#include "../../core/IWallbox.h"

class HeidelbergWallbox final : public IWallbox {
 public:
  HeidelbergWallbox(Rs485Bus& bus, uint8_t slaveId);

  bool begin() override;
  bool poll() override;
  bool setEnabled(bool enabled) override;
  bool setCurrentLimitA(float ampere) override;
  bool safeStop() override;

  const WallboxState& state() const override;
  WallboxCapabilities capabilities() const override;
  const char* driverName() const override;

 private:
  void setCommError(const char* context);
  String chargingStateText(uint16_t value) const;

  Rs485Bus& bus_;
  uint8_t slaveId_;
  WallboxState state_;
  float minCurrentA_ = 6.0f;
  float maxCurrentA_ = 16.0f;
};
