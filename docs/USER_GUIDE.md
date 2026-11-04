# HWVAL user guide (NFR-010)

Operating the HWVAL validation firmware on the NUCLEO-F767ZI.

- Firmware: `v` (see `version` below); spec: SRS-HWVAL-F767.
- Build/flash: `scripts/build.sh` and `scripts/docker-dev.sh flash
  build/hwval.elf` — see [README.md](../README.md) and the
  [docker guide](../docker/README.md).
- The generated list of all test cases lives in
  [TEST_CATALOGUE.md](TEST_CATALOGUE.md) (from the firmware tables).

## 1. Connecting

1. Flash and reset: `scripts/docker-dev.sh flash build/hwval.elf`.
2. Open the ST-LINK VCP at 115200 8N1: `scripts/docker-dev.sh serial`
   (picocom), or `python3 scripts/vcp_capture.py 5` for a capture.
3. The boot banner prints version/git/build date/SRS id, the reset cause
   (and clears the flags), any previous fault record, bootcheck verdicts
   and the watchdog start (GEN-003/010, TFW-007, GEN-009).

The serial line is one-way on boards with the TX-side solder bridge
(SB122/SB123) open (CL-010); flashing/debug still works via SWD. Close
the bridge on the bench to get the console TX direction.

## 2. Interactive console (section 6)

Line editor with backspace, Ctrl-U, Ctrl-C abort of a running test,
10-line history; Ctrl-C at a prompt skips with `NO_OPERATOR`.

| Command | Effect |
|---|---|
| `help [cmd]` | list commands / help for one |
| `version` | firmware + FreeRTOS + SRS versions |
| `info` | DEV_ID/REV_ID, CPUID, UID, OPTCR, clock tree, uptime |
| `list [group\|level]` | test catalogue from the live table |
| `run <sel> [flags]` | run: selector = `all`, group or id; flags = `--level S,L,E`, `--no-operator`, `--destructive`, `--repeat N`, `--until-fail` |
| `abort` | cooperative stop of the running test |
| `results [clear]` | session results (kept in .noinit across resets) |
| `report` | one-line PASS/FAIL/SKIP/N/A summary |
| `harness` | detect the Appendix-A loopback wires |
| `tasks` | FreeRTOS task table + CPU time stats |
| `log error\|warn\|info\|debug` | log level |
| `reset [hard]` | soft reset (hard = IWDG-forced) |
| `mode interactive\|script\|link` | terminal mode (`link` = HWLINK for HWVAL Studio, §6.4) |
| `format text\|json` | result line format |
| `set <k> <v>` / `get <k>` | settings (see `get` with no key) |
| `peek <addr> [n]` / `poke ...` | manual register access (OTP read-only) |
| `gpio <pin> ...` | manual pin control (ownership-checked, SAFE-006) |
| `clock` | clock tree frequencies |
| `baud <rate>` | VCP baud (9600..921600; reverts after 10 s silent) |

Results print as `RESULT <ID> <PASS|FAIL|SKIP|N/A> <ms> <reason>…`
(TFW-010/011/012). A destructive `run` (interactive mode) asks for
confirmation naming what changes (SAFE-005).

## 2a. Link mode (HWLINK, SRS §6.4)

`mode link` prints `OK link` and hands USART3 to the binary HWLINK server
(COBS framing, CRC-32, FlatBuffers).  It implements the HWVAL message set:
`Hello`, `Ping`, `SetBaud` (115200/921600), `Reset`, `GetDeviceInfo`,
`ListGroups`/`ListTests` (paged), `GetHarness` (paged), `Run`, `Abort`,
`OperatorAnswer`, `GetResults` (paged), `ClearResults`, `GetReport`,
`GetSetting`/`SetSetting` and `EnterTextMode`, with `RunStarted`,
`TestStarted`, `TestResult`, `OperatorPrompt`, `RunFinished`, `Progress` and
`Log` events.  `EnterTextMode` (or a text `\r` line after 5 s without a valid
frame, OPEN-011) returns to the text CLI.  The schema is
`protocol/hwlink.fbs`; `src/generated/` holds the flatcc output (regenerated
at build time when a host `flatcc` is available).

If the MCU resets while link mode is active, it boots straight back into link
mode (backup register flag) and resumes a run from the next test after the
one that reset it; the preserved `.noinit` results are available through
`GetResults` (LINK-010).

## 3. Script mode and JSON (used by scripts/hwtest.py)

```
mode script     # no echo, no colour, no prompt (CLI-012)
format json
run all --level S --no-operator
```

Every executed case then emits one JSON object per line:

```json
{"id":"DEV.001","group":"DEV","result":"PASS","duration_ms":1,
 "reason":"","measured":"DEV_ID=411 REV_ID=1001","limits":"…","reqs":"DEV-001"}
```

