/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsPackMonitor.h"
#include <string.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* ADBMS Pack Monitor Register Addresses */

#define SRST        0x0027 // Software Reset
#define SNAP        0x002D // Freeze Result Registers
#define UNSNAP      0x002F // Unfreeze Result Registers
#define ADI1        0x0260 // Start I1ADC and VB1ADC
#define ADI2        0x0168 // Start I2ADC and VB2ADC
#define ADV         0x0430 // Start V1ADC and V2ADC
#define ADX         0x0530 // Start AUX ADC
#define CLRI        0x0711 // Clear IxADC and VBxADC Results
#define CLRA        0x0714 // Clear IxADC and VBXADC Accumulators
#define CLRVX       0x0712 // Clear all VxADC and AUX ADC Results
#define CLRO        0x0713 // Clear all OCxADC Results
#define CLRFLAG     0x0717 // Write 1 to Clear Flag Register Latches
#define RDFLAG      0x0032 // Read FLAG Register
#define RDSTAT      0x0034 // Read STAT Register
#define RDI         0x0004 // Read I1ADC and I2ADC Results
#define RDVB        0x0006 // Read VB1ADC and VB2ADC Results
#define RDIACC      0x0044 // Read I1ADC and I2ADC Accumulators
#define RDVBACC     0x0046 // Read VB1ADC and VB2ADC Accumulators
#define RDIVB1      0x0008 // Read I1ADC and VB1ADC Results
#define RDIVB1ACC   0x0048 // Read I1ADC and VB1ADC Accumulators
#define RDV1A       0x000A // Read V1ADC Results A
#define RDV1B       0x0009 // Read V1ADC Results B
#define RDV1C       0x0003 // Read V1ADC Results C
#define RDV1D       0x001B // Read V1ADC Results D and V2ADC Results
#define RDV2A       0x0007 // Read V2ADC Results A
#define RDV2B       0x000D // Read V2ADC Results B
#define RDV2C       0x0005 // Read V2ADC Results C
#define RDV2D       0x001F // Read V2ADC Results D and V1ADC Results
#define RDV2E       0x0025 // Read V2ADC Results
#define RDXA        0x0030 // Read AUX ADC Results A
#define RDXB        0x0031 // Read AUX ADC Results B
#define RDXC        0x0033 // Read AUX ADC Results C
#define RDOC        0x000B // Read OCxADC Results
#define RDSID       0x002C // Read Serial ID Register Group
#define RDCFGA      0x0002 // Read Configuration Register Group A
#define RDCFGB      0x0026 // Read Configuration Register Group B
#define RDCOMM      0x0722 // Read COMM Register Group
#define WRCFGA      0x0001 // Write Configuration Register Group A
#define WRCFGB      0x0024 // Write Configuration Register Group B
#define WRCOMM      0x0721 // Write COMM Register Group
#define STCOMM      0x0723 // Send COMM Register
#define RDALLI      0x000C // Read IxADC and VBxADC Results
#define RDALLA      0x004C // Read IxADC and VBxADC Accumulators
#define RDALLV      0x0035 // Read All External Input V1ADC Results and V2ADC V9, V10 Results
#define RDALLR      0x0011 // Read All External Input V2ADC Results and V1ADC V7, V8 Results
#define RDALLX      0x0051 // Read All AUX ADC Results
#define RDALLC      0x0010 // Read All Configuration, Flag, and Status Registers

/* END Pack Monitor Register Addresses */

#define VOLTAGE_16BIT_SIZE_BYTES    2
#define VOLTAGE_24BIT_SIZE_BYTES    3

// ADC Result Register Encoding
#define IADC1_GAIN_UV           1
#define IADC2_GAIN_UV           -1

#define VADC1_GAIN              0.0001f
#define VADC1_OFFSET            0.0f

#define VADC2_GAIN              -0.000085f
#define VADC2_OFFSET            0.0f

#define VREF2A_GAIN             0.00024f
#define VREF2A_OFFSET           0.0f

#define VREF2B_GAIN             -0.000204f
#define VREF2B_OFFSET           0.0f

#define VREF1P25_GAIN           0.0001f
#define VREF1P25_OFFSET         0.0f

