/** Protocol.h */
#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "Std_Types.h"
#include "Spi.h"

#define IPC_HEADER             0xA5U
#define IPC_FLAG_EMERGENCY     (1U << 0U)
#define IPC_FLAG_COMM_FAULT    (1U << 1U)
#define IPC_FLAG_DIR_UP        (1U << 2U)
#define IPC_FLAG_DIR_DOWN      (1U << 3U)
#define IPC_FLAG_INDEPENDENT   (1U << 4U)

typedef struct {
    uint8 seq;
    uint8 state;
    uint8 current_floor;
    uint8 target_mask;
    uint8 flags;
    uint8 cmd_mask;
} IpcFrameType;

uint8 Protocol_Checksum(const uint8 Frame[SPI_FRAME_LEN]);
void Protocol_Build(const IpcFrameType *Packet, uint8 Frame[SPI_FRAME_LEN]);
uint8 Protocol_Parse(const uint8 Frame[SPI_FRAME_LEN], IpcFrameType *Packet);

#endif
