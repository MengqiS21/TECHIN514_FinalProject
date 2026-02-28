#ifndef DISPLAY_CONFIG_H
#define DISPLAY_CONFIG_H

#include <stdint.h>

static constexpr const char* kBleDeviceName = "TECHIN514_DISPLAY";
static constexpr const char* kBleServiceUuid = "6f7f0001-8f3b-4c3a-a39a-3f8ec4dca101";
static constexpr const char* kBleCharUuid = "6f7f0002-8f3b-4c3a-a39a-3f8ec4dca101";

static constexpr uint32_t kBleScanSeconds = 4;
static constexpr uint32_t kDataWaitTimeoutMs = 8000;
static constexpr uint32_t kIdleDelayMs = 300;

static constexpr int kGaugeMaxSteps = 600;
static constexpr uint32_t kMotorStepDelayUs = 1200;

#endif