#define VDIV_GAIN               0.0001f
#define VDIV_OFFSET             0.0f

#define VREG_GAIN               0.00024f
#define VREG_OFFSET             0.0f

#define VDD_GAIN                0.001f
#define VDD_OFFSET              0.0f

#define VDIG_GAIN               0.00024f
#define VDIG_OFFSET             0.0f

#define EPAD_GAIN               0.0001f
#define EPAD_OFFSET             0.0f

#define DIE_TEMP1_GAIN          0.01618f
#define DIE_TEMP1_OFFSET        -250.0f

#define DIE_TEMP2_GAIN          0.04878f
#define DIE_TEMP2_OFFSET        -267.0f

#define OVERCURRENT_GAIN1       5.0f
#define OVERCURRENT_GAIN2       2.5f

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum{
    AUX_GAIN = 0,
    AUX_OFFSET = 1,
    NUM_AUX_CONV_VAL = 2,
} AUX_CONV_E;

/* ==================================================================== */
/* ============================== MACROS ============================== */
/* ==================================================================== */

#define EXTRACT_16_BIT(buffer)      (((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define EXTRACT_24_BIT(buffer)      (((uint32_t)buffer[2] << (2 * BITS_IN_BYTE)) | ((uint32_t)buffer[1] << (1 * BITS_IN_BYTE)) | ((uint32_t)buffer[0]))

#define CONVERT_SIGNED_16_BIT_REGISTER(reg, gain, offset)    (((int16_t)(EXTRACT_16_BIT(reg)) * gain) + offset)

#define CONVERT_SIGNED_24_BIT_REGISTER_UV(reg, gain)         ((((int32_t)(EXTRACT_24_BIT(reg) << BITS_IN_BYTE)) /  BYTE_SIZE_DEC) * gain)

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static uint8_t transactionBuffer[EXTENDED_REGISTER_SIZE_BYTES];

static const float auxVoltageConv[NUM_AUX_VOLTAGES][NUM_AUX_CONV_VAL] = 
{
    {VREF2A_GAIN,       VREF2A_OFFSET},
    {VREF2B_GAIN,       VREF2B_OFFSET},
    {VREF1P25_GAIN,     VREF1P25_OFFSET},
    {DIE_TEMP1_GAIN,    DIE_TEMP1_OFFSET},
    {VREG_GAIN,         VREG_OFFSET},
    {VDD_GAIN,          VDD_OFFSET},
    {VDIG_GAIN,         VDIG_OFFSET},
    {EPAD_GAIN,         EPAD_OFFSET},
    {VDIV_GAIN,         VDIV_OFFSET},
    {DIE_TEMP2_GAIN,    DIE_TEMP2_OFFSET}
};

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E startAdcConversions(CHAIN_INFO_S* chainInfo, ADC_MODE_REDUNDANT_E redundantMode, ADC_MEASURE_OPTION_E measureOption)
{
    return commandChain((uint16_t)(ADI1 | redundantMode | measureOption), chainInfo);
}

TRANSACTION_STATUS_E startRedundantAdcConversions(CHAIN_INFO_S* chainInfo, ADC_MEASURE_OPTION_E measureOption)
{
    return commandChain((uint16_t)(ADI2 | measureOption), chainInfo);
}

TRANSACTION_STATUS_E startVoltageConversions(CHAIN_INFO_S* chainInfo, ADC_OPEN_WIRE_E openWire, VOLTAGE_ADC_CHANNEL_E channelSelect)
{
    return commandChain((uint16_t)(ADV | openWire | channelSelect), chainInfo);
}

TRANSACTION_STATUS_E startAuxVoltageConversions(CHAIN_INFO_S* chainInfo)
{
    return commandChain((uint16_t)(ADX), chainInfo);
}

TRANSACTION_STATUS_E clearPackMonitorVoltageRegisters(CHAIN_INFO_S* chainInfo)
{
    TRANSACTION_STATUS_E status = commandChain(CLRI, chainInfo);
    if(status != TRANSACTION_SUCCESS)
    {
        return status;
    }

    status = commandChain(CLRVX, chainInfo);
    if(status != TRANSACTION_SUCCESS)
    {
        return status;
    }

    return commandChain(CLRO, chainInfo);
}

