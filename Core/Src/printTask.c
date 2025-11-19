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

PORT_INSTANCE_S port2 = {
    .spiHandle = &hspi1,
    .csPort = PORTB_CS_GPIO_Port,
    .csPin = PORTB_CS_Pin
};

uint8_t txBuffer[6] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initPrintTask()
{
    printf("\e[1;1H\e[2J");

    // Set both CS high upon start up
    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);

    // Wake the device
    activatePort(&port1, 1, 500);

}

void runPrintTask()
{
    // // Ready the device
    // activatePort(&port1, 1, 10);
    // //writeRegister(0x0024, 1, txBuffer, &port1);
    // port1.localCommandCounter++;
    // if(port1.localCommandCounter > 63)
    // {
    //     port1.localCommandCounter = 1;
    // }
    // static uint8_t rxBuffer[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    // printf("\e[1;1H\e[2J");

    // // start s-adc conversion, read register
    // sendCommand(0x2C0, &port1); // ADCV command
    // // sendCommand(PLCADC); how is polling done?
    // printf("return status: %lu\n", (uint32_t)readRegister(RDCVA, 1, rxBuffer, &port1));
    // printf("rxBuffer: \n");
    // for (uint8_t i = 0; i < 6; i++) {
    //     printf("index[%d]: %X\n", i, rxBuffer[i]);
    // }
    // printf("return status: %lu\n", (uint32_t)readRegister(RDAUXD, 1, rxBuffer, &port1));
    // printf("rxBuffer: \n");
    // for (uint8_t i = 0; i < 6; i++) {
    //     printf("index[%d]: %X\n", i, rxBuffer[i]);
    // }


    // read register
    // printf("return status: %lu\n", (uint32_t)readRegister(RDCFGB, 1, rxBuffer, &port1));
    // printf("rxBuffer: \n");
    // for (uint8_t i = 0; i < 6; i++) {
    //     printf("index[%d]: %X\n", i, rxBuffer[i]);
    // }
}