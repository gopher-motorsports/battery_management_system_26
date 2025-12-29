/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updatePackMonitorTask.h"
#include "adbms/adbmsPackMonitor.h"
#include <stdio.h>

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
    
}
