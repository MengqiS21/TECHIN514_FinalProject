#ifndef DISPLAY_LED_STATUS_H
#define DISPLAY_LED_STATUS_H

#include <Arduino.h>
#include "pins.h"

namespace led_status {

inline bool init() {
  if (PIN_STATUS_LED < 0) {
    return false;
  }
  pinMode(PIN_STATUS_LED, OUTPUT);
  digitalWrite(PIN_STATUS_LED, LOW);
  return true;
}

inline void setFromActivity(uint16_t activity) {
  if (PIN_STATUS_LED < 0) {
    return;
  }
  digitalWrite(PIN_STATUS_LED, activity > 50 ? HIGH : LOW);
}

}  // namespace led_status

#endif
