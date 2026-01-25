#ifndef INC_PACKDATA_H_
#define INC_PACKDATA_H_

#include "lookupTable.h"
#include "utils.h"

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    UNINITIALIZED = 0,
    GOOD,
    BAD
} SENSOR_STATUS_E;

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* Pack Data */

// Number of cells in parallel
#define NUM_PARALLEL_CELLS          1

// Number of cells in series
#define NUM_SERIES_CELLS            140

#define PACK_MILLICOULOMBS          CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS * MINUTES_IN_HOUR * SECONDS_IN_MINUTE
#define PACK_MILLIJOULES            PACK_MILLICOULOMBS * NOMINAL_CELL_VOLTAGE

#define MAX_TEMP_SENSOR_VALUE_C     192.476f
#define MIN_TEMP_SENSOR_VALUE_C     -53.870f

#define ABS_MAX_DISCHARGE_CURRENT_A 250.0f
#define ABS_MAX_CHARGE_CURRENT_A    15.0f

/* Cell Data */

#define CELL_CAPACITY_MAH           13000.0f
#define MAX_C_RATING                1

#define CELL_POLARIZATION_REST_SEC  20
#define CELL_POLARIZATION_REST_MS   CELL_POLARIZATION_REST_SEC * MILLISECONDS_IN_SECOND

#define MAX_CELL_VOLTAGE           4.28f
#define MAX_CELL_WARNING_VOLTAGE   4.3f
#define MAX_CELL_FAULT_VOLTAGE     4.33f
#define MAX_CELL_VOLTAGE_LIMIT     4.5f // indicates bad voltage sensor

#define NOMINAL_CELL_VOLTAGE       3.78f
#define MIN_CELL_WARNING_VOLTAGE   3.1f
#define MIN_CELL_FAULT_VOLTAGE     3.0f
#define MIN_CELL_VOLTAGE_LIMIT     1.2f // indicates bad voltage sensor

#define MAX_CELL_TEMP_WARNING_C    55.0f
#define MAX_CELL_TEMP_FAULT_C      60.0f

#define MAX_PACK_VOLTAGE            MAX_CELL_VOLTAGE * NUM_SERIES_CELLS
#define MIN_PACK_VOLTAGE            MIN_CELL_FAULT_VOLTAGE * NUM_SERIES_CELLS
#define VOLTAGE_MARGIN              10.0f

// Structs

extern const LookupTable_S cellTempTable;
extern const LookupTable_S shuntTempTable;
extern const LookupTable_S prechargeDischargeTempTable;
extern const LookupTable_S shuntResistanceTable;
extern const LookupTable_S stateOfChargeTable;
extern const LookupTable_S stateOfEnergyTable;

#endif /* INC_PACKDATA_H_ */
