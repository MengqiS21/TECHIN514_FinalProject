#include <Arduino.h>

// ---- XIAO ESP32-C3 Pin Mapping ----
#define LL D0   // (GPIO2)
#define UL D1   // (GPIO3)
#define UR D2   // (GPIO4)
#define LR D3   // (GPIO5)

#define LED_PIN D7    // LED
#define SWITCH_PIN D5

// 4-step sequence
const uint8_t seq[4][4] = {
  {1, 0, 1, 0},
  {0, 1, 1, 0},
  {0, 1, 0, 1},
  {1, 0, 0, 1}
};

int stepIndex = 0;

// 3ms
const unsigned long STEP_INTERVAL_US = 3000;
unsigned long lastStepUs = 0;

void applyStep(int s) {
  digitalWrite(UL, seq[s][0]);
  digitalWrite(UR, seq[s][1]);
  digitalWrite(LL, seq[s][2]);
  digitalWrite(LR, seq[s][3]);
}

void motorOff() {
  digitalWrite(UL, LOW);
  digitalWrite(UR, LOW);
  digitalWrite(LL, LOW);
  digitalWrite(LR, LOW);
}

void stepForwardOnce() {
  stepIndex++;
  if (stepIndex > 3) stepIndex = 0;
  applyStep(stepIndex);
}

void setup() {
  // 初始化电机引脚
  pinMode(LL, OUTPUT);
  pinMode(UL, OUTPUT);
  pinMode(UR, OUTPUT);
  pinMode(LR, OUTPUT);

  // 初始化交互引脚
  pinMode(LED_PIN, OUTPUT);
  pinMode(SWITCH_PIN, INPUT_PULLUP); // 仍然使用上拉，开关接GND

  motorOff();
  digitalWrite(LED_PIN, LOW);
}

void loop() {
  if (digitalRead(SWITCH_PIN) == LOW) {
    digitalWrite(LED_PIN, HIGH);

    unsigned long now = micros();
    if (now - lastStepUs >= STEP_INTERVAL_US) {
      lastStepUs = now;
      stepForwardOnce();
    }
  } else {
    digitalWrite(LED_PIN, LOW);
    motorOff();
  }
}