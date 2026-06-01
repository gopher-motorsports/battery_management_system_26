#ifndef INC_TASK_STATISTICS_H_
#define INC_TASK_STATISTICS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "updateCellMonitorTask.h"

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void updateBatteryStatistics(cellMonitorTaskData_S *taskData);

/*!
    @brief Sorts an array of values into ascending order, note that the array will be modified
    @param array - Array of floating point values to sort
    @param n - Size of the array
*/
void sort(float *array, uint32_t n);

#endif /* INC_TASK_STATISTICS_H_ */
