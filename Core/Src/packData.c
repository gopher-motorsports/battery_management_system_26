/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "packData.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define TEMP_SCALE      0.05f
#define TEMP_OFFSET     0.0f

#define CELL_MON_TEMP_LUT_LENGTH        57
#define PACK_MON_TEMP_LUT_LENGTH        24
#define VOLT_LUT_LENGTH                 14

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

const float cellTemp[CELL_MON_TEMP_LUT_LENGTH] = {192.476, 150.979, 129.608, 115.467, 104.982, 96.679, 89.815, 83.964, 78.861, 74.331, 70.253, 66.539, 63.122, 59.954, 56.994, 54.212, 51.582, 49.084, 46.700, 44.417, 42.221, 40.102, 38.051, 36.059, 34.120, 32.226, 30.373, 28.554, 26.764, 25.000, 23.256, 21.529, 19.814, 18.108, 16.406, 14.705, 13.000, 11.287, 9.562, 7.821, 6.057, 4.266, 2.441, 0.575, -1.341, -3.315, -5.359, -7.487, -9.715, -12.066, -14.566, -17.252, -20.174, -23.405, -27.054, -31.300, -36.472, -43.279, -53.870};

const float shuntTemp[PACK_MON_TEMP_LUT_LENGTH] = {141.141, 106.872, 88.574, 76.083, 66.539, 58.747, 52.097, 46.236, 40.941, 36.059, 31.480, 27.120, 22.910, 18.790, 14.705, 10.599, 6.412, 2.071, -2.517, -7.487, -13.046, -19.568, -27.848, -40.281};

const float prechargeDischargeTemp[PACK_MON_TEMP_LUT_LENGTH] = {138.573, 105.208, 87.343, 75.128, 65.783, 58.146, 51.624, 45.872, 40.672, 35.876, 31.374, 27.086, 22.943, 18.887, 14.864, 10.819, 6.691, 2.410, -2.117, -7.022, -12.513, -18.959, -27.150, -39.462};

const int32_t shuntResistance_nOhm[VOLT_LUT_LENGTH] = {84400, 83100, 82200, 81300, 81000, 80100, 80400, 80000, 79500, 79600, 79100, 78900, 78700, 78600};

const LookupTable_S cellTempTable = {
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = cellTemp,
    .size = CELL_MON_TEMP_LUT_LENGTH
};

const LookupTable_S shuntTempTable = { 
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = shuntTemp,
    .size = PACK_MON_TEMP_LUT_LENGTH
};

const LookupTable_S prechargeDischargeTempTable = { 
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = prechargeDischargeTemp,
    .size = PACK_MON_TEMP_LUT_LENGTH
};

const LookupTable_S shuntResistanceTable = {
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = shuntResistance_nOhm,
    .size = VOLT_LUT_LENGTH
};
