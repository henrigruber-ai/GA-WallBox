/*
File: src/wifi/WifiProvisioning.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Wi-Fi connection and captive-portal provisioning for the ESP32 gateway.
*/
#pragma once

#include <Arduino.h>

class WifiProvisioning {
 public:
  bool begin();
  String ipAddress() const;
};
