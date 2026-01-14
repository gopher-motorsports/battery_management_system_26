#ifndef INC_CELL_MONITOR_TELEMETRY_H_
#define INC_CELL_MONITOR_TELEMETRY_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsCellMonitor.h"
#include "debug.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_CELL_MON    1

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

TRANSACTION_STATUS_E updateCellTelemetry(CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData);

TRANSACTION_STATUS_E updateCellBalancing(CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData);

#endif /* INC_CELL_MONITOR_TELEMETRY_H_ */
