/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "packData.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define TEMP_SCALE      0.05f
#define TEMP_OFFSET     0.0f
#define TEMP_LUT_LENGTH 24
#define VOLT_LUT_LENGTH 14

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

const float prechargeDischargeTemp[TEMP_LUT_LENGTH] = {138.573, 105.208, 87.343, 75.128, 65.783, 58.146, 51.624, 45.872, 40.672, 35.876, 31.374, 27.086, 22.943, 18.887, 14.864, 10.819, 6.691, 2.410, -2.117, -7.022, -12.513, -18.959, -27.150, -39.462};

const float shuntTemp[TEMP_LUT_LENGTH] = {141.141, 106.872, 88.574, 76.083, 66.539, 58.747, 52.097, 46.236, 40.941, 36.059, 31.480, 27.120, 22.910, 18.790, 14.705, 10.599, 6.412, 2.071, -2.517, -7.487, -13.046, -19.568, -27.848, -40.281};

const int32_t shuntVoltage[VOLT_LUT_LENGTH] = {-844, -831, -822, -813, -810, -801, -804, -800, -795, -796, -791, -789, -787, -786};

const LookupTable_S prechargeDischangeTempTable = { 
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = prechargeDischargeTemp,
    .size = TEMP_LUT_LENGTH
};

const LookupTable_S shuntTempTable = {
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = shuntTemp,
    .size = TEMP_LUT_LENGTH
};

const LookupTable_S shuntVoltageTable = {
    .xScale = TEMP_SCALE,
    .xOffset = TEMP_OFFSET,
    .y = shuntVoltage,
    .size = VOLT_LUT_LENGTH
};
