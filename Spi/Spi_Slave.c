/**
 * Spi_Slave.c
 * Slave-only SPI IPC driver.
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

static volatile uint8 Spi_SlaveTx[SPI_FRAME_LEN] = {0xA5U, 0, 0, 0, 0, 0, 0, 0};
static volatile uint8 Spi_SlaveRx[SPI_FRAME_LEN] = {0};
static volatile uint8 Spi_SlaveLen = SPI_FRAME_LEN;
static volatile uint8 Spi_SlaveTxPos = 0;
static volatile uint8 Spi_SlaveRxPos = 0;
static volatile uint8 Spi_SlaveNewFrame = 0;
static SpiFrameCallback Spi_SlaveDoneCb = 0;

static void Spi_ClearOverrun(void) {
    (void)IPC_SPI->DR;
    (void)IPC_SPI->SR;
}

static void Spi_GpioInit_Slave(void) {
    Gpio_Init(IPC_PORT, IPC_CS_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(IPC_PORT, IPC_SCK_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(IPC_PORT, IPC_MISO_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_Init(IPC_PORT, IPC_MOSI_PIN, GPIO_AF, GPIO_PUSH_PULL);
    Gpio_SetAF(IPC_PORT, IPC_CS_PIN, GPIO_AF5);
    Gpio_SetAF(IPC_PORT, IPC_SCK_PIN, GPIO_AF5);
    Gpio_SetAF(IPC_PORT, IPC_MISO_PIN, GPIO_AF5);
    Gpio_SetAF(IPC_PORT, IPC_MOSI_PIN, GPIO_AF5);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_CS_PIN);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_SCK_PIN);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_MISO_PIN);
    Gpio_SetSpeedHigh(IPC_PORT, IPC_MOSI_PIN);

    /* Keep NSS high when master resets */
    GPIOB->PUPDR &= ~(0x03UL << (IPC_CS_PIN * 2U));
    GPIOB->PUPDR |=  (0x01UL << (IPC_CS_PIN * 2U));
}

void Spi1_SlaveInit(SpiFrameCallback DoneCb) {
    Spi_GpioInit_Slave();
    IPC_SPI->CR1 = 0;
    IPC_SPI->CR2 = 0;
    IPC_SPI->CR1 &= ~SPI_CR1_MSTR;                /* Slave mode */
    IPC_SPI->CR1 &= ~SPI_CR1_SSM;                 /* Hardware NSS */
    IPC_SPI->CR1 &= ~(SPI_CR1_CPOL | SPI_CR1_CPHA);
    IPC_SPI->CR2 |= SPI_CR2_RXNEIE | SPI_CR2_ERRIE;

    Spi_SlaveDoneCb = DoneCb;
    Spi_SlaveRxPos = 0U;
    Spi_SlaveTxPos = 0U;
    Spi_SlaveNewFrame = 0U;

    Nvic_SetPriority(IPC_SPI_IRQ_NUMBER, 5U);
    Nvic_EnableIrq(IPC_SPI_IRQ_NUMBER);
    IPC_SPI->CR1 |= SPI_CR1_SPE;

    /* Preload byte 0 before master pulls NSS low */
    if ((IPC_SPI->SR & SPI_SR_TXE) != 0U) {
        *((volatile uint8 *)&IPC_SPI->DR) = Spi_SlaveTx[0];
        Spi_SlaveTxPos = 1U;
    }
}

void Spi1_SlaveSetTxFrame(const uint8 *TxFrame, uint8 Length) {
    uint8 i;
    uint32 lock;
    if (TxFrame == 0 || Length == 0U || Length > SPI_FRAME_LEN) return;

    lock = Enter_Critical();
    Spi_SlaveLen = Length;
    for (i = 0U; i < Length; i++) {
        Spi_SlaveTx[i] = TxFrame[i];
    }
    if (Spi_SlaveRxPos == 0U && ((IPC_SPI->SR & SPI_SR_TXE) != 0U)) {
        *((volatile uint8 *)&IPC_SPI->DR) = Spi_SlaveTx[0];
        Spi_SlaveTxPos = 1U;
    }
    Exit_Critical(lock);
}

uint8 Spi1_SlaveReadRxFrame(uint8 *RxFrame, uint8 Length) {
    uint8 i;
    uint32 lock;
    if (RxFrame == 0 || Length == 0U || Length > SPI_FRAME_LEN) return NOK;

    lock = Enter_Critical();
    if (Spi_SlaveNewFrame == 0U) { Exit_Critical(lock); return NOK; }
    for (i = 0U; i < Length; i++) { RxFrame[i] = Spi_SlaveRx[i]; }
    Spi_SlaveNewFrame = 0U;
    Exit_Critical(lock);
    return OK;
}

void Spi1_OnCsEdge(void) { }

void SPI2_IRQHandler(void) {
    if ((IPC_SPI->SR & SPI_SR_RXNE) != 0U) {
        Spi_SlaveRx[Spi_SlaveRxPos++] = (uint8)IPC_SPI->DR;

        if ((IPC_SPI->SR & SPI_SR_TXE) != 0U) {
            if (Spi_SlaveTxPos < Spi_SlaveLen) {
                *((volatile uint8 *)&IPC_SPI->DR) = Spi_SlaveTx[Spi_SlaveTxPos++];
            } else {
                *((volatile uint8 *)&IPC_SPI->DR) = 0xFFU;
            }
        }

        if (Spi_SlaveRxPos >= Spi_SlaveLen) {
            Spi_SlaveNewFrame = 1U;
            if (Spi_SlaveDoneCb != 0) Spi_SlaveDoneCb();
            
            Spi_SlaveRxPos = 0U;
            Spi_SlaveTxPos = 0U;
            if ((IPC_SPI->SR & SPI_SR_TXE) != 0U) {
                *((volatile uint8 *)&IPC_SPI->DR) = Spi_SlaveTx[0];
                Spi_SlaveTxPos = 1U;
            }
        }
    }

    if ((IPC_SPI->SR & SPI_SR_OVR) != 0U) {
        Spi_ClearOverrun();
        Spi_SlaveRxPos = 0U;
        Spi_SlaveTxPos = 0U;
    }
}