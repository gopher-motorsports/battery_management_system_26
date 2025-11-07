/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include "main.h"
#include "adbms/adbmsSpi.h"
#include <stdio.h>

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

extern SPI_HandleTypeDef hspi1;

PORT_INSTANCE_S port1 = {
    .spiHandle = &hspi1,
    .csPort = PORTA_CS_GPIO_Port,
    .csPin = PORTA_CS_Pin
};

uint8_t txBuffer[6] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  ((byte) & 0x80 ? '1' : '0'), \
  ((byte) & 0x40 ? '1' : '0'), \
  ((byte) & 0x20 ? '1' : '0'), \
  ((byte) & 0x10 ? '1' : '0'), \
  ((byte) & 0x08 ? '1' : '0'), \
  ((byte) & 0x04 ? '1' : '0'), \
  ((byte) & 0x02 ? '1' : '0'), \
  ((byte) & 0x01 ? '1' : '0')

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initPrintTask()
{
    printf("\e[1;1H\e[2J");

    // Wake the device
    activatePort(&port1, 1, 500);
}

void runPrintTask()
{
    // Ready the device
    activatePort(&port1, 1, 10);
    writeRegister(0x0024, 1, txBuffer, &port1);
    port1.localCommandCounter++;
    if(port1.localCommandCounter > 63)
    {
        port1.localCommandCounter = 1;
    }
    static uint8_t rxBuffer[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    printf("return status: %lu\n", (uint32_t)readRegister(0x0002, 1, rxBuffer, &port1));
    printf("\e[1;1H\e[2J");
    printf("rxBuffer: \n");
    for (uint8_t i = 0; i < 6; i++) {
        // printf(BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(rxBuffer[i]));
        printf("index[%d]: %X\n", i, rxBuffer[i]);
    }
    printf("\n");
}