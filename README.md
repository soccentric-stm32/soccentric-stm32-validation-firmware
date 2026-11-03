# HWVAL — NUCLEO-F767ZI hardware validation firmware

Firmware specified by [SRS.md](SRS.md): a FreeRTOS application that tests
every usable hardware block of the NUCLEO-F767ZI board and the STM32F767ZI
MCU on it, operated from a terminal over the ST-LINK virtual COM port.

| | |
|---|---|
| Status | Implemented per SRS section (see commit history, §-references) |
| Complication log | [COMPLICATIONS.md](COMPLICATIONS.md) |
| Board reference | [Hardware.md](Hardware.md) (measured + reference) |
| Build environment | [docker/README.md](docker/README.md), STM32CubeF7 v1.17.4 |

## Build (CON-001)

Everything builds inside the `stm32-dev` container (HAL/CMSIS/FreeRTOS come
from the image; the project never modifies SDK code, CON-002):

```bash
scripts/build.sh            # cmake configure + ninja build in the container
scripts/build.sh clean      # from scratch
```

Outputs in `build/`: `hwval.elf`, `hwval.bin`, `hwval.hex`, `hwval.map`
(GEN-001). Linking fails if the image overlaps flash sector 11 (GEN-006,
ASM-004: sector 11 is reserved for the destructive flash tests).

## Flash and run (GEN-002)

```bash
scripts/docker-dev.sh flash build/hwval.elf
scripts/docker-dev.sh reset                 # boots without a debugger
scripts/docker-dev.sh serial                # 115200 8N1 on /dev/ttyACM0
```

Boot banner (GEN-003): version, git commit, build date, SRS id. Reset cause
printed and cleared on every boot (GEN-010). If `scripts/vcp_capture.py` is
preferred over an interactive console: `python3 scripts/vcp_capture.py 5`.

## Firmware layout (NFR-008/NFR-009)

```
board/            board-specific pins/lvalues, HAL config (stm32f7xx_hal_conf.h)
ld/               linker script: sectors 0-10 only, .noinit, .noncacheable
src/              platform code, one file per concern (clock, faults, ...)
tests/            one file per hardware group (arrives with SRS section 8)
```

## HAL time base (GEN-012)

**TIM6** (basic timer) is the HAL time base at a fixed 1 kHz; SysTick stays
owned by FreeRTOS. TIM7 remains free for DAC triggering (DAC-003). TIM6 is
excluded from destructive timer tests while in use — see the note in the
TIM test group when section 8.17 lands.