#ifndef INC_STATUS_UPDATE_TASK_H_
#define INC_STATUS_UPDATE_TASK_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include <stdbool.h>
#include <stdint.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_SDC_SENSE_INPUTS    6

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern volatile uint32_t adcRawValue;
extern volatile uint32_t adcNewDataFlag;

/* ==================================================================== */
/* ============================== STRUCTS ============================= */
/* ==================================================================== */

typedef struct
{
    bool imdLatchOpen;
    bool bmsLatchOpen;
    bool bmsInhibitActive;
    bool sdcStatusI2BInterlock;
    bool sdcStatusTBInterlock;

    float shutdownEndVoltage_V;
} shutdownCircuitStatus_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void initStatusUpdateTask();
void runStatusUpdateTask();

#endif // INC_STATUS_UPDATE_TASK_H_
