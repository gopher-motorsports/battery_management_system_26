#ifndef INC_ADBMS_PACK_MONITOR_H
#define INC_ADBMS_PACK_MONITOR_H

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbmsSpi.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    BASIC_REF_SGND = 0,
    BASIC_REF_1_25V
} BASIC_REFERENCE_VOLTAGE_SETTING_E;

typedef enum
{
    ADVANCED_REF_SGND = 0,
    ADVANCED_REF_1_25V,
    ADVANCED_REF_V3,
    ADVANCED_REF_V4
} ADVANCED_REFERENCE_VOLTAGE_SETTING_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct __attribute__((packed))
{
    // Byte 0
    ADVANCED_REFERENCE_VOLTAGE_SETTING_E v1Reference : 2;
    ADVANCED_REFERENCE_VOLTAGE_SETTING_E v2Reference : 2;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v3Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v4Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v5Reference : 1;
    uint8_t overcurrentAdcsEnabled : 1;

    // Byte 1
    // TODO: What is the clock monitor diagnostic enable?
    // TODO: What is the supply monitor and deglitcher diagnostic enable?

    // Byte 2


}

#endif /* INC_ADBMS_PACK_MONITOR_H */
