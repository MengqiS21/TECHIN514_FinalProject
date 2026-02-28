#ifndef SENSOR_IMU_LSM6DS3_H
#define SENSOR_IMU_LSM6DS3_H

#include <Arduino.h>
#include <Wire.h>
#include "pins.h"

namespace imu {

struct Sample {
  float ax_g;
  float ay_g;
  float az_g;
};

static constexpr uint8_t kRegWhoAmI = 0x0F;
static constexpr uint8_t kRegCtrl1Xl = 0x10;
static constexpr uint8_t kRegOutXL = 0x28;

class Lsm6ds3 {
 public:
  bool begin() {
    if (PIN_I2C_SDA < 0 || PIN_I2C_SCL < 0) {
      return false;
    }

    Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
    Wire.setClock(400000);

    if (!probeAddress(0x6A) && !probeAddress(0x6B)) {
      return false;
    }

    // ODR=416Hz, FS=+/-2g.
    if (!writeReg(kRegCtrl1Xl, 0x60)) {
      return false;
    }
    return true;
  }

  bool readAccel(Sample& out) {
    uint8_t raw[6] = {0};
    if (!readRegs(kRegOutXL, raw, sizeof(raw))) {
      return false;
    }

    const int16_t x = static_cast<int16_t>((raw[1] << 8) | raw[0]);
    const int16_t y = static_cast<int16_t>((raw[3] << 8) | raw[2]);
    const int16_t z = static_cast<int16_t>((raw[5] << 8) | raw[4]);

    // LSM6DS3 +/-2g sensitivity: 0.061 mg/LSB.
    static constexpr float kLsbToG = 0.000061f;
    out.ax_g = x * kLsbToG;
    out.ay_g = y * kLsbToG;
    out.az_g = z * kLsbToG;
    return true;
  }

 private:
  bool probeAddress(uint8_t addr) {
    i2c_addr_ = addr;
    uint8_t whoami = 0;
    if (!readRegs(kRegWhoAmI, &whoami, 1)) {
      return false;
    }
    return whoami == 0x69;
  }

  bool writeReg(uint8_t reg, uint8_t val) {
    Wire.beginTransmission(i2c_addr_);
    Wire.write(reg);
    Wire.write(val);
    return Wire.endTransmission() == 0;
  }

  bool readRegs(uint8_t reg, uint8_t* out, size_t len) {
    Wire.beginTransmission(i2c_addr_);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) {
      return false;
    }
    const size_t got = Wire.requestFrom(static_cast<int>(i2c_addr_), static_cast<int>(len));
    if (got != len) {
      return false;
    }
    for (size_t i = 0; i < len; ++i) {
      out[i] = Wire.read();
    }
    return true;
  }

  uint8_t i2c_addr_ = 0x6A;
};

}  // namespace imu

#endif
