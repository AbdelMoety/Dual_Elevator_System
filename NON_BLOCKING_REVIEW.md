# Non-Blocking Review Notes

Critical project rule: polling is only allowed for UART debug output. All other project timing is interrupt/timer based.

## What this version uses

| Part | Implementation | Busy wait? |
|---|---|---|
| Scheduler timing | SysTick 1 ms interrupt | No |
| Button/sensor inputs | EXTI interrupts | No |
| SPI IPC | Hardware SPI2 + SPI2 IRQ | No |
| Slave TX preload | `Spi1_SlaveSetTxFrame()` keeps byte 0 ready before Master clocks next frame | No |
| Elevator FSM timing | Consumes SysTick flags in main loop | No |
| UART telemetry | Blocking/polling transmit | Yes, allowed by statement |

## ISR safety

- EXTI callbacks only set `volatile` event flags.
- Main loop consumes those flags and updates the elevator FSM.
- SPI buffers are copied/read inside `Enter_Critical()` / `Exit_Critical()`.
