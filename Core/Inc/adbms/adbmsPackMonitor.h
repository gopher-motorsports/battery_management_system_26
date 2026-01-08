#ifndef INC_ADBMS_PACK_MONITOR_H
#define INC_ADBMS_PACK_MONITOR_H

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbmsSpi.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define REGISTER_BYTE0 0
#define REGISTER_BYTE1 1
#define REGISTER_BYTE2 2
#define REGISTER_BYTE3 3
#define REGISTER_BYTE4 4
#define REGISTER_BYTE5 5

#define COUNTER1_MASK   0x1F
#define COUNTER2_BIT    5

#define NUM_VOLTAGE_ADC     10
#define NUM_RD_VOLTAGE_ADC  6
#define NUM_AUX_VOLTAGES    10

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

typedef enum
{
    REGULAR_CLOCK_DIAGNOSTIC_MODE = 0,
    FASTER_CLOCK_DIAGNOSTIC_MODE,
    CLOCK_STUCK_HIGH_DIAGNOSTIC_MODE,
    CLOCK_STUCK_LOW_DIAGNOSTIC_MODE
} CLOCK_MONITOR_DIAGNOSTIC_SETTING_E;

typedef enum
{
    REGULAR_SUPPLY_DIAGNOSTIC_MODE = 0,
    FORCE_MISMATCH_DIAGNOSTIC_MODE,
    FORCE_UNDERVOLTAGE_DIAGNOSTIC_MODE,
    FORCE_OVERVOLTAGE_DIAGNOSTIC_MODE
} SUPPLY_MONITOR_DIAGNOSTIC_SETTING_E;

typedef enum
{
    VOLTAGE_SOAK_TIME_DISABLED = 0,
    VOLTAGE_SOAK_TIME_100_US,
    VOLTAGE_SOAK_TIME_500_US,
    VOLTAGE_SOAK_TIME_1_MS,
    VOLTAGE_SOAK_TIME_2_MS,
    VOLTAGE_SOAK_TIME_10_MS,
    VOLTAGE_SOAK_TIME_20_MS,
    VOLTAGE_SOAK_TIME_150_MS
} VOLTAGE_SOAK_TIME_E;

typedef enum
{
    ACCUMULATE_4_SAMPLES = 0,
    ACCUMULATE_8_SAMPLES,
    ACCUMULATE_12_SAMPLES,
    ACCUMULATE_16_SAMPLES,
    ACCUMULATE_20_SAMPLES,
    ACCUMULATE_24_SAMPLES,
    ACCUMULATE_28_SAMPLES,
    ACCUMULATE_32_SAMPLES
} ACCUMULATION_COUNTER_SETTING_E;

typedef enum
{
    DEGLITCH_DISABLED = 0,
    DEGLITCH_2_OUT_OF_3,
    DEGLITCH_4_OUT_OF_8,
    DEGLITCH_7_OUT_OF_8
} OVERCURRENT_DEGLITCH_SETTING_E;

typedef enum
{
    OVERCURRENT_GAIN_5_mV = 0,
    OVERCURRENT_GAIN_2_5_mV
} OVERCURRENT_GAIN_CONTROL_SETTING_E;

typedef enum
{
    OVERCURRENT_OUTPUTS_DISABLED = 0,
    OVERCURRENT_OUTPUTS_PWM1,
    OVERCURRENT_OUTPUTS_PWM2,
    OVERCURRENT_OUTPUTS_STATIC
} OVERCURRENT_OUTPUT_SETTING_E;

typedef enum
{
    NO_INJECTION_CONVERT_REGULAR = 0,
    INJECT_IxA_VBATT,
    INJECT_IxB_SGND,
    INJECT_SxA_CONVERT_SxA_VS_IxA,
    CONVERT_SxA_VS_IxA_SGND_VS_SGND,
    CONVERT_VDIV,
    CONVERT_SCALED_VREF,
    INJECT_IxB_CONVERT_SxA_VS_IxA
} PACK_ADC_DIAGNOSTIC_SETTING_E; 

typedef enum
{
    NON_REDUNDANT_MODE = 0,
    REDUNDANT_MODE = (1 << 8)
} ADC_MODE_REDUNDANT_E;

typedef enum
{
    SINGLE_SHOT_MEASUREMENT = 0,
    SINGLE_SHOT_DIAGNOSTIC_MEASUREMENT = (1 << 4) | (1),
    CONTINUOUS_MEASUREMENT = (1 << 7),
    CONTINUOUS_DIAGNOSTIC_MEASURMENT = (1 << 7) | (1),
} ADC_MEASURE_OPTION_E;

typedef enum
{
    OPEN_WIRE_DISABLED = 0,
    OPEN_WIRE_POSITIVE = (1 << 6),
    OPEN_WIRE_NEGATIVE = (1 << 7)
} ADC_OPEN_WIRE_E;

