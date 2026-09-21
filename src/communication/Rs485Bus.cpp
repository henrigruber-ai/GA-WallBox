/*
File: src/communication/Rs485Bus.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Implements guarded synchronous access to asynchronous Modbus RTU transactions.
*/
#include "Rs485Bus.h"

Rs485Bus::Rs485Bus(HardwareSerial& serial, int rxPin, int txPin, int dePin, uint32_t baud,
                   uint32_t timeoutMs, uint32_t cooldownMs)
    : serial_(serial),
      rxPin_(rxPin),
      txPin_(txPin),
      dePin_(dePin),
      baud_(baud),
      timeoutMs_(timeoutMs),
      cooldownMs_(cooldownMs) {}

void Rs485Bus::begin() {
  serial_.begin(baud_, SERIAL_8E1, rxPin_, txPin_);
  modbus_.begin(&serial_, dePin_);
  modbus_.master();
}

bool Rs485Bus::canStart() const {
  return !inCooldown() && modbus_.slave() == 0;
}

bool Rs485Bus::inCooldown() const {
  if (cooldownUntilMs_ == 0) {
    return false;
  }
  return static_cast<int32_t>(cooldownUntilMs_ - millis()) > 0;
}

uint8_t Rs485Bus::lastResultCode() const {
  return resultCode_;
}

void Rs485Bus::markFailure(uint8_t resultCode) {
  resultCode_ = resultCode;
  cooldownUntilMs_ = millis() + cooldownMs_;
}

bool Rs485Bus::waitForTransaction(uint32_t startedAt) {
  while (modbus_.slave() != 0 && (millis() - startedAt) < timeoutMs_) {
    modbus_.task();
    delay(1);
  }

  if (!transactionDone_ || modbus_.slave() != 0) {
    markFailure(static_cast<uint8_t>(Modbus::EX_TIMEOUT));
    return false;
  }

  if (resultCode_ != static_cast<uint8_t>(Modbus::EX_SUCCESS)) {
    markFailure(resultCode_);
    return false;
  }

  cooldownUntilMs_ = 0;
  return true;
}

bool Rs485Bus::readHolding(uint8_t slaveId, uint16_t address, uint16_t* data, uint16_t count) {
  if (!canStart()) {
    return false;
  }

  transactionDone_ = false;
  resultCode_ = static_cast<uint8_t>(Modbus::EX_GENERAL_FAILURE);

  const uint16_t transaction = modbus_.readHreg(
      slaveId, address, data, count,
      [this](Modbus::ResultCode event, uint16_t, void*) {
        resultCode_ = static_cast<uint8_t>(event);
        transactionDone_ = true;
        return true;
      });

  if (transaction == 0) {
    markFailure(static_cast<uint8_t>(Modbus::EX_GENERAL_FAILURE));
    return false;
  }

  return waitForTransaction(millis());
}

bool Rs485Bus::readInput(uint8_t slaveId, uint16_t address, uint16_t* data, uint16_t count) {
  if (!canStart()) {
    return false;
  }

  transactionDone_ = false;
  resultCode_ = static_cast<uint8_t>(Modbus::EX_GENERAL_FAILURE);

  const uint16_t transaction = modbus_.readIreg(
      slaveId, address, data, count,
      [this](Modbus::ResultCode event, uint16_t, void*) {
        resultCode_ = static_cast<uint8_t>(event);
        transactionDone_ = true;
        return true;
      });

  if (transaction == 0) {
    markFailure(static_cast<uint8_t>(Modbus::EX_GENERAL_FAILURE));
    return false;
  }

  return waitForTransaction(millis());
}

bool Rs485Bus::writeHolding(uint8_t slaveId, uint16_t address, uint16_t value) {
  if (!canStart()) {
    return false;
  }

  transactionDone_ = false;
  resultCode_ = static_cast<uint8_t>(Modbus::EX_GENERAL_FAILURE);

  const uint16_t transaction = modbus_.writeHreg(
      slaveId, address, value,
      [this](Modbus::ResultCode event, uint16_t, void*) {
        resultCode_ = static_cast<uint8_t>(event);
        transactionDone_ = true;
        return true;
      });

  if (transaction == 0) {
    markFailure(static_cast<uint8_t>(Modbus::EX_GENERAL_FAILURE));
    return false;
  }

  return waitForTransaction(millis());
}
