# Lab Preparation — Dual Elevator over SPI IPC

## 1) Register map used

| Peripheral | Purpose in project | Base address |
|---|---|---:|
| RCC | Enable GPIO/TIM/USART/SPI/SYSCFG clocks | `0x40023800` |
| GPIOA | PWM PA6, UART PA9/PA10 | `0x40020000` |
| GPIOB | Hardware SPI2 IPC: PB12/PB13/PB14/PB15 | `0x40020400` |
| GPIOD | Buttons and floor sensors | `0x40020C00` |
| SYSCFG | Route GPIO pins to EXTI lines | `0x40013800` |
| EXTI | Asynchronous button/sensor interrupts | `0x40013C00` |
| SysTick | 1 ms scheduler timer interrupt | `0xE000E010` |
| TIM3 | 10 kHz PWM motor LED on PA6 / CH1 | `0x40000400` |
| USART1 | Telemetry to PC / Virtual Terminal | `0x40011000` |
| SPI2 | Full-duplex IPC link | `0x40003800` |
| NVIC | Enable IRQs and set priorities | `0xE000E100`, priority bytes at `0xE000E400` |

## 2) PWM math

System clock is kept at the reset/tutorial-friendly HSI value: `16 MHz`.

Motor LED PWM uses TIM3 Channel 1 on PA6.

Target PWM frequency: `10 kHz`.

Formula:

```text
Fpwm = Ftimer / ((PSC + 1) * (ARR + 1))
```

Selected values:

```text
PSC = 0
ARR = 1599
Fpwm = 16,000,000 / ((0 + 1) * (1599 + 1)) = 10,000 Hz
```

Duty mapping:

| Elevator action | Duty |
|---|---:|
| Stop / doors open / emergency | `0%` |
| Slow near target floor | `20%` |
| Full travel | `100%` |

## 3) Packet Definition — 8-byte SPI frame

Same frame layout is used in both directions.

```text
Byte index:   0        1        2        3        4        5        6        7
           +--------+--------+--------+--------+--------+--------+--------+--------+
Field:     | HEADER |  SEQ   | STATE  | FLOOR  | TARGET | FLAGS  |  CMD   |  XOR   |
           +--------+--------+--------+--------+--------+--------+--------+--------+
Value:     |  0xA5  | 0..255 | 0..4   | 1..4   | mask   | bits   | mask   | chk    |
```

| Byte | Name | Meaning |
|---:|---|---|
| 0 | Header | Fixed `0xA5` |
| 1 | Seq | Rolling sequence counter |
| 2 | State | `0=IDLE`, `1=UP`, `2=DOWN`, `3=DOOR`, `4=EMERG` |
| 3 | Current floor | `1..4` |
| 4 | Target mask | Bit0=F1, Bit1=F2, Bit2=F3, Bit3=F4 |
| 5 | Flags | Bit0 emergency, bit1 comm fault, bit2 up, bit3 down, bit4 independent |
| 6 | Command mask | Master-to-slave assigned target floors. Slave-to-master sends `0`. |
| 7 | Checksum | XOR of bytes `0..6` |

## 4) Master task allocation logic

For every hallway request, the master scores Elevator A and Elevator B:

1. **Communication fault**: assign the call to Elevator A only.
2. **Immediate**: elevator is idle and already at requested floor → best score.
3. **Perfect match**: elevator is moving toward the requested floor in the same requested direction.
4. **Passed match**: elevator is moving in the same requested direction but already passed the floor.
5. **Opposite direction**: do not assign yet; keep the hall request pending.
6. **Idle fallback**: if no directional match exists, assign the nearest idle elevator.