typedef enum
{
    PACK_V1_ONLY = 0,
    PACK_V2_ONLY,
    PACK_V3_ONLY,
    PACK_V4_ONLY,
    PACK_V5_ONLY,
    PACK_V6_ONLY,
    PACK_V7_V9,
    PACK_V8_V10,
    PACK_VREF2_ONLY,
    PACK_ALL_CHANNELS,
    PACK_V2_V4_V6,
    PACK_V1_TO_V6,
    PACK_V1_TO_V4,
    PACK_V1_V3_V5,
    PACK_V3_TO_V6,
    PACK_V4_TO_V6
} VOLTAGE_ADC_CHANNEL_E;

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
    CLOCK_MONITOR_DIAGNOSTIC_SETTING_E clockDiagnosticMode : 2;
    SUPPLY_MONITOR_DIAGNOSTIC_SETTING_E supplyDiagnosticMode : 2;
    uint8_t assertThermalShutdownDetectedFault : 1;
    uint8_t reserved1 : 1;
    uint8_t assertTrimError : 1;
    uint8_t assertTestModeDetectedFault : 1;

    // Byte 2
    BASIC_REFERENCE_VOLTAGE_SETTING_E v6Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v7Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v8Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v9Reference : 1;
    BASIC_REFERENCE_VOLTAGE_SETTING_E v10Reference : 1;
    VOLTAGE_SOAK_TIME_E soakTime : 3;

    // Byte 3
    uint8_t gpo1State : 1;
    uint8_t gpo2State : 1;
    uint8_t gpo3State : 1;
    uint8_t gpo4State : 1;
    uint8_t gpo5State : 1;
    uint8_t gpo6State : 2;
    uint8_t reserved2 : 1;

    // Byte 4
    uint8_t gpo1HighZMode : 1;
    uint8_t gpo2HighZMode : 1;
    uint8_t gpo3HighZMode : 1;
    uint8_t gpo4HighZMode : 1;
    uint8_t gpo5HighZMode : 1;
    uint8_t gpo6HighZMode : 1;
    uint8_t gpio1FaultOutputEnable : 1;
    uint8_t spiModeSelect : 1;

    // Byte 5
    ACCUMULATION_COUNTER_SETTING_E accumulationSetting : 3;
    uint8_t commBreak : 1;
    uint8_t referenceOn : 1;
    uint8_t snapStatus : 1;
    uint8_t packVoltage1DifferentialMode : 1;
    uint8_t packVoltage2DifferentialMode : 1; 

} ADBMS_ConfigAPackMonitor;

typedef struct __attribute__((packed))
{
    // Byte 0
    uint8_t oc1Threshold;

    // Byte 1
    uint8_t oc2Threshold;

    // Byte 2
    uint8_t oc3Threshold;

    // Byte 3
    OVERCURRENT_DEGLITCH_SETTING_E ocDeglitchMode : 2;
    uint8_t reserved1 : 1;
    uint8_t ocReducedSafetyInterval : 1;
    uint8_t reserved2 : 4;

    // Byte 4
    uint8_t ocPinOpenDrainMode: 1;
    OVERCURRENT_GAIN_CONTROL_SETTING_E oc1GainControl : 1;
    OVERCURRENT_GAIN_CONTROL_SETTING_E oc2GainControl : 1;
    OVERCURRENT_GAIN_CONTROL_SETTING_E oc3GainControl : 1;
    OVERCURRENT_OUTPUT_SETTING_E ocOutputMode : 2;
    uint8_t ocAOutputInverted : 1;
    uint8_t ocBOutputInverted : 1;

    // Byte 5
    PACK_ADC_DIAGNOSTIC_SETTING_E adcDiagnosticMode : 3;
    uint8_t gpio2ConversionSignalEnable: 1;
    uint8_t gpio1State : 1;
    uint8_t gpio2State : 1;
    uint8_t gpio3State : 1;
    uint8_t gpio4State : 1;

} ADBMS_ConfigBPackMonitor;

typedef struct __attribute__((packed))
{
    // Byte 0
    uint8_t overcurrent1Fault : 1;
    uint8_t overcurrentVoterAFault : 1;
    uint8_t overcurrentAGateDrainFault : 1;
    uint8_t overcurrent3Fault : 1;
    uint8_t overcurrentMismatchFault : 1;
    uint8_t driveUnderVoltage : 1;
    uint8_t reserved1 : 2;

    // Byte 1
    uint8_t overcurrent2Fault : 1;
    uint8_t overcurrentVoterBFault : 1;
    uint8_t overcurrentBGateDrainFault : 1;
    uint8_t overcurrentReferenceFault : 1;
    uint8_t oscillatorStuckFault : 1;
    uint8_t supplyUnderVoltage : 1;
    uint8_t reserved2 : 2;

    // Bytes 2 and 3
    uint16_t conversionCounter1 : 13; 
    uint16_t conversionCounter2 : 3;

    // Byte 4
    uint8_t redundantAdcMultipleTrimError : 1; // Why is it called trim instead of error-correction code?
    uint8_t redundantAdcTrimError : 1;
    uint8_t primaryAdcMultipleTrimError : 1;
    uint8_t primaryAdcTrimError : 1;
    uint8_t digitalSupplyUnderVoltage : 1;
    uint8_t digitalSupplyOverVoltage : 1;
    uint8_t regulatorUnderVoltage : 1;
    uint8_t regulatorOverVoltage : 1;

    // Byte 5
    uint8_t oscillatorMismatchFault : 1;
    uint8_t testModeDetected : 1;
    uint8_t thermalShutdownDetected : 1;
    uint8_t resetDetected : 1;
    uint8_t spiFault : 1;
    uint8_t reserved3 : 1;
    uint8_t voltageDomainFault : 1;
    uint8_t voltageDomainDiagnositcFault : 1;

} ADBMS_FlagRegister;

