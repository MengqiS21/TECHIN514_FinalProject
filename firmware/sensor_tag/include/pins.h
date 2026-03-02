#ifndef SENSOR_PINS_H
#define SENSOR_PINS_H

// LSM6DS3 SDA -> ESP32-C3 GPIO5
static constexpr int PIN_I2C_SDA = 5;

// LSM6DS3 SCL -> ESP32-C3 GPIO6
static constexpr int PIN_I2C_SCL = 6;

// TODO: Optional battery ADC pin. Keep -1 if not used.
static constexpr int PIN_BATTERY_ADC = -1;

#endif
