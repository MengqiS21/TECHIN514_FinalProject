#ifndef SENSOR_CONFIG_H
#define SENSOR_CONFIG_H

#include <stdint.h>

static constexpr const char* kBleDeviceName = "TECHIN514_SENSOR";
static constexpr const char* kBleServiceUuid = "6f7f0001-8f3b-4c3a-a39a-3f8ec4dca101";
static constexpr const char* kBleCharUuid = "6f7f0002-8f3b-4c3a-a39a-3f8ec4dca101";

static constexpr uint32_t kImuSampleWindowMs = 1500;
static constexpr uint32_t kImuSamplePeriodMs = 40;
static constexpr uint8_t kImuInitRetries = 3;

static constexpr uint32_t kBleConnectTimeoutMs = 5000;
static constexpr uint32_t kBlePostNotifyDelayMs = 120;

static constexpr uint32_t kDeepSleepSeconds = 30;

#endif
