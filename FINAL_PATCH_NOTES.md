# Final Patch Notes

Base folder: `DualElevator_SPI_STM32F401VE_v2_portD_fastVT` uploaded by the user.

## Fixed without slowing the project

- Kept fast timings:
  - VT telemetry: `200 ms`
  - SPI exchange request: `25 ms`
  - Floor travel: `400 ms`
  - Door open: `600 ms`
- Replaced unstable Proteus SPI1 PB3/PB4/PB5 link with hardware SPI2:
  - PB12 = CS/NSS
  - PB13 = SCK
  - PB14 = MISO
  - PB15 = MOSI
- The SPI driver is interrupt-driven and does not busy-wait.
- Slave preloads the first TX byte before the master starts the next frame.
- EXTI callbacks now only set volatile flags; the main loop updates the FSM.
- Added `PACKET_DEFINITION.md` for the required point 6 preparation.
- Added `NON_BLOCKING_REVIEW.md` for the critical no-busy-wait requirement.

## Build status in this environment

- MASTER source syntax: PASS using clang arm-none-eabi target.
- SLAVE source syntax: PASS using clang arm-none-eabi target.
- HEX generation requires your local Arm GNU Toolchain.
