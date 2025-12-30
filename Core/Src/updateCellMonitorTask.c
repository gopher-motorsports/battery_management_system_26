/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updateCellMonitorTask.h"
#include "adbms/adbmsCellMonitor.h"
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

CHAIN_INFO_S chainInfo;

// Do something like this to replace ADBMS_BatteryData??
static ADBMS_CellMonitorData cellMonitor[1];

uint8_t txBuffer[6] = {0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdateCellMonitorTask()
{
    // Set both CS high upon start up
    HAL_GPIO_WritePin(PORTA_CS_GPIO_Port, PORTA_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(PORTB_CS_GPIO_Port, PORTB_CS_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(BMS_FAULT_GPIO_Port, BMS_FAULT_Pin, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(BMS_INB_N_GPIO_Port, BMS_INB_N_Pin, GPIO_PIN_SET);

    // Wake the device
    activatePort(&chainInfo, TIME_WAKE_US);

    // Init chain to default values (this will become initChain function)
    chainInfo.commPorts[PORTA] = port1;
    chainInfo.commPorts[PORTB] = port2;
    chainInfo.numDevs = 1;
    chainInfo.currentPort = PORTA;
    chainInfo.chainStatus = CHAIN_COMPLETE;
    // chainInfo.localCommandCounter = 0;

}

void runUpdateCellMonitorTask()
{
    // Ready the device
    activatePort(&chainInfo, TIME_READY_US);

    updateChainStatus(&chainInfo);

    printf("Starting comms . . .\n");
    TRANSACTION_STATUS_E status = readCellMonitorSerialId(&chainInfo, cellMonitor);
    printf("Serial ID status: %u\n", status);
    for(uint8_t i = 0; i < 6; i++)
    {
        printf("Serial ID reading: %X\n", cellMonitor->serialId[i]);
    }

    status = startCellConversions(&chainInfo, NON_REDUNDANT_MODE, CONTINUOUS_MODE, DISCHARGE_DISABLED, NO_FILTER_RESET, CELL_OPEN_WIRE_DISABLED);
    readCellVoltages(&chainInfo, cellMonitor, RAW_CELL_VOLTAGE);
    for(uint8_t i = 0; i < NUM_CELLS_PER_CELL_MONITOR; i++)
    {
        printf("Cell Voltage %u: %f\n", i, cellMonitor->cellVoltage[i]);
    }


    // writeRegister(0x0024, 1, txBuffer, &port1);
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
