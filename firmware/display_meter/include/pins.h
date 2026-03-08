#ifndef DISPLAY_PINS_H
#define DISPLAY_PINS_H

// X27.168 motor wiring (current connection):
// left-top -> D1, left-bottom -> D0, right-top -> D2, right-bottom -> D3
static constexpr int PIN_MOTOR_IN1 = D1;

static constexpr int PIN_MOTOR_IN2 = D0;

static constexpr int PIN_MOTOR_IN3 = D2;

static constexpr int PIN_MOTOR_IN4 = D3;

// Optional status LED pin. Keep -1 if unused.
static constexpr int PIN_STATUS_LED = -1;

#endif