`scripts/hwtest.py` drives this (HOST-001):
`python3 scripts/hwtest.py --level S --no-operator` → runs, prints the
summary and writes `reports/hwval_<UID8>_<date>.json` + `.md`.
Exit code 0 = all PASS, 1 = any FAIL, 2 = communication error
(HOST-004). Operator prompts (HOST-002) are relayed to the terminal;
`--yes` pre-answers them, `--include-operator` enables the group,
`--destructive` (with `--include-operator` and a printed scope notice)
is the host-side SAFE-005 confirmation.

Host legs of equipment tests (HOST-003): `--cdc-echo <dev>` (USB-010),
`--eth-target auto` (ETH-007/008 ping + TCP/UDP echo on port 7 + MB/s),
`--swo N` (DBG-004 capture via OpenOCD TPIU), `--halt-check`
(WDG-004/DBG-005 freeze-register dump during a 5 s halt).

## 4. Running the suite

```
run all --level S --no-operator          # the < 15 min Level-S suite
run all --level S,L --no-operator        # + harness-wired Level-L
run all --level S,L,E --include-operator  # bench/full
run PWR                                  # one group
run RTC.006 --repeat 5                   # one case, 5 times
```

Defaults without flags are the SAFE-005 boot rules: no operator, no
destructive. Cases with unmet harness/equipment preconditions report
`SKIP NO_HARNESS` / `NO_EQUIPMENT` / `NOT_FITTED` rather than failing
(ENV-002/003). Results survive a reset in .noinit (TFW-022); boot-time
MCU-resetting cases report through the BOOTCHECK banner (TFW-007).

## 5. Harness wiring (Appendix A)

The 43-loopback-wire set is defined in
[`board/hwval_board_pins.c`](../board/hwval_board_pins.c) (`hwval_wire_table`):
each entry names the endpoints (port, pin), the used-by test group and a
`verify` flag for pairs whose AF muxing still needs a bench check
(OPEN-004). Highlights:

- H01/H02: DAC_OUT1/2 → ADC inputs (ADC-004 linearity)
- H03..H09: 7 UART TX→RX loops; H10/H11 RTS→CTS; H12/H12b cross injection
- H13..H16, H19/H19b: SPI loops; H20*: I2C pairs; H21: CAN loop
- H22..H25*: timer capture/encoder/hall; H26*: MCO → capture
- H27/H27b: SAI SD_A → SD_B / SPDIFRX_IN1; H28: DFSDM; H31: DCMI; H33*: WKUP; H34: EXTI; H35: LPTIM

`harness` on the console detects each wire at runtime (one end driven,
the other read, both directions — ENV-002). Wires flagged `verify=1`
are excluded from the detection bitmask until bench-confirmed.

## 6. External equipment (Appendix B)

Only the Level-E cases need equipment; each reports `SKIP NO_EQUIPMENT`
without it: RJ45 switch/host (ETH-006..008, host legs via `hwtest.py`),
second micro-AB USB cable for USB-010/011 (+ flash drive / HIDs for
USB-021), microSD card + adapter (SD-002..007), QSPI NOR module
(QSPI-002/003), CAN transceivers (CAN-008), I2C module (I2C-007), bench
supply (RST-006/PWR-011), ammeter at JP6 (PWR-009), scope (CLK-010,
GPIO-005, LTDC-002). Consult [Hardware.md](../Hardware.md) for board
details and the solder-bridge/jumpers section (BRD-001, OPEN-002/003).

## 7. Safety summary (section 9)

- Option bytes and OTP are never written (SAFE-001/002; OTP mapped
  read-only by the MPU, verified by SAFE.002).
- Destructive flash activity is limited to sector 11 by an address
  check inside the flash wrapper (SAFE-004, verified by SAFE.004).
- Destructive runs need the explicit `--destructive` selection + printed
  confirmation (SAFE-005).
- Pins owned by another circuit are not driven (SAFE-006 registry).
- Clock tests switch to HSI before reconfiguring the PLL (SAFE-007) and
  stay inside the [DS11532] tables.
- Low-power tests keep a wake-up source armed and the DBG_SLEEP/STOP/
  STANDBY bits set so a debugger can connect after a stuck wake-up
  (SAFE-008).
- SD/USB writes never reformat or write outside the agreed block range
  without confirmation (SAFE-010).

## 8. Interpreting reports

- `FAIL` rows: read `reason` (what deviated) and `measured/limits`
  (the numbers). Re-run the single case with `run <id>` and, if needed,
  `log debug` before re-running.
- `SKIP NO_HARNESS`/`NO_EQUIPMENT`: wire/equipment missing — not a
  board defect. `N/A NOT_FITTED`/`NOT_BONDED`: the feature does not
  exist on this board (documented in SRS §12).
- The JSON report is the machine-readable record
  (`reports/*.json`); the Markdown one is for the bench file.