/*
File: include/config_defaults.h
Version: 0.1.0
Date: 2026-09-21
Purpose: Safe build defaults for pins, timing and optional authentication.
*/
#pragma once

#ifndef GA_RS485_RX_PIN
#define GA_RS485_RX_PIN 16
#endif

#ifndef GA_RS485_TX_PIN
#define GA_RS485_TX_PIN 17
#endif

#ifndef GA_RS485_DE_PIN
#define GA_RS485_DE_PIN 4
#endif

#ifndef GA_MODBUS_BAUD
#define GA_MODBUS_BAUD 9600
#endif

#ifndef GA_MODBUS_SLAVE_ID
#define GA_MODBUS_SLAVE_ID 2
#endif

#ifndef GA_CONTROL_LEASE_MS
#define GA_CONTROL_LEASE_MS 15000UL
#endif

#ifndef GA_MODBUS_COOLDOWN_MS
#define GA_MODBUS_COOLDOWN_MS 7000UL
#endif

#ifndef GA_API_TOKEN
#define GA_API_TOKEN ""
#endif

#ifndef GA_DEVICE_NAME
#define GA_DEVICE_NAME "GA-WallBox"
#endif

#define GA_STATUS_POLL_MS 1000UL
#define GA_MODBUS_TIMEOUT_MS 1200UL
