#ifndef INC_PACK_MONITOR_TELEMETRY_H_
#define INC_PACK_MONITOR_TELEMETRY_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsPackMonitor.h"
#include "debug.h"

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

TRANSACTION_STATUS_E updatePackTelemetry(CHAIN_INFO_S* chainInfoData, ADBMS_PackMonitorData* packMonitorData);

#endif /* INC_PACK_MONITOR_TELEMETRY_H_ */
