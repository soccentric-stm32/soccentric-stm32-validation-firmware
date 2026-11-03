# HWVAL test catalogue

Generated from the firmware test tables (`tests/test_*.c`)
by `scripts/gen_testdoc.py` (NFR-010, TFW-002). Do not edit
by hand; rerun the generator after changing the tables.

201 test cases.

## ADC (9 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| ADC.001 | resolutions 12/10/8/6 x3 ADCs | S | - | - | - | 3000 ms | ADC-001 |
| ADC.002 | VREFINT/temp/VBAT internals | S | - | - | - | 2000 ms | ADC-002, PWR-001 |
| ADC.003 | external channels self-driven | S | - | - | - | 5000 ms | ADC-003, GPIO-008 |
| ADC.004 | DAC->ADC linearity (Level L) | L | - | - | - | 300 ms | ADC-004 |
| ADC.005 | sequences/injected/dual-sim | S | - | - | - | 3000 ms | ADC-005, ADC-009 |
| ADC.006 | timer TRGO trigger | S | - | - | - | 3000 ms | ADC-006 |
| ADC.007 | AWD + overrun | S | - | - | - | 2000 ms | ADC-007 |
| ADC.008 | DMA, noise, rate | S | - | - | - | 5000 ms | ADC-008, ADC-010, ADC-011 |
| ADC.012 | EOC/JEOC/OVR interrupts (ADC3) | S | - | - | - | 3000 ms | ADC-012 |

## BRD (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| BRD.001 | jumper/bridge + power checks | S | - | - | - | 200 ms | BRD-001, BRD-002 |

## BTN (3 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| BTN.001 | B1 idle/press/release | S | op | - | - | 20000 ms | BTN-001 |
| BTN.002 | B1 EXTI 5 presses | S | op | - | - | 20000 ms | BTN-002 |
| BTN.003 | B2 (reset) -> RST.003 | S | - | - | - | 300 ms | BTN-003 |

## CACHE (4 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| CACHE.001 | I-cache on/off/invalidate | S | - | - | - | 500 ms | CACHE-001 |
| CACHE.002 | D-cache maintenance vs DMA writes | S | - | - | - | 500 ms | CACHE-002, GEN-007 |
| CACHE.003 | write attribute coherence | S | - | - | - | 500 ms | CACHE-003 |
| CACHE.004 | TCM access + DMA into DTCM | S | - | - | - | 1000 ms | CACHE-004, DMA-005 |

## CAN (3 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| CAN.001 | loopback 4 baud rates x3 | S | - | - | - | 10000 ms | CAN-001 |
| CAN.002 | mailboxes + filters | S | - | - | - | 5000 ms | CAN-002, CAN-003 |
| CAN.004 | errors/bus-off/wakeup/bus (wired/ext) | L | - | - | - | 300 ms | CAN-004, CAN-005, CAN-006, CAN-007, CAN-008, CAN-009 |

## CEC (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| CEC.001 | HDMI-CEC loopback (wired) | L | - | - | - | 300 ms | CEC-001, CEC-002 |

## CLK (13 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| CLK.001 | HSI 16 MHz vs LSE | S | - | - | - | 8000 ms | CLK-001 |
| CLK.002 | HSE bypass 8 MHz vs LSE | S | - | - | - | 8000 ms | CLK-002 |
| CLK.003 | HSE crystal mode (X3) | S | - | - | - | 300 ms | CLK-003 |
| CLK.004 | LSE + drive levels | S | - | - | - | 8000 ms | CLK-004 |
| CLK.005 | LSI via TIM5_CH4 capture | S | - | - | - | 3000 ms | CLK-005 |
| CLK.006 | main PLL 96/144/216 vs LSE | S | - | - | - | 15000 ms | CLK-006 |
| CLK.007 | PLLQ 48 MHz + CLK48 mux | S | - | - | - | 300 ms | CLK-007 |
| CLK.008 | PLLI2S + SAI clock lock | S | - | - | - | 3000 ms | CLK-008, CLK-009 |
| CLK.010 | MCO1/MCO2 configs (+wire/scope) | S | - | - | - | 1000 ms | CLK-010 |
| CLK.011 | CSS config, NMI path | S | - | - | - | 1000 ms | CLK-011 |
| CLK.012 | LSE CSS (not on F767) | S | - | - | - | 300 ms | CLK-012 |
| CLK.013 | APB prescaler sweep + restore | S | - | - | - | 3000 ms | CLK-013 |
| CLK.015 | RCC_CIR ready interrupt | S | - | - | - | 1000 ms | CLK-015, CLK-014 |

