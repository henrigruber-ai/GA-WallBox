/*
File: src/main.cpp
Version: 0.1.0
Date: 2026-09-21
Purpose: Selects a wallbox driver and starts Wi-Fi, Modbus, control and REST services.
*/
#include <Arduino.h>
#include <time.h>

#include "../include/config_defaults.h"
#include "api/ApiServer.h"
#include "communication/Rs485Bus.h"
#include "core/WallboxController.h"
#include "wifi/WifiProvisioning.h"

#if defined(GA_WALLBOX_DRIVER_PULSARES)
#include "wallbox/pulsares/PulsaresWallbox.h"
#elif defined(GA_WALLBOX_DRIVER_HEIDELBERG)
#include "wallbox/heidelberg/HeidelbergWallbox.h"
#else
#error "Select GA_WALLBOX_DRIVER_PULSARES or GA_WALLBOX_DRIVER_HEIDELBERG."
#endif

Rs485Bus rs485(Serial2, GA_RS485_RX_PIN, GA_RS485_TX_PIN, GA_RS485_DE_PIN,
               GA_MODBUS_BAUD, GA_MODBUS_TIMEOUT_MS, GA_MODBUS_COOLDOWN_MS);

#if defined(GA_WALLBOX_DRIVER_PULSARES)
PulsaresWallbox wallbox(rs485, GA_MODBUS_SLAVE_ID);
#elif defined(GA_WALLBOX_DRIVER_HEIDELBERG)
HeidelbergWallbox wallbox(rs485, GA_MODBUS_SLAVE_ID);
#endif

WallboxController controller(wallbox);
ApiServer api(controller);
WifiProvisioning wifi;

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println();
  Serial.println("GA-WallBox starting...");

  rs485.begin();

  const bool wallboxReady = controller.begin();
  Serial.printf("[WALLBOX] driver=%s ready=%s\n",
                controller.driverName(), wallboxReady ? "true" : "false");

  if (!wifi.begin()) {
    controller.safeStop("wifi unavailable");
  } else {
    configTime(0, 0, "pool.ntp.org", "time.cloudflare.com");
  }

  api.begin();
}

void loop() {
  controller.loop();
  api.loop();
  delay(2);
}
