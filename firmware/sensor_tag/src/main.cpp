#include <Arduino.h>
#include <BLEDevice.h>
#include <esp_sleep.h>
#include <math.h>

#include "activity_algo.h"
#include "ble_protocol.h"
#include "config.h"
#include "imu_lsm6ds3.h"
#include "pins.h"
#include "power_stages.h"

enum class SensorState {
  BOOT,
  IMU_INIT,
  SENSE_IMU,
  PROCESS,
  BLE_TX,
  RADIO_OFF,
  DEEP_SLEEP,
};

namespace {

class SensorServerCallbacks : public BLEServerCallbacks {
 public:
  void onConnect(BLEServer* /*server*/) override { connected_ = true; }
  void onDisconnect(BLEServer* server) override {
    connected_ = false;
    server->startAdvertising();
  }

  bool isConnected() const { return connected_; }

 private:
  bool connected_ = false;
};

SensorState g_state = SensorState::BOOT;
imu::Lsm6ds3 g_imu;
bool g_has_i2c_pins = false;
bool g_imu_ready = false;
uint32_t g_seq = 0;
uint16_t g_activity = 0;
uint16_t g_battery_mv = 0;

float g_sum_abs_accel = 0.0f;
uint32_t g_sample_count = 0;

bool validateRequiredPins() {
  bool ok = true;
  if (PIN_I2C_SDA < 0) {
    Serial.println("ERROR: PIN_I2C_SDA not set. Edit include/pins.h");
    ok = false;
  }
  if (PIN_I2C_SCL < 0) {
    Serial.println("ERROR: PIN_I2C_SCL not set. Edit include/pins.h");
    ok = false;
  }
  return ok;
}

uint16_t readBatteryMv() {
  if (PIN_BATTERY_ADC < 0) {
    return 0;
  }
  const int raw = analogRead(PIN_BATTERY_ADC);
  // Minimal placeholder conversion for 12-bit ADC @ 3.3V.
  const uint32_t mv = static_cast<uint32_t>(raw) * 3300UL / 4095UL;
  return static_cast<uint16_t>(mv);
}

void sampleImuWindow() {
  LOG_STAGE("IMU_SAMPLING_START");
  g_sum_abs_accel = 0.0f;
  g_sample_count = 0;

  if (!g_imu_ready) {
    Serial.println("IMU not ready; skipping sampling.");
    return;
  }

  const uint32_t start_ms = millis();
  while (millis() - start_ms < kImuSampleWindowMs) {
    imu::Sample s = {0.0f, 0.0f, 0.0f};
    if (g_imu.readAccel(s)) {
      g_sum_abs_accel += fabsf(s.ax_g) + fabsf(s.ay_g) + fabsf(s.az_g);
      ++g_sample_count;
    }
    delay(kImuSamplePeriodMs);
  }

  Serial.print("IMU samples: ");
  Serial.println(g_sample_count);
}

void sendBlePayload() {
  LOG_STAGE("BLE_ON");

  BLEDevice::init(kBleDeviceName);
  BLEServer* server = BLEDevice::createServer();
  auto* callbacks = new SensorServerCallbacks();
  server->setCallbacks(callbacks);

  BLEService* service = server->createService(BLEUUID(BLE_SERVICE_UUID));
  BLECharacteristic* ch = service->createCharacteristic(
      BLEUUID(BLE_CHAR_UUID),
      BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);

  service->start();

  BLEAdvertising* adv = BLEDevice::getAdvertising();
  adv->addServiceUUID(BLEUUID(BLE_SERVICE_UUID));
  adv->start();

  const uint32_t start_wait = millis();
  while (!callbacks->isConnected() && (millis() - start_wait < kBleConnectTimeoutMs)) {
    delay(20);
  }

  if (callbacks->isConnected()) {
    ActivityPayload payload{};
    payload.seq = g_seq;
    payload.activity = g_activity;
    payload.battery_mv = g_battery_mv;

    LOG_STAGE("BLE_SEND");
    ch->setValue(reinterpret_cast<const uint8_t*>(&payload), sizeof(payload));
    ch->notify();
    delay(kBlePostNotifyDelayMs);
  } else {
    Serial.println("BLE: no central connected before timeout.");
  }

  adv->stop();
  LOG_STAGE("BLE_OFF");
  BLEDevice::deinit(true);
}

void enterDeepSleep() {
  LOG_STAGE("DEEP_SLEEP");
  esp_sleep_enable_timer_wakeup(static_cast<uint64_t>(kDeepSleepSeconds) * 1000000ULL);
  Serial.flush();
  delay(50);
  esp_deep_sleep_start();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
}

void loop() {
  switch (g_state) {
    case SensorState::BOOT:
      LOG_STAGE("IDLE");
      g_has_i2c_pins = validateRequiredPins();
      g_state = SensorState::IMU_INIT;
      break;

    case SensorState::IMU_INIT: {
      g_imu_ready = false;
      if (!g_has_i2c_pins) {
        Serial.println("I2C pins missing; IMU init skipped.");
        g_state = SensorState::SENSE_IMU;
        break;
      }

      for (uint8_t i = 0; i < kImuInitRetries; ++i) {
        if (g_imu.begin()) {
          g_imu_ready = true;
          break;
        }
        Serial.print("IMU init failed, retry ");
        Serial.println(i + 1);
        delay(100);
      }

      if (!g_imu_ready) {
        Serial.println("IMU init failed after retries; using activity=0.");
      }
      g_state = SensorState::SENSE_IMU;
      break;
    }

    case SensorState::SENSE_IMU:
      sampleImuWindow();
      g_state = SensorState::PROCESS;
      break;

    case SensorState::PROCESS: {
      if (g_sample_count > 0) {
        const float avg = g_sum_abs_accel / static_cast<float>(g_sample_count);
        g_activity = activity::computeActivityFromAverage(avg);
      } else {
        g_activity = 0;
      }
      g_battery_mv = readBatteryMv();
      Serial.print("Activity: ");
      Serial.println(g_activity);
      g_state = SensorState::BLE_TX;
      break;
    }

    case SensorState::BLE_TX:
      sendBlePayload();
      g_state = SensorState::RADIO_OFF;
      break;

    case SensorState::RADIO_OFF:
      ++g_seq;
      g_state = SensorState::DEEP_SLEEP;
      break;

    case SensorState::DEEP_SLEEP:
      enterDeepSleep();
      break;
  }
}