TRANSACTION_STATUS_E clearAccumulators(CHAIN_INFO_S* chainInfo)
{
    return commandChain(CLRA, chainInfo);
}

TRANSACTION_STATUS_E clearPackMonitorFlags(CHAIN_INFO_S* chainInfo)
{
    memset(transactionBuffer, 0xFF, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = writeChain(CLRFLAG, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    activatePort(chainInfo, TIME_READY_US);

    return status;
}

TRANSACTION_STATUS_E readFlagRegister(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDFLAG, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    memcpy(&packMonitor->flagGroup, transactionBuffer, REGISTER_SIZE_BYTES);
    packMonitor->flagGroup.conversionCounter1 = (((uint16_t)(transactionBuffer[REGISTER_BYTE2] & COUNTER1_MASK)) << BITS_IN_BYTE) | ((uint16_t)(transactionBuffer[REGISTER_BYTE3]));
    packMonitor->flagGroup.conversionCounter2 = transactionBuffer[REGISTER_BYTE2] >> COUNTER2_BIT;

    return status;
}

TRANSACTION_STATUS_E readStatRegister(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSTAT, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    memcpy(&packMonitor->statGroup, transactionBuffer, REGISTER_SIZE_BYTES);

    return status;
}

TRANSACTION_STATUS_E readCurrentAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDI, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->currentAdc1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, IADC1_GAIN_UV);
    packMonitor->currentAdc2_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + VOLTAGE_24BIT_SIZE_BYTES), IADC1_GAIN_UV);

    return status;
}

TRANSACTION_STATUS_E readBatteryVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDVB, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->batteryVoltage1 = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + VOLTAGE_16BIT_SIZE_BYTES), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->batteryVoltage2 = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES* 2)), VADC2_GAIN, VADC2_OFFSET);

    return status;
}

TRANSACTION_STATUS_E readCurrentAccumulators(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDIACC, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->currentAdcAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, IADC1_GAIN_UV);
    packMonitor->currentAdcAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + VOLTAGE_24BIT_SIZE_BYTES), IADC2_GAIN_UV);

    return status;       
}

TRANSACTION_STATUS_E readBatteryVoltageAccumulators(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDVBACC, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->batteryVoltageAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, VADC1_GAIN);
    packMonitor->batteryVoltageAccumulator2_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + VOLTAGE_24BIT_SIZE_BYTES), VADC2_GAIN);

    return status;
}

TRANSACTION_STATUS_E readPrimaryAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDIVB1, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->currentAdc1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, IADC1_GAIN_UV);
    packMonitor->batteryVoltage1 = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + 4), VADC1_GAIN, VADC1_OFFSET); // TODO: create #define for 4

    return status;
}

TRANSACTION_STATUS_E readPrimaryAccumulators(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDIVB1ACC, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->currentAdcAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, IADC1_GAIN_UV);
    packMonitor->batteryVoltageAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + VOLTAGE_24BIT_SIZE_BYTES), VADC1_GAIN);

    return status;
}

TRANSACTION_STATUS_E readVoltage1B(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDV1B, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    packMonitor->voltageAdc[3] = CONVERT_SIGNED_16_BIT_REGISTER(transactionBuffer, VADC1_GAIN, VADC1_OFFSET);
    packMonitor->voltageAdc[4] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + VOLTAGE_16BIT_SIZE_BYTES), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->voltageAdc[5] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 2)), VADC1_GAIN, VADC1_OFFSET);

    return status;
}

