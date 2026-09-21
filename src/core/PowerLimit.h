/*
File: src/core/PowerLimit.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Pure helper functions for translating power limits into safe AC current limits.
*/
#pragma once

#include <math.h>

namespace ga {

inline float currentFromPower(float powerW, int phases = 3, float phaseVoltageV = 230.0f) {
  if (powerW <= 0.0f || phases <= 0 || phaseVoltageV <= 0.0f) {
    return 0.0f;
  }
  return powerW / (static_cast<float>(phases) * phaseVoltageV);
}

inline float normalizeCurrent(float requestedA, float minA, float maxA) {
  if (requestedA <= 0.0f) {
    return 0.0f;
  }
  if (requestedA < minA) {
    return 0.0f;
  }
  if (requestedA > maxA) {
    return maxA;
  }
  return requestedA;
}

}  // namespace ga