## CORE (12 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| CORE.001 | integer ALU/DSP/CLZ | S | - | - | - | 500 ms | CORE-001 |
| CORE.002 | FPU SP+DP incl denorm/inf/nan | S | - | - | - | 500 ms | CORE-002 |
| CORE.003 | divide-by-zero UsageFault traps | S | - | - | - | 500 ms | CORE-003 |
| CORE.004 | unaligned access trapping | S | - | - | - | 500 ms | CORE-004 |
| CORE.005 | BusFault on unmapped + BFAR | S | - | - | - | 500 ms | CORE-005 |
| CORE.006 | undefined instruction UsageFault | S | - | - | - | 500 ms | CORE-006 |
| CORE.007 | NMI pend + PendSV drain + SVC vector | S | - | - | - | 500 ms | CORE-007 |
| CORE.008 | SysTick rate vs TIM2 | S | - | - | - | 1500 ms | CORE-008 |
| CORE.009 | benchmark cached/uncached/ITCM | S | - | - | - | 2000 ms | CORE-009, CACHE-004 |
| CORE.010 | WFI/WFE wake-up | S | - | - | - | 1000 ms | CORE-010 |
| CORE.011 | LDREX/STREX atomicity | S | - | - | - | 2000 ms | CORE-011 |
| CORE.012 | barriers + self-modifying code | S | - | - | - | 500 ms | CORE-012 |

## CRC (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| CRC.001 | CRC unit vs CPU | S | - | - | - | 1000 ms | CRC-001 |

## DAC (5 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DAC.001 | channels 1/2 formats+buffer | S | - | - | - | 2000 ms | DAC-001, DAC-006 |
| DAC.002 | dual channel registers | S | - | - | - | 1000 ms | DAC-002 |
| DAC.003 | trigger selections | S | - | - | - | 1000 ms | DAC-003 |
| DAC.004 | noise/triangle + amplitude | S | - | - | - | 1000 ms | DAC-004 |
| DAC.005 | DMA sine + underrun | S | - | - | - | 2000 ms | DAC-005 |

## DBG (3 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DBG.001 | DWT + watchpoint + FPB monitor | S | - | - | - | 2000 ms | DBG-001, DBG-002, DBG-003 |
| DBG.004 | ITM/SWO + freeze + low-power debug (Level E parts noted) | S | - | - | - | 1000 ms | DBG-004, DBG-005, DBG-006 |
| DBG.007 | ETM trace | E | - | - | - | 200 ms | DBG-007 |

## DCMI (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DCMI.001 | camera interface (wired) | L | - | - | - | 300 ms | DCMI-001, DCMI-002, DCMI-003 |

## DEV (6 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DEV.001 | DBGMCU_IDCODE DEV_ID/REV_ID | S | - | - | - | 200 ms | DEV-001 |
| DEV.002 | Arm Cortex-M7 CPUID | S | - | - | - | 200 ms | DEV-002 |
| DEV.003 | 2 MB flash size register | S | - | - | - | 200 ms | DEV-003 |
| DEV.004 | LQFP144 package register | S | - | - | - | 200 ms | DEV-004 |
| DEV.005 | 96-bit unique ID decode | S | - | - | - | 200 ms | DEV-005 |
| DEV.006 | factory calibration values | S | - | - | - | 200 ms | DEV-006 |

## DFSDM (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DFSDM.001 | parallel path sync3 | S | - | - | - | 2000 ms | DFSDM-001, DFSDM-002, DFSDM-003, DFSDM-004 |

## DMA (6 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DMA.001 | DMA2 m2m on 8 streams + widths/fifo | S | - | - | - | 5000 ms | DMA-001, DMA-003 |
| DMA.002 | TC/HT/TE/DME interrupts | S | - | - | - | 2000 ms | DMA-002 |
| DMA.004 | circular + double buffer | S | - | - | - | 2000 ms | DMA-004 |
| DMA.005 | memory reachability incl ITCM TE | S | - | - | - | 3000 ms | DMA-005 |
| DMA.006 | m2m throughput MB/s | S | - | - | - | 4000 ms | DMA-006 |
| DMA.007 | two-stream priority arbitration | S | - | - | - | 4000 ms | DMA-007 |