TRANSACTION_STATUS_E readOvercurrentRegister(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDOC, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    float oc1Gain = (packMonitor->configGroupB.oc1GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    float oc2Gain = (packMonitor->configGroupB.oc2GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    float oc3Gain = (packMonitor->configGroupB.oc3GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);

    packMonitor->overcurrentStatusGroup.overcurrentAdc1 = transactionBuffer[REGISTER_BYTE0] * oc1Gain;
    packMonitor->overcurrentStatusGroup.overcurrentAdc2 = transactionBuffer[REGISTER_BYTE1] * oc2Gain;
    packMonitor->overcurrentStatusGroup.overcurrentAdc3 = transactionBuffer[REGISTER_BYTE2] * oc3Gain;
    packMonitor->overcurrentStatusGroup.overcurrentAdc3Max = transactionBuffer[REGISTER_BYTE4] * oc3Gain;
    packMonitor->overcurrentStatusGroup.overcurrentAdc3Min = transactionBuffer[REGISTER_BYTE5] * oc3Gain;

    return status;
}

TRANSACTION_STATUS_E readPackMonitorSerialId(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDSID, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    memcpy(packMonitor->serialId, transactionBuffer, REGISTER_SIZE_BYTES);

    return status;
}

TRANSACTION_STATUS_E readPackMonitorConfigA(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDCFGA, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    memcpy(&packMonitor->configGroupA, transactionBuffer, REGISTER_SIZE_BYTES);

    return status;
}

TRANSACTION_STATUS_E readPackMonitorConfigB(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDCFGB, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);

    memcpy(&packMonitor->configGroupB, transactionBuffer, REGISTER_SIZE_BYTES);

    return status;
}

TRANSACTION_STATUS_E writePackMonitorConfigA(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    memcpy(transactionBuffer, &packMonitor->configGroupA, REGISTER_SIZE_BYTES);

    return writeChain(WRCFGA, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);
}

TRANSACTION_STATUS_E writePackMonitorConfigB(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, REGISTER_SIZE_BYTES);

    memcpy(transactionBuffer, &packMonitor->configGroupB, REGISTER_SIZE_BYTES);

    return writeChain(WRCFGB, chainInfo, transactionBuffer, REGISTER_SIZE_BYTES);
}

TRANSACTION_STATUS_E readAllCurrentAndBatteryVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, EXTENDED_REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDALLI, chainInfo, transactionBuffer, EXTENDED_REGISTER_SIZE_BYTES);

    packMonitor->currentAdc1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, IADC1_GAIN_UV);
    packMonitor->currentAdc2_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + VOLTAGE_24BIT_SIZE_BYTES), IADC2_GAIN_UV);
    
    packMonitor->batteryVoltage1 = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_24BIT_SIZE_BYTES * 2)), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->batteryVoltage2 = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_24BIT_SIZE_BYTES * 2 + VOLTAGE_16BIT_SIZE_BYTES)), VADC2_GAIN, VADC2_OFFSET);

    float oc1Gain = (packMonitor->configGroupB.oc1GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    float oc2Gain = (packMonitor->configGroupB.oc2GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    float oc3Gain = (packMonitor->configGroupB.oc3GainControl) ? (OVERCURRENT_GAIN2) : (OVERCURRENT_GAIN1);
    packMonitor->overcurrentStatusGroup.overcurrentAdc1 = transactionBuffer[(REGISTER_BYTE0 + 10)] * oc1Gain;
    packMonitor->overcurrentStatusGroup.overcurrentAdc2 = transactionBuffer[(REGISTER_BYTE1 + 10)] * oc2Gain;
    packMonitor->overcurrentStatusGroup.overcurrentAdc3 = transactionBuffer[(REGISTER_BYTE2 + 10)] * oc3Gain;

    memcpy(((&packMonitor->statGroup) + REGISTER_BYTE3), (transactionBuffer + 13), 1);

    memcpy(&packMonitor->flagGroup, (transactionBuffer + 14), REGISTER_SIZE_BYTES);

    return status;
}

TRANSACTION_STATUS_E readAllAccumulationRegisters(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, EXTENDED_REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDALLA, chainInfo, transactionBuffer, EXTENDED_REGISTER_SIZE_BYTES);

    packMonitor->currentAdcAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV(transactionBuffer, IADC1_GAIN_UV);
    packMonitor->currentAdcAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + VOLTAGE_24BIT_SIZE_BYTES), IADC2_GAIN_UV);
    packMonitor->batteryVoltageAccumulator1_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + (VOLTAGE_24BIT_SIZE_BYTES * 2)), VADC1_GAIN);
    packMonitor->batteryVoltageAccumulator2_uV = CONVERT_SIGNED_24_BIT_REGISTER_UV((transactionBuffer + (VOLTAGE_24BIT_SIZE_BYTES * 3)), VADC2_GAIN);

    memcpy(((&packMonitor->statGroup) + REGISTER_BYTE3), (transactionBuffer + 12), 2);

    memcpy(&packMonitor->flagGroup, (transactionBuffer + 14), REGISTER_SIZE_BYTES);

    return status;
}

