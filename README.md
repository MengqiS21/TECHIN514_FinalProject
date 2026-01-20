# Pet Activity & Proximity Tracker

A wearable sensing tag for pets and a tabletop display that visualizes daily activity and proximity-based companionship.  
The system is designed as a calm, ambient tool that supports reflection rather than real-time alerts, helping pet owners understand patterns of activity and togetherness over the course of a day. By translating sensed data into a physical gauge, the project emphasizes presence and routine over precise tracking or notifications.

## 1. Overview

### What it does
This project consists of a lightweight sensing device attached to a pet’s collar and a tabletop display device placed in the home. The system tracks the pet’s activity level and estimates proximity to the owner throughout the day, visualizing overall “bond time” using a physical gauge instead of a screen.

### General system sketch
![Overall system sketch](images/Overview.png)


## 2. Sensing Device (Pet Collar Tag)

### Description
The sensing device is a lightweight tag mounted on a pet’s collar that detects motion and estimates proximity to the owner. It classifies activity levels (resting, walking, high activity) and tracks how much time the pet spends near the display device without using GPS.

### Detailed sensing device sketch
![Sensing device sketch](images/Sensor_device.png)


### How it works
- An onboard accelerometer detects motion patterns to estimate activity intensity.
- Bluetooth RSSI values are sampled to estimate proximity to the display device.
- Lightweight DSP runs on the sensing device to smooth accelerometer data and RSSI readings, classifying activity level and proximity into stable bins.
- Summarized data is transmitted periodically to the display device via BLE.

### Sensors and components (with part numbers)
- **Accelerometer:** ADXL345 (Analog Devices, 3-axis digital accelerometer)
- **Microcontroller + BLE:** ESP32-C3-MINI-1
- **Battery:** CR2032 coin cell *or* 150–300 mAh LiPo battery
- **Power management:** MCP1700 low-dropout regulator
- **Optional proximity aid:** Onboard ESP32 BLE RSSI (no extra sensor required)


## 3. Display Device (Bond Meter)

### Description
The display device is a tabletop “bond meter” that physically visualizes the pet’s daily activity and proximity time using a stepper-motor-driven gauge needle. An LED provides real-time feedback when the pet is nearby, and a button allows users to switch display modes or acknowledge the status.

### Detailed display device sketch
![Display device sketch](images/Display_device.png)

### How it works
- The display receives summarized activity and proximity data via BLE.
- The microcontroller maps daily totals to a gauge needle position using a stepper motor.
- The LED indicates current proximity state (e.g., pet nearby vs away).
- The button toggles display modes (activity vs proximity) or resets daily tracking.

### Components (with part numbers)
- **Microcontroller + BLE:** ESP32-C3-MINI-1
- **Stepper motor:** 28BYJ-48 stepper motor
- **Motor driver:** ULN2003 stepper driver
- **LED:** WS2812B single RGB LED *or* standard 5mm LED
- **Button:** Standard tact switch
- **Battery:** 500–1200 mAh LiPo battery


## 4. System Communication & Data Flow

### Wireless communication overview
![Communication overview diagram](images/Communication_overview.png)

### Detailed system diagram
![Detailed data flow diagram](images/Process_diagram.png)


## Notes on Power and Feasibility
Both devices are battery-powered and optimized for low-duty-cycle operation. The sensing device minimizes power usage through aggressive sleep scheduling, while the display device uses short stepper motor movements and infrequent updates to maintain reasonable battery life.