# Packet Definition — Required Preparation Point

This is the only preparation item expected for point 6 during discussion.

## 8-byte SPI IPC frame

Same frame is used in both directions: Master → Slave and Slave → Master.

```text
Byte index:   0        1        2        3        4        5        6        7
           +--------+--------+--------+--------+--------+--------+--------+--------+
Field:     | HEADER |  SEQ   | STATE  | FLOOR  | TARGET | FLAGS  |  CMD   |  XOR   |
           +--------+--------+--------+--------+--------+--------+--------+--------+
Value:     |  0xA5  | 0..255 | 0..4   | 1..4   | mask   | bits   | mask   | chk    |
```

## Field meaning

| Byte | Field | Meaning |
|---:|---|---|
| 0 | `HEADER` | Fixed `0xA5`; used to reject invalid frames. |
| 1 | `SEQ` | Rolling sequence counter; increments every transmitted frame. |
| 2 | `STATE` | `0=IDLE`, `1=UP`, `2=DOWN`, `3=DOOR`, `4=EMERG`. |
| 3 | `FLOOR` | Current elevator floor, `1..4`. |
| 4 | `TARGET` | Local target bit mask: bit0=F1, bit1=F2, bit2=F3, bit3=F4. |
| 5 | `FLAGS` | bit0 emergency, bit1 comm fault, bit2 moving up, bit3 moving down, bit4 independent. |
| 6 | `CMD` | Master-to-slave command target mask. Slave sends `0` in this byte. |
| 7 | `XOR` | Checksum = XOR of bytes `0..6`. |

## Checksum rule

```c
checksum = frame[0] ^ frame[1] ^ frame[2] ^ frame[3] ^ frame[4] ^ frame[5] ^ frame[6];
frame[7] = checksum;
```

A received frame is accepted only when:

```text
frame[0] == 0xA5
frame[7] == XOR(frame[0..6])
```