TRANSACTION_STATUS_E readVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, EXTENDED_REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDALLV, chainInfo, transactionBuffer, EXTENDED_REGISTER_SIZE_BYTES);

    for(uint8_t i = 0; i < NUM_RD_VOLTAGE_ADC; i++)
    {
        packMonitor->voltageAdc[i] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * i)), VADC1_GAIN, VADC1_OFFSET);
    }

    packMonitor->voltageAdc[6] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 6)), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->voltageAdc[7] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 7)), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->voltageAdc[8] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 8)), VADC2_GAIN, VADC2_OFFSET);
    packMonitor->voltageAdc[9] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 9)), VADC2_GAIN, VADC2_OFFSET);

    return status;
}

TRANSACTION_STATUS_E readSecondaryVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, EXTENDED_REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDALLR, chainInfo, transactionBuffer, EXTENDED_REGISTER_SIZE_BYTES);

    for(uint8_t i = 0; i < NUM_RD_VOLTAGE_ADC; i++)
    {
        packMonitor->redundantVoltageAdc[i] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * i)), VADC2_GAIN, VADC2_OFFSET);
    }

    packMonitor->voltageAdc[6] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 6)), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->voltageAdc[7] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 7)), VADC1_GAIN, VADC1_OFFSET);
    packMonitor->voltageAdc[8] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 8)), VADC2_GAIN, VADC2_OFFSET);
    packMonitor->voltageAdc[9] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * 9)), VADC2_GAIN, VADC2_OFFSET);

    return status;
}

TRANSACTION_STATUS_E readAuxiliaryVoltages(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, EXTENDED_REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDALLX, chainInfo, transactionBuffer, EXTENDED_REGISTER_SIZE_BYTES);

    float *auxVoltageAdc[NUM_AUX_VOLTAGES] = 
    {
        &packMonitor->referenceVoltage,
        &packMonitor->redundantReferenceVoltage,
        &packMonitor->referenceVoltage1P25,
        &packMonitor->primaryIntTemp,
        &packMonitor->vregPowerSupply,
        &packMonitor->vddPowerSupply,
        &packMonitor->digitalSupply,
        &packMonitor->exposedPadVoltage,
        &packMonitor->dividedReferenceVoltage,
        &packMonitor->secondaryIntTemp
    };

    for(uint8_t i = 0; i < NUM_AUX_VOLTAGES; i++)
    {
        *auxVoltageAdc[i] = CONVERT_SIGNED_16_BIT_REGISTER((transactionBuffer + (VOLTAGE_16BIT_SIZE_BYTES * i)), auxVoltageConv[i][AUX_GAIN], auxVoltageConv[i][AUX_OFFSET]);
    }

    return status;
}

TRANSACTION_STATUS_E readAllConfigFlagStatus(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor)
{
    memset(transactionBuffer, 0x00, EXTENDED_REGISTER_SIZE_BYTES);

    TRANSACTION_STATUS_E status = readChain(RDALLC, chainInfo, transactionBuffer, EXTENDED_REGISTER_SIZE_BYTES);

    memcpy(&packMonitor->configGroupA, transactionBuffer, REGISTER_SIZE_BYTES);
    memcpy(&packMonitor->configGroupB, transactionBuffer, REGISTER_SIZE_BYTES);
    memcpy((&packMonitor->statGroup + REGISTER_BYTE3), (transactionBuffer + 11), 2);
    memcpy(&packMonitor->flagGroup, (transactionBuffer + 14), REGISTER_SIZE_BYTES);
    packMonitor->flagGroup.conversionCounter1 = (((uint16_t)((transactionBuffer[REGISTER_BYTE2] + 14) & COUNTER1_MASK)) << BITS_IN_BYTE) | ((uint16_t)(transactionBuffer[REGISTER_BYTE3] + 14));
    packMonitor->flagGroup.conversionCounter2 = (transactionBuffer[REGISTER_BYTE2] + 14) >> COUNTER2_BIT;
 
    return status;
}
