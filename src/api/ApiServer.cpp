/*
File: src/api/ApiServer.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Implements the stable GA-WallBox REST API and optional Bearer authentication.
*/
#include "ApiServer.h"

#include <ArduinoJson.h>
#include <WiFi.h>
#include <time.h>

#include "../../include/config_defaults.h"
#include "../../include/version.h"

ApiServer::ApiServer(WallboxController& controller) : controller_(controller), server_(80) {}

void ApiServer::begin() {
  const char* headerKeys[] = {"Authorization"};
  server_.collectHeaders(headerKeys, 1);

  server_.on("/api/device", HTTP_GET, [this]() { handleDevice(); });
  server_.on("/api/status", HTTP_GET, [this]() { handleStatus(); });
  server_.on("/api/control", HTTP_POST, [this]() { handleControl(); });

  // Compatibility endpoints from the earlier Heidelberg gateway prototype.
  server_.on("/api/current", HTTP_POST, [this]() { handleCurrent(); });
  server_.on("/api/enable", HTTP_POST, [this]() { handleEnable(); });
  server_.on("/api/disable", HTTP_POST, [this]() { handleDisable(); });

  server_.on("/health", HTTP_GET, [this]() { handleHealth(); });
  server_.onNotFound([this]() {
    JsonDocument doc;
    doc["error"] = "not_found";
    sendJson(404, doc);
  });

  server_.begin();
  Serial.println("[API] HTTP server started on port 80");
}

void ApiServer::loop() {
  server_.handleClient();
}

bool ApiServer::authorized() const {
  const String configuredToken = GA_API_TOKEN;
  if (configuredToken.length() == 0) {
    return true;
  }

  const String expected = "Bearer " + configuredToken;
  return server_.header("Authorization") == expected;
}

void ApiServer::sendUnauthorized() {
  JsonDocument doc;
  doc["error"] = "unauthorized";
  sendJson(401, doc);
}

void ApiServer::sendJson(int statusCode, const JsonDocument& doc) {
  String body;
  serializeJson(doc, body);
  server_.send(statusCode, "application/json", body);
}

void ApiServer::setNullable(JsonDocument& doc, const char* key, float value) const {
  if (isnan(value)) {
    doc[key] = nullptr;
  } else {
    doc[key] = value;
  }
}

String ApiServer::deviceId() const {
  const uint64_t chip = ESP.getEfuseMac();
  char buffer[24];
  snprintf(buffer, sizeof(buffer), "GAWB-%04X%08X",
           static_cast<uint16_t>(chip >> 32),
           static_cast<uint32_t>(chip));
  return String(buffer);
}

String ApiServer::isoTimestamp() const {
  const time_t now = time(nullptr);
  if (now < 1700000000) {
    return "";
  }

  struct tm utc;
  gmtime_r(&now, &utc);

  char buffer[25];
  strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", &utc);
  return String(buffer);
}

void ApiServer::fillCapabilities(JsonObject caps) const {
  const WallboxCapabilities c = controller_.capabilities();
  caps["power_measurement"] = c.powerMeasurement;
  caps["energy_measurement"] = c.energyMeasurement;
  caps["current_measurement"] = c.currentMeasurement;
  caps["voltage_measurement"] = c.voltageMeasurement;
  caps["frequency_measurement"] = c.frequencyMeasurement;
  caps["phase_measurement"] = c.phaseMeasurement;
  caps["switching"] = c.switching;
  caps["power_control"] = c.powerControl;
  caps["current_limit_control"] = c.currentLimitControl;
  caps["vehicle_state"] = c.vehicleState;
  caps["temperature_measurement"] = c.temperatureMeasurement;
  caps["min_current_a"] = c.minCurrentA;
  caps["max_current_a"] = c.maxCurrentA;
}

void ApiServer::fillStatus(JsonDocument& doc) const {
  const WallboxState& s = controller_.state();

  doc["online"] = s.online;
  const String stamp = isoTimestamp();
  if (stamp.length() == 0) {
    doc["timestamp"] = nullptr;
  } else {
    doc["timestamp"] = stamp;
  }

  doc["state"] = s.state;
  doc["enabled"] = s.enabled;
  doc["switch_state"] = s.enabled;
  doc["vehicle_connected"] = s.vehicleConnected;
  doc["charging"] = s.charging;

  doc["requested_current_a"] = s.requestedCurrentA;
  setNullable(doc, "effective_current_limit_a", s.effectiveCurrentLimitA);

  setNullable(doc, "power_w", s.powerW);
  setNullable(doc, "energy_wh", s.energyWh);
  setNullable(doc, "apparent_power_va", s.apparentPowerVA);
  setNullable(doc, "apparent_energy_vah", s.apparentEnergyVAh);
  setNullable(doc, "frequency_hz", s.frequencyHz);

  setNullable(doc, "voltage_l1_v", s.voltageL1V);
  setNullable(doc, "voltage_l2_v", s.voltageL2V);
  setNullable(doc, "voltage_l3_v", s.voltageL3V);

  setNullable(doc, "current_l1_a", s.currentL1A);
  setNullable(doc, "current_l2_a", s.currentL2A);
  setNullable(doc, "current_l3_a", s.currentL3A);

  setNullable(doc, "temperature_c", s.temperatureC);

  doc["error_code"] = s.errorCode;
  if (s.error.length() == 0) {
    doc["error"] = nullptr;
  } else {
    doc["error"] = s.error;
  }

  if (controller_.lastControlAgeMs() == UINT32_MAX) {
    doc["control_age_ms"] = nullptr;
  } else {
    doc["control_age_ms"] = controller_.lastControlAgeMs();
  }
  doc["control_lease_expired"] = controller_.leaseExpired();
}

