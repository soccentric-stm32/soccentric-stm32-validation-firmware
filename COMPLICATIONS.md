# Complication Log — HWVAL (SRS-HWVAL-F767)

Running list of complications encountered while implementing the SRS,
section by section. One commit/push per SRS section, newest entries last.

| ID | Section | Complication | Discovered | Status / resolution |
|---|---|---|---|---|
| CL-001 | — | Repository had no git remote; SRS.md itself was untracked. | baseline | Created private repo `sandeshghimire/hwval-f767` via `gh`, added as `origin`, baseline commit includes SRS.md + this log. |
| CL-002 | §4 | `stm32-dev:latest` docker image not prebuilt; STM32CubeF7 v1.17.4 exists only inside it (CON-002 forbids local SDK copies). Builds must run in the container. | baseline | `scripts/docker-dev.sh build` started in background before §4 work; ~1.1 GB clone from GitHub required. |
| CL-003 | §4 | Host `arm-none-eabi-gcc` is the same 13.2.rel1 as the image, but no local SDK → cannot compile-check outside docker; every build iteration pays container startup. | baseline | Mitigation: single long-lived helper (`scripts/build.sh`, see §4 commit) and batched file writes. |
| CL-004 | §8 | Most of §8 is Level L (harness Appendix A), Level E (external equipment) or operator (O). The bench has the board but not the harness/equipment, so all hardware test code must implement ENV-002/ENV-003 gating: detect harness/equipment at runtime, else SKIP (`NO_HARNESS`/`NO_OPERATOR`/`NOT_FITTED`). | baseline | Test framework and every test group carry level/operator/destructive flags and preconditions from day one. |
| CL-005 | App. A | SRS Appendix A leaves the harness pin pairs to the design (OPEN-001). Pin pairs cannot stay undefined without blocking GPIO-004 and all L-level tests. | baseline | Pin pairs fixed in `board/hwval_board_pins.h` during §8.13 (GPIO), then SRS Appendix A filled and OPEN-001 closed in the appendices pass. |
| CL-006 | §4 | GEN-002 (flash + boot without debugger) and similar hardware-in-the-loop checks need exclusive ST-LINK access; anything holding OpenOCD (gdbserver) blocks flashing. | baseline | Rule: always flash/reset through `scripts/docker-dev.sh flash|reset`, never leave gdbserver running between sections. |