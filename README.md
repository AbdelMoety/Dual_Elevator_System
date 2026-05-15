# STM32F401VE Collaborative Dual-Elevator System over SPI IPC

Final fast version based on the provided STM32F4 template/tutorial style.

## What is inside

- Two-board build from one codebase:
  - `ELEVATOR_ROLE=MASTER`: Dispatcher + Elevator A.
  - `ELEVATOR_ROLE=SLAVE`: Elevator B.
- Minimal register-level layers:
  - `Rcc`, `Gpio`, `Exti`, `Nvic`, `Timer`, `Pwm`, `Usart`, `Spi`.
- Application layer:
  - Elevator finite-state machine.
  - 8-byte SPI packet protocol with checksum.
  - Master task allocation algorithm.
  - Critical sections around SPI shared buffers.
  - Volatile ISR-to-main flags.
  - Non-blocking SPI and timing; UART debug output is the only polling part.

## Quick build

Set the Arm toolchain path in `cmake/ArmToolchain.cmake`, then:

```bash
cmake -S . -B build-master -GNinja -DELEVATOR_ROLE=MASTER
cmake --build build-master

cmake -S . -B build-slave -GNinja -DELEVATOR_ROLE=SLAVE
cmake --build build-slave
```

Load these files in Proteus:

```text
build-master/dual-elevator-spi.hex   -> Master STM32F401VE
build-slave/dual-elevator-spi.hex    -> Slave STM32F401VE
```

## Core pins

| Function | Master | Slave |
|---|---|---|
| PWM motor LED | PA6 | PA6 |
| UART TX | PA9 | PA9 |
| UART RX | PA10 | PA10 |
| SPI2 CS / NSS | PB12 output | PB12 hardware NSS input |
| SPI2 SCK | PB13 | PB13 |
| SPI2 MISO | PB14 | PB14 |
| SPI2 MOSI | PB15 | PB15 |
| Common ground | VSS/GND | VSS/GND |

## Fast version timings

| Item | Period / time |
|---|---:|
| UART telemetry | 200 ms |
| SPI state exchange | 25 ms |
| Floor travel simulation | 400 ms per floor |
| Door open time | 600 ms |
| FSM tick | 10 ms |

## Documents

- `PROTEUS_GUIDE.md`: wiring and Proteus configuration.
- `PACKET_DEFINITION.md`: required 8-byte SPI frame diagram.
- `LAB_PREP.md`: register map, PWM math, packet definition, allocation notes.
- `NON_BLOCKING_REVIEW.md`: why the code satisfies the no-busy-wait rule.
- `TEST_CASES.md`: demo tests and expected VT output.
