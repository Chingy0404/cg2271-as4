# MCXC444 ⇄ ESP32 Communication Protocol

Source of truth: [`common/protocol.h`](../common/protocol.h). Update this table whenever you change a message type or payload layout, so the other board's owner knows.

## Physical link

| Setting | Value |
|---|---|
| Interface | UART (TODO: confirm which UART on the MCXC444, e.g. UART2) |
| Baud | 9600 (`AS4_UART_BAUD`) |
| Format | 8N1 |
| MCXC444 TX → ESP32 RX | TODO pin → GPIO16 (`Serial2` RX) |
| MCXC444 RX ← ESP32 TX | TODO pin ← GPIO17 (`Serial2` TX) |
| Ground | **Common GND between both boards** |

## Frame

```
| 0xAA | TYPE | LEN | PAYLOAD (0–16 bytes) | CHECKSUM |
```

`CHECKSUM = TYPE ^ LEN ^ payload[0] ^ … ^ payload[LEN-1]`

The receiver resyncs on the next `0xAA` after any bad frame. Every command from the ESP32 is answered with `MSG_ACK`/`MSG_NACK`, so the sender can retry.

## Message table

| Type | Name | Direction | Payload | Notes |
|---|---|---|---|---|
| 0x01 | `MSG_SENSOR_DATA` | MCX → ESP | TODO (e.g. `[temp_hi, temp_lo, light_hi, light_lo]`) | sent every N ms by the poll task |
| 0x02 | `MSG_EVENT` | MCX → ESP | TODO (e.g. `[event_id]`) | sent when the interrupt-driven sensor fires |
| 0x10 | `MSG_ACTUATOR_CMD` | ESP → MCX | TODO (e.g. `[actuator_id, value]`) | result of the cloud call |
| 0x11 | `MSG_CONFIG` | ESP → MCX | TODO | remote thresholds / modes |
| 0x20 | `MSG_ACK` | both | `[acked_type]` | |
| 0x21 | `MSG_NACK` | both | `[acked_type]` | |
| 0x22 | `MSG_PING` | both | none | link health check |

Multi-byte values are **big-endian** (high byte first).
