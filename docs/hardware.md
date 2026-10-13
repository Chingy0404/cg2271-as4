# Hardware & pin table

Fill this in **before** wiring, so two people don't claim the same pin. The final report needs a circuit diagram or pin table — this becomes it. Put Fritzing / draw.io exports in `docs/diagrams/`.

## Components

Requirement: 4/5-member group → 4 sensors + 3 actuators (3-member → 3 + 2). At least 1 sensor on the MCXC444; at most 1 on-board sensor and 1 on-board actuator; at least 1 polled and 1 interrupt-driven sensor.

| # | Component | Sensor/Actuator | Board | Polled / Interrupt / Output | Owner | Status |
|---|---|---|---|---|---|---|
| 1 | TODO | Sensor | MCXC444 | Polled | | |
| 2 | TODO | Sensor | MCXC444 | Interrupt | | |
| 3 | TODO | Sensor | ESP32 | | | |
| 4 | TODO | Sensor | | | | |
| 5 | TODO | Actuator | | | | |
| 6 | TODO | Actuator | | | | |
| 7 | TODO | Actuator | | | | |

## MCXC444 pins

| Pin | Function | Connected to | Notes |
|---|---|---|---|
| | UART TX | ESP32 GPIO16 | |
| | UART RX | ESP32 GPIO17 | |
| GND | Ground | ESP32 GND | common ground |

## ESP32 pins

| GPIO | Function | Connected to | Notes |
|---|---|---|---|
| 16 | Serial2 RX | MCXC444 UART TX | |
| 17 | Serial2 TX | MCXC444 UART RX | |