typedef struct __attribute__((packed))
{
    // Byte 0
    uint8_t overcurrentPinAState : 1;
    uint8_t overcurrentPinBState : 1;
    uint8_t reserved1 : 6;

    // Byte 1
    uint8_t derivativeCode : 2;
    uint8_t reserved2 : 4;
    uint8_t currentAdc1Initialized : 1;
    uint8_t currentAdc2Initialized : 1;

    // Byte 2
    uint8_t reserved3 : 8;

    // Byte 3
    uint8_t gpo5LowLevelState : 1;
    uint8_t gpo6LowLevelState : 1;
    uint8_t gpo1HighLevelState : 1;
    uint8_t gpo2HighLevelState : 1;
    uint8_t gpo3HighLevelState : 1;
    uint8_t gpo4HighLevelState : 1;
    uint8_t gpo5HighLevelState : 1;
    uint8_t gpo6HighLevelState : 1;

    // Byte 4
    uint8_t gpio1State : 1;
    uint8_t gpio2State : 1;
    uint8_t gpio3State : 1;
    uint8_t gpio4State : 1;
    uint8_t gpo1LowLevelState : 1;
    uint8_t gpo2LowLevelState : 1;
    uint8_t gpo3LowLevelState : 1;
    uint8_t gpo4LowLevelState : 1;

    // Byte 5
    uint8_t reserved4 : 4;
    uint8_t revisionId : 4;

} ADBMS_StatRegister;

typedef struct __attribute__((packed))
{
    float overcurrentAdc1;
    float overcurrentAdc2;
    float overcurrentAdc3;
    float overcurrentAdc3Max;
    float overcurrentAdc3Min;

} ADBMS_OvercurrentStatus;

typedef struct __attribute__((packed))
{
    ADBMS_ConfigAPackMonitor configGroupA;
    ADBMS_ConfigBPackMonitor configGroupB;

    ADBMS_FlagRegister flagGroup;
    ADBMS_StatRegister statGroup;

    int32_t currentAdc1_uV;
    int32_t currentAdc2_uV;

    float batteryVoltage1;
    float batteryVoltage2;

    int32_t currentAdcAccumulator1_uV;
    int32_t currentAdcAccumulator2_uV;
    
    int32_t batteryVoltageAccumulator1_uV;
    int32_t batteryVoltageAccumulator2_uV;

    float voltageAdc[NUM_VOLTAGE_ADC];
    float redundantVoltageAdc[NUM_RD_VOLTAGE_ADC];

    float referenceVoltage;
    float redundantReferenceVoltage;
    float referenceVoltage1P25;
    float primaryIntTemp;
    float vregPowerSupply;
    float vddPowerSupply;
    float digitalSupply;
    float exposedPadVoltage;
    float dividedReferenceVoltage;
    float secondaryIntTemp;

    ADBMS_OvercurrentStatus overcurrentStatusGroup;

    uint8_t serialId[REGISTER_SIZE_BYTES];

    uint32_t convCountTimer;

} ADBMS_PackMonitorData;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

TRANSACTION_STATUS_E softReset(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E freezeRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E unfreezeRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E startAdcConversions(CHAIN_INFO_S* chainInfo, ADC_MODE_REDUNDANT_E redundantMode, ADC_MEASURE_OPTION_E measureOption);

TRANSACTION_STATUS_E startRedundantAdcConversions(CHAIN_INFO_S* chainInfo, ADC_MEASURE_OPTION_E measureOption);

TRANSACTION_STATUS_E startVoltageConversions(CHAIN_INFO_S* chainInfo, ADC_OPEN_WIRE_E openWire, VOLTAGE_ADC_CHANNEL_E channelSelect);

TRANSACTION_STATUS_E startAuxVoltageConversions(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E clearPackMonitorVoltageRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E clearAccumulators(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E clearPackMonitorFlags(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E readFlagRegister(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readStatRegister(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readCurrentAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readBatteryVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readCurrentAccumulators(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readBatteryVoltageAccumulators(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readPrimaryAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readPrimaryAccumulators(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readVoltage1B(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readOvercurrentRegister(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readPackMonitorSerialId(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readPackMonitorConfigA(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readPackMonitorConfigB(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E writePackMonitorConfigA(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E writePackMonitorConfigB(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readAllCurrentAndBatteryVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readAllAccumulationRegisters(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readSecondaryVoltageAdcs(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readAuxiliaryVoltages(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

TRANSACTION_STATUS_E readAllConfigFlagStatus(CHAIN_INFO_S* chainInfo, ADBMS_PackMonitorData* packMonitor);

#endif /* INC_ADBMS_PACK_MONITOR_H */
