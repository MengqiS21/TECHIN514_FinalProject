#include <Arduino.h>
#include <BLEDevice.h>
#include <cstring>

#include "ble_protocol.h"
#include "config.h"
#include "led_status.h"
#include "motor_gauge.h"
#include "pins.h"
#include "power_stages.h"

enum class DisplayState {
  BOOT,
  BLE_SCAN_CONNECT,
  WAIT_FOR_DATA,
  UPDATE_DISPLAY,
  IDLE,
};

namespace {

class DisplayClientCallbacks : public BLEClientCallbacks {
 public:
  void onConnect(BLEClient* /*client*/) override {}

  void onDisconnect(BLEClient* /*client*/) override {
    Serial.println("BLE disconnected.");
    connected_ = false;
  }

  bool connected() const { return connected_; }
  void setConnected(bool value) { connected_ = value; }

 private:
  bool connected_ = false;
};

DisplayState g_state = DisplayState::BOOT;
DisplayClientCallbacks g_client_callbacks;
BLEClient* g_client = nullptr;
BLERemoteCharacteristic* g_remote_char = nullptr;

ActivityPayload g_last_payload{};
volatile bool g_payload_ready = false;
uint32_t g_wait_start_ms = 0;

bool g_motor_ready = false;
bool g_ble_initialized = false;

void notifyCallback(BLERemoteCharacteristic* /*remote*/, uint8_t* data, size_t len, bool /*isNotify*/) {
  if (len < sizeof(ActivityPayload)) {
    return;
  }
  memcpy(&g_last_payload, data, sizeof(ActivityPayload));
  g_payload_ready = true;
}

bool validatePins() {
  bool ok = true;
  if (PIN_MOTOR_IN1 < 0) {
    Serial.println("ERROR: PIN_MOTOR_IN1 not set. Edit include/pins.h");
    ok = false;
  }
  if (PIN_MOTOR_IN2 < 0) {
    Serial.println("ERROR: PIN_MOTOR_IN2 not set. Edit include/pins.h");
    ok = false;
  }
  if (PIN_MOTOR_IN3 < 0) {
    Serial.println("ERROR: PIN_MOTOR_IN3 not set. Edit include/pins.h");
    ok = false;
  }
  if (PIN_MOTOR_IN4 < 0) {
    Serial.println("ERROR: PIN_MOTOR_IN4 not set. Edit include/pins.h");
    ok = false;
  }
  return ok;
}

bool ensureBleInit() {
  if (g_ble_initialized) {
    return true;
  }
  BLEDevice::init(kBleDeviceName);
  g_ble_initialized = true;
  return true;
}

bool connectToSensor() {
  if (!ensureBleInit()) {
    return false;
  }

  BLEScan* scan = BLEDevice::getScan();
  scan->setActiveScan(true);
  BLEScanResults results = scan->start(kBleScanSeconds, false);

  BLEAdvertisedDevice* target = nullptr;
  for (int i = 0; i < results.getCount(); ++i) {
    BLEAdvertisedDevice device = results.getDevice(i);
    if (device.haveServiceUUID() && device.isAdvertisingService(BLEUUID(BLE_SERVICE_UUID))) {
      target = new BLEAdvertisedDevice(device);
      break;
    }
  }

  scan->clearResults();

  if (target == nullptr) {
    Serial.println("BLE scan: target service not found.");
    return false;
  }

  if (g_client == nullptr) {
    g_client = BLEDevice::createClient();
    g_client->setClientCallbacks(&g_client_callbacks);
  }

  if (!g_client->connect(target)) {
    Serial.println("BLE connect failed.");
    delete target;
    return false;
  }
  delete target;

  BLERemoteService* service = g_client->getService(BLEUUID(BLE_SERVICE_UUID));
  if (service == nullptr) {
    Serial.println("BLE service missing on peer.");
    g_client->disconnect();
    return false;
  }

  g_remote_char = service->getCharacteristic(BLEUUID(BLE_CHAR_UUID));
  if (g_remote_char == nullptr) {
    Serial.println("BLE characteristic missing on peer.");
    g_client->disconnect();
    return false;
  }

  if (!g_remote_char->canNotify()) {
    Serial.println("BLE characteristic does not support notify.");
    g_client->disconnect();
    return false;
  }

  if (!g_remote_char->registerForNotify(notifyCallback)) {
    Serial.println("BLE notify subscription failed.");
    g_client->disconnect();
    return false;
  }

  g_client_callbacks.setConnected(true);
  g_wait_start_ms = millis();
  return true;
}

bool isBleConnected() {
  if (g_client == nullptr) {
    return false;
  }
  return g_client->isConnected() && g_client_callbacks.connected();
}

void updateDisplayFromPayload() {
  const uint16_t activity = (g_last_payload.activity > 100) ? 100 : g_last_payload.activity;

  Serial.print("RX seq=");
  Serial.print(g_last_payload.seq);
  Serial.print(" activity=");
  Serial.print(activity);
  Serial.print(" battery_mv=");
  Serial.println(g_last_payload.battery_mv);

  if (g_motor_ready) {
    motor_gauge::setTargetFromActivity(activity);
  } else {
    Serial.println("Motor pins not configured; display update is print-only.");
  }
  led_status::setFromActivity(activity);
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
}

void loop() {
  switch (g_state) {
    case DisplayState::BOOT:
      validatePins();
      g_motor_ready = motor_gauge::init();
      led_status::init();
      g_state = DisplayState::BLE_SCAN_CONNECT;
      break;

    case DisplayState::BLE_SCAN_CONNECT:
      LOG_STAGE("BLE_SCAN");
      if (connectToSensor()) {
        LOG_STAGE("BLE_CONNECTED");
        g_state = DisplayState::WAIT_FOR_DATA;
      } else {
        delay(400);
      }
      break;

    case DisplayState::WAIT_FOR_DATA:
      if (!isBleConnected()) {
        g_state = DisplayState::BLE_SCAN_CONNECT;
        break;
      }

      if (g_payload_ready) {
        g_state = DisplayState::UPDATE_DISPLAY;
        break;
      }

      if (millis() - g_wait_start_ms > kDataWaitTimeoutMs) {
        g_state = DisplayState::IDLE;
      }
      delay(20);
      break;

    case DisplayState::UPDATE_DISPLAY:
      LOG_STAGE("DISPLAY_UPDATE");
      updateDisplayFromPayload();
      g_payload_ready = false;
      g_wait_start_ms = millis();
      g_state = DisplayState::WAIT_FOR_DATA;
      break;

    case DisplayState::IDLE:
      LOG_STAGE("IDLE");
      delay(kIdleDelayMs);
      if (!isBleConnected()) {
        g_state = DisplayState::BLE_SCAN_CONNECT;
      } else {
        g_wait_start_ms = millis();
        g_state = DisplayState::WAIT_FOR_DATA;
      }
      break;
  }
}
