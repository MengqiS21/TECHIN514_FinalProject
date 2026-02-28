#ifndef SENSOR_BLE_PROTOCOL_H
#define SENSOR_BLE_PROTOCOL_H

#include <stdint.h>
#include "config.h"

#pragma pack(push, 1)
struct ActivityPayload {
  uint32_t seq;
  uint16_t activity;
  uint16_t battery_mv;
};
#pragma pack(pop)

static constexpr const char* BLE_SERVICE_UUID = kBleServiceUuid;
static constexpr const char* BLE_CHAR_UUID = kBleCharUuid;

#endif
