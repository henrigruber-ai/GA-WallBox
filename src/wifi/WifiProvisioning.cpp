/*
File: src/wifi/WifiProvisioning.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Connects stored Wi-Fi credentials or starts a temporary GA-WallBox setup portal.
*/
#include "WifiProvisioning.h"

#include <WiFi.h>
#include <WiFiManager.h>

bool WifiProvisioning::begin() {
  WiFi.mode(WIFI_STA);

  WiFiManager manager;
  manager.setConfigPortalTimeout(180);
  manager.setConnectTimeout(20);

  const bool connected = manager.autoConnect("GA-WallBox-Setup");
  if (!connected) {
    Serial.println("[WIFI] connection/provisioning failed");
    return false;
  }

  Serial.printf("[WIFI] connected: %s\n", WiFi.localIP().toString().c_str());
  return true;
}

String WifiProvisioning::ipAddress() const {
  return WiFi.localIP().toString();
}