## DMA2D (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| DMA2D.001 | Chrom-ART fill/copy/blend | S | - | - | - | 5000 ms | DMA2D-001, DMA2D-002, DMA2D-003 |

## ETH (6 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| ETH.001 | RMII + PHY ID over MDIO | S | - | - | - | 3000 ms | ETH-001, ETH-002 |
| ETH.003 | PHY register sweep read/write-restore | S | - | - | - | 3000 ms | ETH-003 |
| ETH.004 | loopback/PTP/MMC/PM (frames = Level E) | S | - | - | - | 3000 ms | ETH-004, ETH-005, ETH-009, ETH-010, ETH-011, ETH-012, ETH-013 |
| ETH.006 | auto-negotiation + link status (RJ45) | E | - | - | - | 5000 ms | ETH-006 |
| ETH.007 | LwIP DHCP/ICMP + TCP/UDP echo | E | - | - | - | 10000 ms | ETH-007 |
| ETH.008 | TCP echo throughput (loopback + wire) | E | - | - | - | 10000 ms | ETH-008 |

## FLS (7 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| FLS.001 | CRC unit vs CPU over image | S | - | - | - | 2000 ms | FLS-001 |
| FLS.002 | AXIM vs ITCM alias reads | S | - | - | - | 3000 ms | FLS-002 |
| FLS.003 | wait states per (HCLK,WS) table | S | - | - | - | 5000 ms | FLS-003, GEN-005 |
| FLS.004 | ART/prefetch cycle delta | S | - | DESTRUCTIVE | - | 2000 ms | FLS-004 |
| FLS.005 | sector-11 patterns + timing (destructive) | S | - | DESTRUCTIVE | - | 10000 ms | FLS-005, FLS-008, FLS-009 |
| FLS.006 | program to non-erased refused (destructive) | S | - | DESTRUCTIVE | reset | 2000 ms | FLS-006 |
| FLS.007 | unlock/lock; wrong key -> reset | S | - | - | reset | 3000 ms | FLS-007, TFW-007 |

## FMC (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| FMC.001 | NOR/NAND/SDRAM controller regs | S | - | - | - | 2000 ms | FMC-001 |
| FMC.002 | external memory device | E | - | - | - | 300 ms | FMC-002 |

## GPIO (10 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| GPIO.001 | pin table 114 I/O | S | - | - | - | 300 ms | GPIO-001 |
| GPIO.002 | pull-up/pull-down over all free pins | S | - | - | - | 5000 ms | GPIO-002 |
| GPIO.003 | short/follower scan | S | - | - | - | 10000 ms | GPIO-003 |
| GPIO.004 | harness loopback (Level L) | L | - | - | - | 5000 ms | GPIO-004 |
| GPIO.005 | output speeds | L | - | - | - | 500 ms | GPIO-005 |
| GPIO.006 | BSRR/ODR/IDR/LCKR | S | - | - | - | 500 ms | GPIO-006 |
| GPIO.007 | AF0-15 per pin | S | - | - | - | 3000 ms | GPIO-007 |
| GPIO.008 | analog mode | S | - | - | - | 500 ms | GPIO-008 |
| GPIO.009 | ports I/J/K not bonded | S | - | - | - | 300 ms | GPIO-009 |
| GPIO.010 | 5 V tolerance (Level E) | E | - | - | - | 300 ms | GPIO-010 |

## I2C (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| I2C.007 | bus scan x4 (quiet bare board) | S | - | - | - | 20000 ms | I2C-007 |
| I2C.001 | features + transfer gating (wired) | L | - | - | - | 3000 ms | I2C-001, I2C-002, I2C-003, I2C-004, I2C-005, I2C-006, I2C-008 |

## I2S (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| I2S.001 | I2S cfg + standards | S | - | - | - | 2000 ms | I2S-001, I2S-002, I2S-003 |

