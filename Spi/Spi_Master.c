/**
 * Spi_Master.c
 * Master-only SPI IPC driver. Fixed for reliable Slave synchronization.
 */
#include "stm32f401xe.h"
#include "Spi.h"
#include "Gpio.h"
#include "Nvic.h"
#include "Critical.h"

#define IPC_SPI                 SPI2
#define IPC_SPI_IRQ_NUMBER      36U
#define IPC_PORT                GPIO_B
#define IPC_CS_PIN              12U
#define IPC_SCK_PIN             13U
#define IPC_MISO_PIN            14U
#define IPC_MOSI_PIN            15U

static const uint8 *Spi_MasterTx = 0;
static uint8 *Spi_MasterRx = 0;
static volatile uint8 Spi_MasterLen = 0;
static volatile uint8 Spi_MasterTxPos = 0;
static volatile uint8 Spi_MasterRxPos = 0;
static volatile uint8 Spi_MasterBusy = 0;
static SpiFrameCallback Spi_MasterDoneCb = 0;

static void Spi_ClearOverrun(void) {
    (void)IPC_SPI->DR;
    (void)IPC_SPI->SR;
}

static void Spi_GpioInit_Master(void) {
    Gpio_Init(IPC_PORT, IPC_CS_PIN, GPIO_OUTPUT, GPIO_PUSH_PULL);
    Gpio_WritePin(IPC_PORT, IPC_CS_PIN, HIGH);
    Gpio_Init(IPC_PORT, IPC_SCK_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(IPC_PORT, IPC_MISO_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(IPC_PORT, IPC_MOSI_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(IPC_PORT, IPC_SCK_PIN, GPIO_AF5);
    Gpio_SetAF(IPC_PORT, IPC_MISO_PIN, GPIO_AF5);
    Gpio_SetAF(IPC_PORT, IPC_MOSI_PIN, GPIO_AF5);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_CS_PIN);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_SCK_PIN);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_MISO_PIN);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_MOSI_PIN);
}

void Spi1_MasterInit(void) {
    Spi_GpioInit_Master();
    IPC_SPI->CR1 = 0;
    IPC_SPI->CR2 = 0;
    IPC_SPI->CR1 |= SPI_CR1_MSTR;                 /* Master mode */
    IPC_SPI->CR1 |= SPI_CR1_SSM | SPI_CR1_SSI;    /* Software NSS */
    IPC_SPI->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA); 
    IPC_SPI->CR1 &= ~SPI_CR1_BR;
    IPC_SPI->CR1 |= (3UL << SPI_CR1_BR_Pos);      /* 1 MHz */
    IPC_SPI->CR2 |= SPI_CR2_ERRIE;
    Nvic_SetPriority(IPC_SPI_IRQ_NUMBER, 5U);
    Nvic_EnableIrq(IPC_SPI_IRQ_NUMBER);
    IPC_SPI->CR1 |= SPI_CR1_SPE;
}

uint8 Spi1_MasterStartFrame(const uint8 *TxFrame, uint8 *RxFrame, uint8 Length, SpiFrameCallback DoneCb) {
    uint32 lock;
    if (TxFrame == 0 || RxFrame == 0 || Length == 0U || Length > SPI_FRAME_LEN) return NOK;

    lock = Enter_Critical();
    if (Spi_MasterBusy != 0U) { Exit_Critical(lock); return NOK; }

    Spi_MasterTx = TxFrame;
    Spi_MasterRx = RxFrame;
    Spi_MasterLen = Length;
    Spi_MasterTxPos = 0U;
    Spi_MasterRxPos = 0U;
    Spi_MasterDoneCb = DoneCb;
    Spi_MasterBusy = 1U;

    Spi_ClearOverrun();
    Gpio_WritePin(IPC_PORT, IPC_CS_PIN, LOW);
    // for(volatile uint8 delay = 0; delay < 15; delay++);

    /* BUG FIX: Only enable RXNE interrupt. Send the first byte to kick off the clock. */
    *((volatile uint8 *)&IPC_SPI->DR) = Spi_MasterTx[Spi_MasterTxPos++];
    IPC_SPI->CR2 |= SPI_CR2_RXNEIE | SPI_CR2_ERRIE;

    Exit_Critical(lock);
    return OK;
}

uint8 Spi1_MasterIsBusy(void) { return Spi_MasterBusy; }

void Spi1_MasterAbort(void) {
    uint32 lock = Enter_Critical();
    IPC_SPI->CR2 &= ~(SPI_CR2_RXNEIE | SPI_CR2_TXEIE | SPI_CR2_ERRIE);
    Spi_ClearOverrun();
    Gpio_WritePin(IPC_PORT, IPC_CS_PIN, HIGH);
    Spi_MasterBusy = 0U;
    Exit_Critical(lock);
}

void SPI2_IRQHandler(void) {
    if ((IPC_SPI->SR & SPI_SR_RXNE) != 0U) {
        /* 1. Read the byte that just finished arriving */
        Spi_MasterRx[Spi_MasterRxPos++] = (uint8)IPC_SPI->DR;

        for(volatile uint8 delay = 0; delay < 25; delay++);

        /* 2. If we have more to send, send it NOW. This gives the Slave a safe gap. */
        if (Spi_MasterRxPos < Spi_MasterLen) {
            *((volatile uint8 *)&IPC_SPI->DR) = Spi_MasterTx[Spi_MasterTxPos++];
        } else {
            /* Frame Complete */
            IPC_SPI->CR2 &= ~SPI_CR2_RXNEIE;
            // for(volatile uint8 delay = 0; delay < 15; delay++);
            Gpio_WritePin(IPC_PORT, IPC_CS_PIN, HIGH);
            Spi_MasterBusy = 0U;
            if (Spi_MasterDoneCb != 0) Spi_MasterDoneCb();
        }
    }

    if ((IPC_SPI->SR & SPI_SR_OVR) != 0U) {
        Spi_ClearOverrun();
    }
}