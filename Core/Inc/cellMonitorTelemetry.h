#ifndef INC_CELL_MONITOR_TELEMETRY_H_
#define INC_CELL_MONITOR_TELEMETRY_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsCellMonitor.h"
#include "debug.h"

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

TRANSACTION_STATUS_E updateCellTelemetry(CHAIN_INFO_S* chainInfoData, ADBMS_CellMonitorData* cellMonitorData);

#endif /* INC_CELL_MONITOR_TELEMETRY_H_ */
