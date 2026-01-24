/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "gcanUtils.h"

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

// const FLOAT_CAN_STRUCT *cellVoltageParams[NUM_CELL_MON][NUM_CELLS_PER_CELL_MONITOR] =
// {
//     {&segment1Cell1Voltage_V, &segment1Cell2Voltage_V, &segment1Cell3Voltage_V, &segment1Cell4Voltage_V, &segment1Cell5Voltage_V, &segment1Cell6Voltage_V, &segment1Cell7Voltage_V, &segment1Cell8Voltage_V, &segment1Cell9Voltage_V, &segment1Cell10Voltage_V, &segment1Cell11Voltage_V, &segment1Cell12Voltage_V, &segment1Cell13Voltage_V, &segment1Cell14Voltage_V},
//     {&segment2Cell1Voltage_V, &segment2Cell2Voltage_V, &segment2Cell3Voltage_V, &segment2Cell4Voltage_V, &segment2Cell5Voltage_V, &segment2Cell6Voltage_V, &segment2Cell7Voltage_V, &segment2Cell8Voltage_V, &segment2Cell9Voltage_V, &segment2Cell10Voltage_V, &segment2Cell11Voltage_V, &segment2Cell12Voltage_V, &segment2Cell13Voltage_V, &segment2Cell14Voltage_V},
//     {&segment3Cell1Voltage_V, &segment3Cell2Voltage_V, &segment3Cell3Voltage_V, &segment3Cell4Voltage_V, &segment3Cell5Voltage_V, &segment3Cell6Voltage_V, &segment3Cell7Voltage_V, &segment3Cell8Voltage_V, &segment3Cell9Voltage_V, &segment3Cell10Voltage_V, &segment3Cell11Voltage_V, &segment3Cell12Voltage_V, &segment3Cell13Voltage_V, &segment3Cell14Voltage_V},
//     {&segment4Cell1Voltage_V, &segment4Cell2Voltage_V, &segment4Cell3Voltage_V, &segment4Cell4Voltage_V, &segment4Cell5Voltage_V, &segment4Cell6Voltage_V, &segment4Cell7Voltage_V, &segment4Cell8Voltage_V, &segment4Cell9Voltage_V, &segment4Cell10Voltage_V, &segment4Cell11Voltage_V, &segment4Cell12Voltage_V, &segment4Cell13Voltage_V, &segment4Cell14Voltage_V},
//     {&segment5Cell1Voltage_V, &segment5Cell2Voltage_V, &segment5Cell3Voltage_V, &segment5Cell4Voltage_V, &segment5Cell5Voltage_V, &segment5Cell6Voltage_V, &segment5Cell7Voltage_V, &segment5Cell8Voltage_V, &segment5Cell9Voltage_V, &segment5Cell10Voltage_V, &segment5Cell11Voltage_V, &segment5Cell12Voltage_V, &segment5Cell13Voltage_V, &segment5Cell14Voltage_V},
//     {&segment6Cell1Voltage_V, &segment6Cell2Voltage_V, &segment6Cell3Voltage_V, &segment6Cell4Voltage_V, &segment6Cell5Voltage_V, &segment6Cell6Voltage_V, &segment6Cell7Voltage_V, &segment6Cell8Voltage_V, &segment6Cell9Voltage_V, &segment6Cell10Voltage_V, &segment6Cell11Voltage_V, &segment6Cell12Voltage_V, &segment6Cell13Voltage_V, &segment6Cell14Voltage_V},
//     {&segment7Cell1Voltage_V, &segment7Cell2Voltage_V, &segment7Cell3Voltage_V, &segment7Cell4Voltage_V, &segment7Cell5Voltage_V, &segment7Cell6Voltage_V, &segment7Cell7Voltage_V, &segment7Cell8Voltage_V, &segment7Cell9Voltage_V, &segment7Cell10Voltage_V, &segment7Cell11Voltage_V, &segment7Cell12Voltage_V, &segment7Cell13Voltage_V, &segment7Cell14Voltage_V},
//     {&segment8Cell1Voltage_V, &segment8Cell2Voltage_V, &segment8Cell3Voltage_V, &segment8Cell4Voltage_V, &segment8Cell5Voltage_V, &segment8Cell6Voltage_V, &segment8Cell7Voltage_V, &segment8Cell8Voltage_V, &segment8Cell9Voltage_V, &segment8Cell10Voltage_V, &segment8Cell11Voltage_V, &segment8Cell12Voltage_V, &segment8Cell13Voltage_V, &segment8Cell14Voltage_V},
//     {&segment9Cell1Voltage_V, &segment9Cell2Voltage_V, &segment9Cell3Voltage_V, &segment9Cell4Voltage_V, &segment9Cell5Voltage_V, &segment9Cell6Voltage_V, &segment9Cell7Voltage_V, &segment9Cell8Voltage_V, &segment9Cell9Voltage_V, &segment9Cell10Voltage_V, &segment9Cell11Voltage_V, &segment9Cell12Voltage_V, &segment9Cell13Voltage_V, &segment9Cell14Voltage_V},
//     {&segment10Cell1Voltage_V, &segment10Cell2Voltage_V, &segment10Cell3Voltage_V, &segment10Cell4Voltage_V, &segment10Cell5Voltage_V, &segment10Cell6Voltage_V, &segment10Cell7Voltage_V, &segment10Cell8Voltage_V, &segment10Cell9Voltage_V, &segment10Cell10Voltage_V, &segment10Cell11Voltage_V, &segment10Cell12Voltage_V, &segment10Cell13Voltage_V, &segment10Cell14Voltage_V}
    
// };