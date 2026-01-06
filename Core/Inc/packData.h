#ifndef INC_PACKDATA_H_
#define INC_PACKDATA_H_

#include "lookupTable.h"
#include "utils.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* Pack Data */

// Number of cells in parallel
#define NUM_PARALLEL_CELLS          1

// Number of cells in series
#define NUM_SERIES_CELLS            140

#define PACK_MILLICOULOMBS          CELL_CAPACITY_MAH * NUM_PARALLEL_CELLS * MINUTES_IN_HOUR * SECONDS_IN_MINUTE
#define PACK_MILLIJOULES            PACK_MILLICOULOMBS * NOMINAL_BRICK_VOLTAGE

#define MAX_TEMP_SENSOR_VALUE_C     120.0f
#define MIN_TEMP_SENSOR_VALUE_C     -40.0f

#define ABS_MAX_DISCHARGE_CURRENT_A 250.0f
#define ABS_MAX_CHARGE_CURRENT_A    15.0f

/* Cell Data */

#define CELL_CAPACITY_MAH           13000.0f
#define MAX_C_RATING                1

#define CELL_POLARIZATION_REST_SEC  20
#define CELL_POLARIZATION_REST_MS   CELL_POLARIZATION_REST_SEC * MILLISECONDS_IN_SECOND

#define MAX_BRICK_VOLTAGE           4.28f
#define MAX_BRICK_WARNING_VOLTAGE   4.3f
#define MAX_BRICK_FAULT_VOLTAGE     4.33f

#define NOMINAL_BRICK_VOLTAGE       3.78f
#define MIN_BRICK_WARNING_VOLTAGE   3.1f
#define MIN_BRICK_FAULT_VOLTAGE     3.0f

#define MAX_BRICK_TEMP_WARNING_C    55.0f
#define MAX_BRICK_TEMP_FAULT_C      60.0f

// Structs

extern LookupTable_S cellMonTempTable;
extern LookupTable_S packMonTempTable;

#endif /* INC_PACKDATA_H_ */