## INT (8 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| INT.001 | software-pend all external IRQs | S | - | - | - | 20000 ms | INT-001, COV-003 |
| INT.002 | 3-deep nesting + tail-chaining | S | - | - | - | 1000 ms | INT-002 |
| INT.003 | PRIMASK/BASEPRI/FAULTMASK | S | - | - | - | 1000 ms | INT-003 |
| INT.004 | EXTI lines 0-15, edges | S | - | - | - | 2000 ms | INT-004 |
| INT.005 | internal EXTI lines config | S | - | - | - | 1000 ms | INT-005 |
| INT.006 | EXTI event WFE wake | S | - | - | - | 500 ms | INT-006 |
| INT.007 | SYSCFG: EXTICR/comp cell/FMP/ClassB | S | - | - | - | 1000 ms | INT-007 |
| INT.008 | edge latency via H34 wire | L | - | - | - | 500 ms | INT-008 |

## JPEG (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| JPEG.001 | JPEG codec enable path | S | - | - | - | 1000 ms | JPEG-001, JPEG-002 |

## LED (3 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| LED.001 | LD1/LD2/LD3 colour confirm | S | op | - | - | 20000 ms | LED-001, LED-004 |
| LED.002 | PWM brightness ramp | S | op | - | - | 20000 ms | LED-002 |
| LED.003 | IDR short check | S | op | - | - | 500 ms | LED-003 |

## LPTIM (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| LPTIM.001 | LPTIM1 clocks + compare/ARR | S | - | - | - | 3000 ms | LPTIM-001, LPTIM-002 |

## LTDC (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| LTDC.001 | LCD-TFT controller w/o display | S | - | - | - | 3000 ms | LTDC-001, LTDC-002, LTDC-003 |

## MDIOS (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| MDIOS.001 | MDIO slave registers | S | - | - | - | 1000 ms | MDIOS-001 |

## MPU (3 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| MPU.001 | MPU type + unprivileged probe (MPU-004) | S | - | - | - | 500 ms | MPU-001, MPU-004 |
| MPU.002 | RO/XN/no-access fault semantics | S | - | - | - | 500 ms | MPU-002 |
| MPU.003 | subregion disable + PRIVDEFENA | S | - | - | - | 500 ms | MPU-003 |

## OB (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| OB.001 | option bytes decode vs defaults | S | - | - | - | 300 ms | OB-001 |
| OB.002 | option-byte writes not tested | S | - | - | - | 100 ms | OB-002, SAFE-001 |

## OTP (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| OTP.001 | OTP read + lock state | S | - | - | - | 300 ms | OTP-001 |
| OTP.002 | OTP writes not tested | S | - | - | - | 100 ms | OTP-002, SAFE-002 |

## PWR (9 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| PWR.001 | VDDA via VREFINT (PWR-001/002) | S | - | - | - | 1000 ms | PWR-001, PWR-002 |
| PWR.003 | PVD flag + EXTI16 | S | - | - | - | 1000 ms | PWR-003 |
| PWR.004 | voltage scaling + over-drive | S | - | - | - | 1000 ms | PWR-004 |
| PWR.005 | Sleep entry/wake (timer) | S | - | - | - | 2000 ms | PWR-005 |
| PWR.006 | Stop (LPDS) wake by RTC WUT | S | - | - | reset | 10000 ms | PWR-006 |
| PWR.007 | Standby entry (resets MCU) | S | - | - | reset | 5000 ms | PWR-007, TFW-007 |
| PWR.008 | WKUP pins via harness/operators | S | - | - | - | 500 ms | PWR-008 |
| PWR.009 | IDD/current + sources (ext eq) | S | - | DESTRUCTIVE | - | 300 ms | PWR-009, PWR-011, PWR-012 |
| PWR.010 | DBP + BDRST + backup regs (destructive) | S | - | DESTRUCTIVE | - | 5000 ms | PWR-010 |

## QSPI (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| QSPI.001 | indirect read timeout no-device | S | - | - | - | 3000 ms | QSPI-001 |
| QSPI.002 | NOR module (Level E) | E | - | - | - | 300 ms | QSPI-002, QSPI-003 |

## RAM (5 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| RAM.001..005 | March C- all regions (free: march, live: save/restore) | S | - | - | - | 10000 ms | RAM-001, RAM-002, RAM-003, RAM-004, RAM-005 |
| RAM.006 | data/address bus tests | S | - | - | - | 2000 ms | RAM-006 |
| RAM.007 | 8/16/32/64-bit accesses | S | - | DESTRUCTIVE | - | 1000 ms | RAM-007 |
| RAM.008 | backup SRAM March (destructive) | S | op | DESTRUCTIVE | reset | 1000 ms | RAM-008 |
| RAM.009 | backup SRAM retention (reset now; standby via operator) | S | op | - | reset | 5000 ms | RAM-009, TFW-007 |