void ApiServer::handleDevice() {
  if (!authorized()) {
    sendUnauthorized();
    return;
  }

  JsonDocument doc;
  JsonObject device = doc["device"].to<JsonObject>();
  device["type"] = "GA-WallBox";
  device["vendor"] = "Gruber Automation";
  device["name"] = GA_DEVICE_NAME;
  device["device_id"] = deviceId();
  device["firmware"] = GA_FIRMWARE_VERSION;
  device["driver"] = controller_.driverName();

  JsonObject caps = doc["capabilities"].to<JsonObject>();
  fillCapabilities(caps);

  sendJson(200, doc);
}

void ApiServer::handleStatus() {
  if (!authorized()) {
    sendUnauthorized();
    return;
  }

  JsonDocument doc;
  fillStatus(doc);
  sendJson(200, doc);
}

void ApiServer::handleControl() {
  if (!authorized()) {
    sendUnauthorized();
    return;
  }

  JsonDocument input;
  const DeserializationError parseError = deserializeJson(input, server_.arg("plain"));
  if (parseError) {
    JsonDocument error;
    error["ok"] = false;
    error["error"] = "invalid_json";
    sendJson(400, error);
    return;
  }

  ControlRequest request;

  if (input["enabled"].is<bool>()) {
    request.hasEnabled = true;
    request.enabled = input["enabled"].as<bool>();
  }
  if (input["power_limit_w"].is<float>() || input["power_limit_w"].is<int>()) {
    request.hasPowerLimitW = true;
    request.powerLimitW = input["power_limit_w"].as<float>();
  }
  if (input["current_limit_a"].is<float>() || input["current_limit_a"].is<int>()) {
    request.hasCurrentLimitA = true;
    request.currentLimitA = input["current_limit_a"].as<float>();
  }

  String errorText;
  if (!controller_.applyControl(request, errorText)) {
    JsonDocument error;
    error["ok"] = false;
    error["error"] = errorText.length() ? errorText : "control_rejected";
    sendJson(400, error);
    return;
  }

  JsonDocument response;
  response["ok"] = true;
  JsonObject status = response["status"].to<JsonObject>();

  JsonDocument normalized;
  fillStatus(normalized);
  for (JsonPair pair : normalized.as<JsonObject>()) {
    status[pair.key()] = pair.value();
  }

  sendJson(200, response);
}

void ApiServer::handleCurrent() {
  if (!authorized()) {
    sendUnauthorized();
    return;
  }

  JsonDocument input;
  if (deserializeJson(input, server_.arg("plain"))) {
    JsonDocument error;
    error["ok"] = false;
    error["error"] = "invalid_json";
    sendJson(400, error);
    return;
  }

  if (!input["ampere"].is<float>() && !input["ampere"].is<int>()) {
    JsonDocument error;
    error["ok"] = false;
    error["error"] = "ampere_required";
    sendJson(400, error);
    return;
  }

  ControlRequest request;
  request.hasCurrentLimitA = true;
  request.currentLimitA = input["ampere"].as<float>();

  String errorText;
  JsonDocument response;
  response["ok"] = controller_.applyControl(request, errorText);
  if (!response["ok"].as<bool>()) {
    response["error"] = errorText;
    sendJson(400, response);
    return;
  }

  sendJson(200, response);
}

void ApiServer::handleEnable() {
  if (!authorized()) {
    sendUnauthorized();
    return;
  }

  ControlRequest request;
  request.hasEnabled = true;
  request.enabled = true;

  String errorText;
  JsonDocument response;
  response["ok"] = controller_.applyControl(request, errorText);
  if (!response["ok"].as<bool>()) {
    response["error"] = errorText;
    sendJson(400, response);
    return;
  }
  sendJson(200, response);
}

void ApiServer::handleDisable() {
  if (!authorized()) {
    sendUnauthorized();
    return;
  }

  ControlRequest request;
  request.hasEnabled = true;
  request.enabled = false;

  String errorText;
  JsonDocument response;
  response["ok"] = controller_.applyControl(request, errorText);
  if (!response["ok"].as<bool>()) {
    response["error"] = errorText;
    sendJson(400, response);
    return;
  }
  sendJson(200, response);
}

void ApiServer::handleHealth() {
  JsonDocument doc;
  doc["ok"] = true;
  doc["firmware"] = GA_FIRMWARE_VERSION;
  doc["wifi"] = WiFi.status() == WL_CONNECTED;
  doc["wallbox_online"] = controller_.state().online;
  sendJson(200, doc);
}
