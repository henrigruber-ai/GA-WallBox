/*
File: src/communication/Rs485Bus.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Synchronous Modbus RTU master wrapper with timeout and failure cooldown.
*/
#pragma once

#include <Arduino.h>
#include <ModbusRTU.h>

class Rs485Bus {
 public:
  Rs485Bus(HardwareSerial& serial, int rxPin, int txPin, int dePin, uint32_t baud,
           uint32_t timeoutMs, uint32_t cooldownMs);

  void begin();
  bool readHolding(uint8_t slaveId, uint16_t address, uint16_t* data, uint16_t count = 1);
  bool readInput(uint8_t slaveId, uint16_t address, uint16_t* data, uint16_t count = 1);
  bool writeHolding(uint8_t slaveId, uint16_t address, uint16_t value);

  bool inCooldown() const;
  uint8_t lastResultCode() const;

 private:
  bool waitForTransaction(uint32_t startedAt);
  bool canStart() const;
  void markFailure(uint8_t resultCode);

  HardwareSerial& serial_;
  ModbusRTU modbus_;
  int rxPin_;
  int txPin_;
  int dePin_;
  uint32_t baud_;
  uint32_t timeoutMs_;
  uint32_t cooldownMs_;

  volatile bool transactionDone_ = false;
  volatile uint8_t resultCode_ = static_cast<uint8_t>(Modbus::EX_SUCCESS);
  uint32_t cooldownUntilMs_ = 0;
};
