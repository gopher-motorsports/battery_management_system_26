/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updatePackMonitorTask.h"
#include "adbms/adbmsPackMonitor.h"
#include <stdio.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define LINK_DIV_GAIN 483.35f

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

extern SPI_HandleTypeDef hspi2;

PORT_INSTANCE_S packMonPort = {
    .spiHandle = &hspi2,
    .csPort = PACK_MON_CS_N_GPIO_Port,
    .csPin = PACK_MON_CS_N_Pin
};

CHAIN_INFO_S packMonInfo;

static ADBMS_PackMonitorData packMonitor;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initUpdatePackMonitorTask()
{
    // Set CS high upon start up
    HAL_GPIO_WritePin(PACK_MON_CS_N_GPIO_Port, PACK_MON_CS_N_Pin, GPIO_PIN_SET);

    // Init chain to default values (this will become initChain function)
    packMonInfo.commPorts[PORTA] = packMonPort;
    packMonInfo.commPorts[PORTB] = packMonPort;
    packMonInfo.numDevs = 1;
    packMonInfo.currentPort = PORTA;
    packMonInfo.chainStatus = CHAIN_COMPLETE;


}

void runUpdatePackMonitorTask()
{
    updateChainStatus(&packMonInfo);

    TRANSACTION_STATUS_E status = readPackMonitorSerialId(&packMonInfo, &packMonitor);
    
    printf("Serial ID Transaction Status: %u\n", status);

    for(uint8_t i = 0; i < REGISTER_SIZE_BYTES; i++)
    {
        printf("Serial ID Byte [%u]: %X\n", i, packMonitor.serialId[i]);
    }

    readPackMonitorConfigA(&packMonInfo, &packMonitor);
    packMonitor.configGroupA.v4Reference = 1;
    packMonitor.configGroupA.v6Reference = 1;
    writePackMonitorConfigA(&packMonInfo, &packMonitor);

    static bool voltageAdcStarted = 0;

    if(!voltageAdcStarted)
    {
        status = startAdcConversions(&packMonInfo, REDUNDANT_MODE, CONTINUOUS_MEASUREMENT);
        voltageAdcStarted = 1;
    }

    status = readVoltageAdc1(&packMonInfo, &packMonitor);

    status = startVoltageConversions(&packMonInfo, OPEN_WIRE_DISABLED, PACK_ALL_CHANNELS);

    float linkPlusDivVoltage = packMonitor.voltageAdc[3];
    float linkMinusDivVoltage = packMonitor.voltageAdc[5];

    printf("Link+ Div Voltage: %f\n", linkPlusDivVoltage);
    printf("Link- Div Voltage: %f\n", linkMinusDivVoltage);
    // TODO: calculate using divider gain

    float linkVoltage = (packMonitor.voltageAdc[3] - packMonitor.voltageAdc[5]) * LINK_DIV_GAIN;

    printf("Link Voltage: %f\n", linkVoltage);

    
}
