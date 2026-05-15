/** Spi.h */
#ifndef SPI_DRIVER_H
#define SPI_DRIVER_H

#include "Std_Types.h"

#define SPI_FRAME_LEN   8U

typedef void (*SpiFrameCallback)(void);

void Spi1_MasterInit(void);
uint8 Spi1_MasterStartFrame(const uint8 *TxFrame, uint8 *RxFrame, uint8 Length, SpiFrameCallback DoneCb);
uint8 Spi1_MasterIsBusy(void);
void Spi1_MasterAbort(void);

void Spi1_SlaveInit(SpiFrameCallback DoneCb);
void Spi1_SlaveSetTxFrame(const uint8 *TxFrame, uint8 Length);
uint8 Spi1_SlaveReadRxFrame(uint8 *RxFrame, uint8 Length);
void Spi1_OnCsEdge(void);

#endif
