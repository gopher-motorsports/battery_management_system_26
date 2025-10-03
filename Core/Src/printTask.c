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
}