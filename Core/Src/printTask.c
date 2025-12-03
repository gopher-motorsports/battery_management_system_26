/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "printTask.h"
#include <stdio.h>
#include "main.h"

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

static uint32_t lastHeartbeatUpdate = 0;

void initPrintTask()
{
    // HAL_GPIO_WritePin(MCU_HEART_GPIO_Port, MCU_HEART_Pin, GPIO_PIN_SET);
    if(HAL_GetTick() - lastHeartbeatUpdate > 800)
    {
        HAL_GPIO_TogglePin(MCU_FAULT_GPIO_Port, MCU_FAULT_Pin);
        lastHeartbeatUpdate = HAL_GetTick();
        printf("Heartbeat!\n");
    }
}

void runPrintTask()
{
    printf("\e[1;1H\e[2J");


}