## RNG (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| RNG.001 | RNG data + distribution | S | - | - | - | 3000 ms | RNG-001, RNG-002 |

## RST (5 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| RST.001 | reset flags boot snapshot | S | - | - | reset | 300 ms | RST-001, GEN-010 |
| RST.002 | software reset -> SFTRSTF | S | op | - | reset | 2000 ms | RST-002, TFW-007 |
| RST.003 | B2 press / POR replug (operator) | S | op | - | - | 300 ms | RST-003, RST-004, RST-006 |
| RST.005 | low-power reset config | S | - | - | - | 300 ms | RST-005, SAFE-001 |
| RST.007 | peripheral reset bits | S | - | - | reset | 2000 ms | RST-007, CLK-014 |

## RTC (10 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| RTC.001 | 10 s advance vs HSE timer | S | - | DESTRUCTIVE | - | 15000 ms | RTC-001, CLK-004 |
| RTC.002 | LSI/HSE/LSE clock sources (destructive) | S | - | DESTRUCTIVE | - | 20000 ms | RTC-002 |
| RTC.003 | calendar roll-overs | S | - | - | - | 20000 ms | RTC-003 |
| RTC.004 | 12 h + 24 h + BCD formats | S | - | - | - | 3000 ms | RTC-004 |
| RTC.005 | alarms A/B with masks | S | - | - | - | 15000 ms | RTC-005 |
| RTC.006 | wake-up timer clockings | S | - | - | - | 20000 ms | RTC-006 |
| RTC.007 | smooth calibration shift | S | op | - | - | 30000 ms | RTC-007 |
| RTC.008 | timestamp + tampers via B1 (operator) | S | op | - | - | 300 ms | RTC-008, RTC-009 |
| RTC.010 | 32 backup registers | S | - | - | - | 2000 ms | RTC-010, PWR-010 |
| RTC.011 | shift + refin source | S | - | - | - | 10000 ms | RTC-011 |

## RTOS (8 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| RTOS.020 | task create/suspend/resume/prio/delete | S | - | - | - | 1500 ms | FRT-020 |
| RTOS.021 | queues, semaphores, events, notify, buffers | S | - | - | - | 3000 ms | FRT-021 |
| RTOS.021.FPU | lazy FPU context across 3 tasks | S | - | - | - | 2000 ms | FRT-009 |
| RTOS.022 | software timer period accuracy | S | - | - | - | 2000 ms | FRT-022 |
| RTOS.023 | 1 kHz tick vs TIM2 +-0.1 % | S | - | - | - | 2000 ms | FRT-023, FRT-002 |
| RTOS.024 | IRQ latency + context switch in cycles | S | - | - | - | 2000 ms | FRT-024 |
| RTOS.025 | task list, runtime stats, stack headroom | S | - | DESTRUCTIVE | - | 1000 ms | FRT-025 |
| RTOS.026 | overflow/malloc hooks (resets MCU) | S | - | DESTRUCTIVE | reset | 1000 ms | FRT-026, FRT-004, FRT-005 |

## SAFE (6 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| SAFE.001 | option bytes untouched | S | - | - | - | 300 ms | SAFE-001, SAFE-003 |
| SAFE.002 | OTP write blocked by MPU | S | - | - | - | 300 ms | SAFE-002 |
| SAFE.004 | flash wrapper sector-11 only | S | - | - | - | 300 ms | SAFE-004 |
| SAFE.005 | destructive run gating | S | - | - | - | 1000 ms | SAFE-005 |
| SAFE.006 | pin ownership registry | S | - | - | - | 300 ms | SAFE-006 |
| SAFE.007 | clock/low-power/confirm paths | S | - | - | - | 300 ms | SAFE-007, SAFE-008, SAFE-009, SAFE-010 |

## SAI (5 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| SAI.001 | master TX -> sync slave RX (protocols/sizes/TDM) | S | - | - | - | 30000 ms | SAI-001 |
| SAI.002 | SAI1<->SAI2 external sync + wired loop | L | - | - | - | 5000 ms | SAI-002 |
| SAI.003 | AC'97 framing (wired) | L | - | - | - | 5000 ms | SAI-003 |
| SAI.005 | companding/mute/FIFO IRQ/DMA | L | - | - | - | 30000 ms | SAI-005 |
| SAI.006 | MCLK per kernel-clock source | L | - | - | - | 5000 ms | SAI-006 |

