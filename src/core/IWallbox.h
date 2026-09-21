/*
File: src/core/IWallbox.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Common driver contract for all supported wallbox hardware.
*/
#pragma once

#include "WallboxTypes.h"

class IWallbox {
 public:
  virtual ~IWallbox() = default;

  virtual bool begin() = 0;
  virtual bool poll() = 0;

  virtual bool setEnabled(bool enabled) = 0;
  virtual bool setCurrentLimitA(float ampere) = 0;
  virtual bool safeStop() = 0;

  virtual const WallboxState& state() const = 0;
  virtual WallboxCapabilities capabilities() const = 0;

  virtual const char* driverName() const = 0;
};
