#ifndef DISPLAY_POWER_STAGES_H
#define DISPLAY_POWER_STAGES_H

#include <Arduino.h>

#define LOG_STAGE(name)              \
  do {                               \
    Serial.print("[STAGE] ");       \
    Serial.println(name);            \
  } while (0)

#endif
