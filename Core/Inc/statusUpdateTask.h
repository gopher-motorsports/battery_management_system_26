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

#define SHDN_END_V_GAIN         11
#define SHDN_END_V_THRESHOLD    10.0f
#define PRECHARGE_WINDOW_MS     3000

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

extern volatile uint32_t adcRawValue;
extern volatile uint32_t adcNewDataFlag;
extern volatile bool prechargeDelayComplete;

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
