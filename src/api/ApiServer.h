/*
File: src/api/ApiServer.h
Version: 0.1.0
Date: 2026-09-21
Purpose: HTTP/JSON contract between the Fronius-Regler and GA-WallBox.
*/
#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WebServer.h>

#include "../core/WallboxController.h"

class ApiServer {
 public:
  explicit ApiServer(WallboxController& controller);

  void begin();
  void loop();

 private:
  bool authorized();
  void sendUnauthorized();
  void sendJson(int statusCode, const JsonDocument& doc);
  void fillStatus(JsonDocument& doc) const;
  void fillCapabilities(JsonObject caps) const;
  void setNullable(JsonDocument& doc, const char* key, float value) const;
  String isoTimestamp() const;
  String deviceId() const;

  void handleDevice();
  void handleStatus();
  void handleControl();
  void handleCurrent();
  void handleEnable();
  void handleDisable();
  void handleHealth();

  WallboxController& controller_;
  WebServer server_;
};
