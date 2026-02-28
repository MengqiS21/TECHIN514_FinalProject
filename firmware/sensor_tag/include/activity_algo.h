#ifndef SENSOR_ACTIVITY_ALGO_H
#define SENSOR_ACTIVITY_ALGO_H

#include <Arduino.h>

namespace activity {

inline uint16_t clampToPercent(float value) {
  if (value < 0.0f) {
    return 0;
  }
  if (value > 100.0f) {
    return 100;
  }
  return static_cast<uint16_t>(value + 0.5f);
}

inline uint16_t computeActivityFromAverage(float avgSumAbs) {
  // Simple linear scale: 0g -> 0, 3g total movement -> 100.
  const float scaled = (avgSumAbs / 3.0f) * 100.0f;
  return clampToPercent(scaled);
}

}  // namespace activity

#endif
