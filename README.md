# Smart Autonomous Braking System

An Arduino-based obstacle detection and automatic braking system for small robotic vehicles. The system continuously monitors the distance ahead using an ultrasonic sensor and cuts power to the motors in real time when an obstacle is detected within a critical range — with optional Bluetooth-based manual override.

## Features

- **Real-time obstacle detection** using an HC-SR04 ultrasonic sensor
- **Automatic braking** — motors stop the instant an obstacle is detected within the configured safety distance
- **Configurable safety distance** — critical braking distance is adjustable in code
- **Bluetooth manual control** (optional mode) via HC-05 — forward/backward/left/right/stop commands
- **4-wheel drive control** through an L293D-based motor driver shield
- Written in Embedded C / Arduino C++ (`.ino`)

## Hardware used

| Component | Purpose |
|---|---|
| Arduino Uno | Main microcontroller |
| HC-SR04 Ultrasonic Sensor | Distance measurement for obstacle detection |
| L293D Motor Driver (shield) | Drives the 4 DC motors |
| HC-05 Bluetooth Module | Wireless manual control (optional mode) |
| 4x DC Gear Motors + Wheels | Vehicle drive |
| 7.4V Battery Pack (2x 3.7V Li-ion) | Power supply for motors and logic |

## Circuit / wiring overview

| Signal | Arduino Pin |
|---|---|
| HC-SR04 Trig | A1 |
| HC-SR04 Echo | A0 |
| HC-05 TX/RX | Digital pins (see note below) |
| Motor driver | Connected via L293D shield header |

> **Note:** If using the HC-05 on Arduino Uno's hardware serial (pins 0/1), it will conflict with USB programming and Serial Monitor. It's recommended to either disconnect the HC-05 during upload/debugging, or wire it to different digital pins using `SoftwareSerial`.

## How it works

1. The ultrasonic sensor continuously measures the distance to the nearest object ahead.
2. If the distance is **greater** than the configured safety threshold, the vehicle drives forward.
3. If the distance drops **at or below** the safety threshold, the system immediately stops all motors (`Stop()`), preventing collision.
4. Once the obstacle is no longer in range, the vehicle automatically resumes forward motion.
5. *(Optional mode)* If Bluetooth control is enabled instead, the vehicle responds to manual movement commands sent from a paired phone/app over HC-05.

## Getting started

### Requirements

- [Arduino IDE](https://www.arduino.cc/en/software)
- `AFMotor.h` library (if using an Adafruit Motor Shield–compatible board) — install via Arduino IDE Library Manager
- `Servo.h` (built-in, if using a scanning sensor mount)

### Upload instructions

1. Connect the Arduino Uno via USB
2. Open the `.ino` file in Arduino IDE
3. Select **Tools → Board → Arduino Uno**
4. Select the correct **Tools → Port**
5. Click **Upload**

> If you see a `programmer is not responding` error, disconnect any Bluetooth module wired to pins 0/1 before uploading, then reconnect afterward.

### Configuration

Key parameters can be adjusted at the top of the sketch:

```cpp
#define OBSTACLE_DIST 25   // braking distance threshold in cm
#define Speed 170          // motor speed (0–255)
```

## Testing

A standalone sensor-only test sketch (no motors) is recommended before running the full system, to confirm the ultrasonic sensor is wired correctly and returning clean readings via Serial Monitor at **9600 baud**.

## Future improvements

- Add turn-and-avoid behavior instead of a full stop
- Add speed ramping instead of an instant stop for smoother braking
- Add a buzzer/LED warning indicator when braking is triggered
- Log distance data over time for analysis

## License

This project is open source and available under the [MIT License](LICENSE).