## SD (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| SD.001 | SDMMC1 register/path state | S | - | - | - | 3000 ms | SD-001 |
| SD.002 | card + FatFs (Level E) | E | - | - | - | 300 ms | SD-002, SD-003, SD-004, SD-005, SD-006, SD-007 |

## SPDIF (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| SPDIF.001 | SPDIFRX via SAI SPDIF-TX (H27b) | L | - | - | - | 30000 ms | SPDIF-001, SAI-004 |
| SPDIF.002 | error flags on stream stop (H27b) | L | - | - | - | 30000 ms | SPDIF-002 |

## SPI (3 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| SPI.001 | loopback masters (wired) | L | - | - | - | 20000 ms | SPI-001 |
| SPI.002 | master-slave/CRC/TI/DMA (wired) | L | - | - | - | 1000 ms | SPI-002, SPI-003, SPI-004, SPI-005, SPI-006 |
| SPI.007 | SPI3/6 TX-only (no loop wire) | S | - | - | - | 2000 ms | SPI-007 |

## STLK (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| STLK.001 | ST-LINK host-side items | S | - | - | - | 200 ms | STLK-001, STLK-002, STLK-003, STLK-004, STLK-005 |

## TIM (9 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| TIM.001 | TIM1-14 time bases (rate window) | S | - | - | - | 15000 ms | TIM-001 |
| TIM.002 | TIM2/TIM5 32-bit counting | S | - | - | - | 3000 ms | TIM-002 |
| TIM.003 | CC channels modes via flags | S | - | - | - | 5000 ms | TIM-003 |
| TIM.004L | capture/encoder/deadtime/break/hall/ETR wiring | L | - | - | - | 300 ms | TIM-004, TIM-005, TIM-007, TIM-008, TIM-010, TIM-014 |
| TIM.006 | one-pulse + repetition | S | - | - | - | 3000 ms | TIM-006 |
| TIM.009 | sync/reset/master-slave | S | - | - | - | 3000 ms | TIM-009 |
| TIM.011 | DCR/DMAR burst addressing | S | - | - | - | 1000 ms | TIM-011 |
| TIM.012 | basic timers (TIM7; TIM6=HAL) | S | - | - | - | 3000 ms | TIM-012, GEN-012 |
| TIM.013 | special remap inputs | S | - | - | - | 1000 ms | TIM-013 |

## UART (6 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| UART.001 | 7 instances loopback (wired) | L | - | - | - | 20000 ms | UART-001, UART-003 |
| UART.002 | formats + special modes | S | - | - | - | 2000 ms | UART-002, UART-003, UART-009 |
| UART.004 | USART3 via CLI (VCP) | S | - | - | - | 200 ms | UART-004 |
| UART.005 | flow control + errors + DMA + wakeup (wired) | L | - | - | - | 300 ms | UART-005, UART-006, UART-007, UART-008, UART-010 |
| UART.009c | special mode registers | S | - | - | - | 1000 ms | UART-009 |
| UART.010c | wakeup config path | S | - | - | - | 300 ms | UART-010 |

## USB (2 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| USB.001 | OTG FS core + VBUS/OC/ID | S | - | - | - | 3000 ms | USB-001, USB-020, USB-022, USB-023 |
| USB.010 | CDC/MSC/HID/host/device (Level E) | E | - | - | - | 300 ms | USB-010, USB-011, USB-012, USB-013, USB-021, USB-024, USB-025 |

## USBHS (1 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| USBHS.001 | OTG HS core + ULPI N/A | S | - | - | - | 3000 ms | USBHS-001, USBHS-002, USBHS-003 |

## WDG (4 cases)

| ID | title | level | operator | destructive | resets | timeout | verifies |
|---|---|---|---|---|---|---|---|
| WDG.001 | IWDG 1 s expiry -> reset (measured vs RTC) | S | - | - | reset | 3000 ms | WDG-001 |
| WDG.002 | IWDG window violation -> reset | S | - | - | reset | 3000 ms | WDG-002 |
| WDG.003 | WWDG EWI + out-of-window reset | S | - | - | reset | 3000 ms | WDG-003, TFW-007 |
| WDG.004 | DBGMCU freeze bits (Level E) | S | - | - | - | 300 ms | WDG-004 |
