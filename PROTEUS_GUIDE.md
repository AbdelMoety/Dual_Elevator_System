# Proteus Configuration Guide — STM32F401VE Dual Elevator System

Use two `STM32F401VE` MCUs in Proteus, preferably LQFP100.

## 1) Build the two HEX files

Open the project folder in a terminal with the Arm GNU toolchain available.

Edit `cmake/ArmToolchain.cmake` and set your real toolchain path:

```cmake
set(ARM_DIR  "E:/Third Year (Second Term)/arm-gnu-toolchain-15.2.rel1-mingw-w64-x86_64-arm-none-eabi")
```

Then build:

```bash
cmake -S . -B build-master -GNinja -DELEVATOR_ROLE=MASTER
cmake --build build-master

cmake -S . -B build-slave -GNinja -DELEVATOR_ROLE=SLAVE
cmake --build build-slave
```

Outputs:

```text
build-master/dual-elevator-spi.hex   -> load into Master MCU
build-slave/dual-elevator-spi.hex    -> load into Slave MCU
```

## 2) Add Proteus components

| Quantity | Component | Notes |
|---:|---|---|
| 2 | `STM32F401VE` | Master Board A + Slave Board B |
| 2 | `LED-RED` | PWM motor simulation LEDs |
| 2 | `RES` | 220 Ω series resistor for each LED |
| 24 | `BUTTON` or `SW-SPST` | Master: 15 inputs. Slave: 9 inputs. |
| 2 | `VIRTUAL TERMINAL` | UART telemetry, one per board |
| 1 | `POWER` / `VCC` | 3.3 V rail |
| 1 | `GROUND` | Common ground |

## 3) STM32 power and boot setup

For each STM32:

1. Connect all visible `VDD` pins to **3.3 V**.
2. Connect all visible `VSS` pins to **GND**.
3. Connect `VDDA` to **3.3 V** and `VSSA` to **GND** if shown.
4. Tie `BOOT0` to **GND**.
5. No external crystal is required. Firmware uses internal HSI `16 MHz`.
6. Set MCU **Clock Frequency** to `16MHz` if Proteus exposes it.

## 4) Load firmware

Master STM32 properties:

```text
Program File = build-master/dual-elevator-spi.hex
Clock Frequency = 16MHz
```

Slave STM32 properties:

```text
Program File = build-slave/dual-elevator-spi.hex
Clock Frequency = 16MHz
```

## 5) IPC SPI2 wiring

This final patch uses hardware SPI2 to avoid the PB3/PB4/PB5 SPI1 issues that appeared in Proteus.

Connect exactly:

| Signal | Master pin | Slave pin |
|---|---|---|
| SPI2_NSS / CS | PB12 | PB12 |
| SPI2_SCK | PB13 | PB13 |
| SPI2_MISO | PB14 | PB14 |
| SPI2_MOSI | PB15 | PB15 |
| Common ground | VSS/GND | VSS/GND |

Important notes:

- `PB12` is **not a push button**. It is the CS/NSS wire controlled by the Master firmware.
- Use a common ground between the two MCUs, otherwise SPI logic levels have no shared reference.
- SPI mode is Mode 0: `CPOL=0`, `CPHA=0`.
- Master clocks an 8-byte full-duplex frame every `25 ms`.
- Slave preloads byte 0 before the master starts the next frame.

## 6) UART Virtual Terminal wiring

For each board:

| STM32 pin | Virtual Terminal pin |
|---|---|
| PA9 / USART1_TX | RXD |
| PA10 / USART1_RX | TXD, optional |
| GND | GND |

Virtual Terminal settings:

```text
Baud rate: 9600
Data bits: 8
Parity: none
Stop bits: 1
```

Expected boot messages:

```text
MASTER Board A ready
SLAVE Board B ready
```

## 7) PWM motor LED wiring

For each board:

```text
PA6 ---- 220 Ω resistor ---- LED anode
LED cathode ---------------- GND
```

Duty meaning:

- `0%`: stopped, doors open, or emergency.
- `20%`: slow speed near target floor.
- `100%`: full travel speed.

## 8) Master input wiring — Port D

All input pins use internal pull-up. Wire every button/sensor between the STM32 pin and GND. Pressing the button creates a falling edge interrupt.

For Proteus reliability, external pull-up is also fine:

```text
3.3V --- 10kΩ --- PDx --- button --- GND
```

| Input | Master pin |
|---|---|
| Cabin F1 | PD0 |
| Cabin F2 | PD1 |
| Cabin F3 | PD2 |
| Cabin F4 | PD3 |
| Hall U1 | PD5 |
| Hall D2 | PD6 |
| Hall U2 | PD7 |
| Hall D3 | PD8 |
| Hall U3 | PD9 |
| Hall D4 | PD10 |
| Floor sensor F1 | PD11 |
| Floor sensor F2 | PD12 |
| Floor sensor F3 | PD13 |
| Floor sensor F4 | PD14 |
| Emergency stop | PD15 |

PD4 is unused.

## 9) Slave input wiring — Port D

Wire each button/sensor between pin and GND.

| Input | Slave pin |
|---|---|
| Cabin F1 | PD0 |
| Cabin F2 | PD1 |
| Cabin F3 | PD2 |
| Cabin F4 | PD3 |
| Floor sensor F1 | PD11 |
| Floor sensor F2 | PD12 |
| Floor sensor F3 | PD13 |
| Floor sensor F4 | PD14 |
| Emergency stop | PD15 |

## 10) Expected startup VT

Master:

```text
MASTER Board A ready
M A:F1 S:IDLE T:0x0 H:0x0 B:F1 IDLE Comm:OK
```

Slave:

```text
SLAVE Board B ready
S B:F1 S:IDLE T:0x0 Mode:LINK
```

## 11) Common Proteus mistakes

| Problem | Fix |
|---|---|
| No UART text | Check PA9 to terminal RXD, baud `9600`, common GND. |
| SPI always fault | Check PB12/PB13/PB14/PB15 pin-to-pin and common GND. Do not use PA4 or PB3/PB4/PB5 with this version. |
| Buttons do nothing | Button must pull the pin to GND. Firmware uses pull-up and falling-edge EXTI. |
| LED always off | Confirm PA6 -> resistor -> LED -> GND. |
| MCU does not boot | Tie BOOT0 to GND and load the correct master/slave HEX. |
