# Demo Test Cases

## Startup

Expected on Master:

```text
MASTER Board A ready
M A:F1 S:IDLE T:0x0 H:0x0 B:F1 IDLE Comm:OK
```

Expected on Slave:

```text
SLAVE Board B ready
S B:F1 S:IDLE T:0x0 Mode:LINK
```

## Test 1 — Master cabin request

Press Master `PD3` = Cabin F4.

Expected:

```text
M A:F1 S:UP T:0x8 H:0x0 B:F1 IDLE Comm:OK
M A:F2 S:UP T:0x8 H:0x0 B:F1 IDLE Comm:OK
M A:F3 S:UP T:0x8 H:0x0 B:F1 IDLE Comm:OK
M A:F4 S:DOOR T:0x0 H:0x0 B:F1 IDLE Comm:OK
```

## Test 2 — Slave cabin request

Press Slave `PD2` = Cabin F3.

Expected:

```text
S B:F1 S:UP T:0x4 Mode:LINK
S B:F2 S:UP T:0x4 Mode:LINK
S B:F3 S:DOOR T:0x0 Mode:LINK
```

Master should also show B status changing.

## Test 3 — Hall call assigned by Master

Press Master `PD5` = Hall U1.

If A is at F1 and idle, expected immediate handling:

```text
M A:F1 S:DOOR T:0x0 H:0x0 B:F1 IDLE Comm:OK
```

## Test 4 — Perfect match

1. Press Master `PD3` so A moves up to F4.
2. While A is moving up, press Master `PD7` = Hall U2.

Expected: A takes the call because it is moving toward F2 in the same direction.

## Test 5 — Emergency

Press `PD15` on either board.

Expected state:

```text
S:EMERG
```

PWM motor LED becomes off.

## Test 6 — SPI fault

Disconnect one SPI wire or stop the slave.

Expected after timeout:

```text
Master: Comm:FAULT
Slave:  Mode:INDEP
```
