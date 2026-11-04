# HWVAL — NUCLEO-F767ZI hardware validation firmware

Firmware specified by [SRS.md](SRS.md): a FreeRTOS application that tests
every usable hardware block of the NUCLEO-F767ZI board and the STM32F767ZI
MCU on it, operated from a terminal over the ST-LINK virtual COM port.

| | |
|---|---|
| Status | **Feature-complete**: SRS §4–§12 + appendices implemented, including HWLINK link mode (§6.4); bench validation pending |
| Complication log | [COMPLICATIONS.md](COMPLICATIONS.md) |
| Board reference | [Hardware.md](Hardware.md) (measured + reference) |
| Build environment | [docker/README.md](docker/README.md), STM32CubeF7 v1.17.4 (FreeRTOS V10.2.0 as shipped, CL-011), LwIP 2.1.2 (as shipped) |
| User guide | [docs/USER_GUIDE.md](docs/USER_GUIDE.md) (NFR-010) |
| Test catalogue | [docs/TEST_CATALOGUE.md](docs/TEST_CATALOGUE.md) — generated from the firmware tables (TFW-002) |

## Build (CON-001)

Everything builds inside the `stm32-dev` container (HAL/CMSIS/FreeRTOS/LwIP
come from the image; the project never modifies SDK code, CON-002):

```bash
scripts/build.sh            # cmake configure + ninja build (in the container)
scripts/build.sh clean      # from scratch
```

The build then runs the gates (non-zero exit on failure):

- `scripts/check_footprint.py` — NFR-003 (image ≤ 1792 KB, sectors 0–10) and
  NFR-004 (≥ 256 KB SRAM1 and ≥ 64 KB DTCM free), measured from `hwval.map`.
- `scripts/check_coverage.py` — COV-001 (every §8 requirement has a test
  case; every case lists requirements) and COV-002 (every `*_BASE` of the
  CMSIS header appears in the §12 matrix).
- `scripts/cppcheck.sh strict` — static analysis (NFR-007); the compile
  itself runs with `-Wall -Wextra -Werror`.

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

## Host tooling (section 11)

`scripts/hwtest.py` (HOST-001..005): switches the firmware to script mode +
JSON, runs a selection, and writes `reports/hwval_<UID8>_<date>.json`/`.md`.
HWVAL Studio uses the same VCP in **link mode** (`mode link`, HWLINK COBS +
FlatBuffers, SRS §6.4); the schema is `protocol/hwlink.fbs` and the FlatBuffers
runtime is vendored in `third_party/flatcc` (generated files in
`src/generated/`).  `hwtest.py` keeps using the text CLI.
It also hosts the equipment legs: `--cdc-echo` (USB-010), `--eth-target
auto` (ETH-007/008 ping + TCP/UDP echo + Mbit/s), `--swo N` (DBG-004),
`--halt-check` (WDG-004/DBG-005). Exit codes 0/1/2 (pass/fail/comms).

## Firmware layout (NFR-008/NFR-009)

```
board/            board-specific pins/wires, HAL config, LwIP opts + arch
ld/               linker script: sectors 0-10 only, .noinit, .noncacheable,
                  .sram1 (NFR-004 DMA/link-buffer section)
protocol/         shared HWLINK FlatBuffers schema (hwlink.fbs)
src/              platform code, one file per concern (clock, faults, net,
                  link mode...)
src/generated/    flatcc headers generated from protocol/hwlink.fbs
third_party/      vendored flatcc 0.6.1 runtime (CON-002)
tests/            one file per hardware group (SRS section 8)
docs/             user guide + generated test catalogue
```

## HAL time base (GEN-012)

**TIM6** (basic timer) is the HAL time base at a fixed 1 kHz; SysTick stays
owned by FreeRTOS. TIM7 remains free for DAC triggering (DAC-003). TIM6 is
excluded from destructive timer tests while in use — enforced in the TIM
test group (TIM.012, GEN-012).

## Bench validation plan (next step)

The firmware is complete but awaits the bench pass (not yet run): Level-S
suite (`run all --level S --no-operator`), then harness-wired L and
equipment E cases, exactly as staged in [docs/USER_GUIDE.md](docs/USER_GUIDE.md)
§4–§8. Open bench items are Appendix D OPEN-002/003/005.

## License

[MIT](LICENSE) © 2026 Sandesh Ghimire. Use it as a starting point for your own
products. Third-party code (STM32CubeF7, FreeRTOS, LwIP, Embassy, Zephyr and
others) is fetched at build time and keeps its own licence.
