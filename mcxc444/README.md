# MCXC444 (FRDM board) firmware

## One-time setup (one person does this, then pushes)

1. In MCUXpresso IDE, create a new project for the FRDM-MCXC444 with the **FreeRTOS** component enabled (same way as in the labs).
2. Set the project location to this folder, so you end up with `mcxc444/<ProjectName>/` containing `.project`, `.cproject`, `source/`, `board/`, `freertos/`, etc.
3. In `source/FreeRTOSConfig.h` make sure:
   ```c
   #define configUSE_PREEMPTION    1
   #define configUSE_TIME_SLICING  1
   ```
   and remove components you don't use (graded: "removing any unnecessary components").
4. Run `./scripts/sync_protocol.sh` from the repo root to copy `protocol.h` into `source/`.
5. Copy the structure from `skeleton/main_skeleton.c` into `source/main.c`.
6. Build once, then commit. `.gitignore` already excludes `Debug/`, `Release/` and the workspace `.metadata/`.

## Everyone else

`File → Import → Existing Projects into Workspace` → select `mcxc444/<ProjectName>`. Don't tick "Copy projects into workspace" — you want to edit the files inside the Git repo.

## Avoiding merge conflicts

- Put each sensor/actuator driver in its own file (`source/sensor_light.c/.h`, `source/servo.c/.h`, …) instead of all in `main.c`.
- Only one person edits `FreeRTOSConfig.h` and pin config (`board/pin_mux.c`) at a time — say so in the group chat first.
- Pin assignments go in the table in [`docs/hardware.md`](../docs/hardware.md) before you wire anything.
