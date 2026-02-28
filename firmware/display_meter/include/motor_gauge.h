#ifndef DISPLAY_MOTOR_GAUGE_H
#define DISPLAY_MOTOR_GAUGE_H

#include <Arduino.h>

#include "config.h"
#include "pins.h"

namespace motor_gauge {

namespace {
constexpr int kPins[4] = {PIN_MOTOR_IN1, PIN_MOTOR_IN2, PIN_MOTOR_IN3, PIN_MOTOR_IN4};
constexpr uint8_t kHalfStepSeq[8][4] = {
    {1, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 1, 0},
    {0, 0, 1, 1},
    {0, 0, 0, 1},
    {1, 0, 0, 1},
};
}  // namespace

inline bool& motorReady() {
  static bool ready = false;
  return ready;
}

inline int& currentStep() {
  static int step = 0;
  return step;
}

inline int& currentPos() {
  static int pos = 0;
  return pos;
}

inline bool init() {
  for (int i = 0; i < 4; ++i) {
    if (kPins[i] < 0) {
      return false;
    }
  }

  for (int i = 0; i < 4; ++i) {
    pinMode(kPins[i], OUTPUT);
    digitalWrite(kPins[i], LOW);
  }

  motorReady() = true;
  currentStep() = 0;
  currentPos() = 0;
  return true;
}

inline void applyStep(uint8_t seqIndex) {
  for (int i = 0; i < 4; ++i) {
    digitalWrite(kPins[i], kHalfStepSeq[seqIndex][i]);
  }
}

inline void stepMotor(int dir) {
  if (dir > 0) {
    currentStep() = (currentStep() + 1) & 0x07;
  } else {
    currentStep() = (currentStep() + 7) & 0x07;
  }
  applyStep(static_cast<uint8_t>(currentStep()));
  delayMicroseconds(kMotorStepDelayUs);
}

inline int clampTarget(int raw) {
  if (raw < 0) {
    return 0;
  }
  if (raw > kGaugeMaxSteps) {
    return kGaugeMaxSteps;
  }
  return raw;
}

inline void setTargetFromActivity(uint16_t activity) {
  if (!motorReady()) {
    static bool warned = false;
    if (!warned) {
      Serial.println("WARNING: Motor pins unset or not initialized; skipping motor movement.");
      warned = true;
    }
    return;
  }

  const uint16_t clamped = (activity > 100) ? 100 : activity;
  const int target = clampTarget((clamped * kGaugeMaxSteps) / 100);

  while (currentPos() < target) {
    stepMotor(+1);
    ++currentPos();
  }
  while (currentPos() > target) {
    stepMotor(-1);
    --currentPos();
  }
}

}  // namespace motor_gauge

#endif
