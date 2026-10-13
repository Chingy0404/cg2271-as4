# CG2271 Assignment 4 — IoT Mini Project

> **Problem statement:** TODO — 1–2 sentences on what the system does and for whom.

FreeRTOS on the FRDM-MCXC444 + ESP32 with cloud connectivity. Brief: [`docs/CG2271_As4_Brief.pdf`](docs/CG2271_As4_Brief.pdf).

## Deadlines

| What | Due | Marks | Files |
|---|---|---|---|
| Interim report + code | **Fri 23 Oct 2026, 23:59** (Wk 10) | 5 | `B0X_YY_Interim.pdf`, `B0X_YY_Interim_Code.zip` |
| Demo (5–7 min) | Week 13 lab | 35 | — |
| Final report + code | **Fri 13 Nov 2026, 23:59** (Wk 13) | 10 | `B0X_YY_Final.pdf`, `B0X_YY_Code.zip` |

Reports: font size 12, 1.15 spacing, justified. Interim = 2 pages, Final = 5–10 pages. Templates are in [`docs/reports/templates/`](docs/reports/templates/).

## Repo layout

```
common/protocol.h        UART packet format shared by both boards (single source of truth)
esp32/as4_esp32/         Arduino sketch for the ESP32
mcxc444/                 MCUXpresso FreeRTOS project (see mcxc444/README.md)
mcxc444/skeleton/        Reference task/ISR/queue layout to copy from
docs/protocol.md         Message table — update when you change protocol.h
docs/hardware.md         Component list + pin table (becomes the report's pin table)
docs/reports/            Report templates and drafts
scripts/                 sync_protocol.sh, package_submission.sh
```

## Who owns what

| Member | GitHub | Parts |
|---|---|---|
| Yeo Yee Ching | @Chingy0404 | TODO |
| | | |
| | | |
| | | |

## Requirements checklist (from the rubric)

- [ ] 4 sensors + 3 actuators (4/5-member) / 3 + 2 (3-member) — 5 marks
- [ ] ≥1 sensor on MCXC444; ≤1 on-board sensor and ≤1 on-board actuator
- [ ] ≥1 polled sensor and ≥1 interrupt-driven sensor — 2 marks
- [ ] ≥1 ISR; ISR doesn't block or wait for input, uses `FromISR` APIs — 3 marks
- [ ] ≥3 tasks that yield; no obvious bugs — 4 marks
- [ ] ≥1 mutex/semaphore and ≥1 queue — 2 marks
- [ ] `configUSE_PREEMPTION 1` and `configUSE_TIME_SLICING 1`, unused components removed — 2 marks
- [ ] ESP32 used meaningfully, libraries beyond the template — 7 marks
- [ ] Bi-directional MCXC444 ⇄ ESP32 comms; bonus for packetisation / flow control — 5 marks
- [ ] Non-trivial online function; bonus for GenAI / database / web app / MQTT — 5 marks
- [ ] All parts serve one coherent purpose

## Getting started

```bash
git clone https://github.com/Chingy0404/cg2271-as4.git
cd cg2271-as4
cp esp32/as4_esp32/secrets.example.h esp32/as4_esp32/secrets.h   # then fill in your hotspot + keys
```

- **ESP32:** open `esp32/as4_esp32/as4_esp32.ino` in Arduino IDE, install **Arduino_JSON** from Library Manager, select your ESP32 board, upload.
- **MCXC444:** see [`mcxc444/README.md`](mcxc444/README.md).

Read [`CONTRIBUTING.md`](CONTRIBUTING.md) before your first commit.

## Submitting

```bash
./scripts/package_submission.sh interim B01 10   # -> B01_10_Interim_Code.zip
./scripts/package_submission.sh final   B01 10   # -> B01_10_Code.zip
```
Replace `B01 10` with your lab slot and group number.
