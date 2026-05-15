/** Protocol.c */
#include "Protocol.h"

uint8 Protocol_Checksum(const uint8 Frame[SPI_FRAME_LEN]) {
    uint8 checksum = 0U;
    uint8 i;
    for (i = 0U; i < (SPI_FRAME_LEN - 1U); i++) {
        checksum ^= Frame[i];
    }
    return checksum;
}

void Protocol_Build(const IpcFrameType *Packet, uint8 Frame[SPI_FRAME_LEN]) {
    Frame[0] = IPC_HEADER;
    Frame[1] = Packet->seq;
    Frame[2] = Packet->state;
    Frame[3] = Packet->current_floor;
    Frame[4] = Packet->target_mask;
    Frame[5] = Packet->flags;
    Frame[6] = Packet->cmd_mask;
    Frame[7] = Protocol_Checksum(Frame);
}

uint8 Protocol_Parse(const uint8 Frame[SPI_FRAME_LEN], IpcFrameType *Packet) {
    if (Frame[0] != IPC_HEADER) {
        return NOK;
    }
    if (Frame[7] != Protocol_Checksum(Frame)) {
        return NOK;
    }

    Packet->seq = Frame[1];
    Packet->state = Frame[2];
    Packet->current_floor = Frame[3];
    Packet->target_mask = Frame[4];
    Packet->flags = Frame[5];
    Packet->cmd_mask = Frame[6];
    return OK;
}
