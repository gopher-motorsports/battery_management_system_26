#ifndef INC_ADBMS_CELL_MONITOR_H_
#define INC_ADBMS_CELL_MONITOR_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbmsSpi.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define NUM_CELLS_PER_CELL_MONITOR  14
#define NUM_CELL_MONITOR_GPIO       10

#define REGISTER_BYTE0      0
#define REGISTER_BYTE1      1
#define REGISTER_BYTE2      2
#define REGISTER_BYTE3      3
#define REGISTER_BYTE4      4
#define REGISTER_BYTE5      5

// ADC result register encoding
#define CELL_MON_CELL_ADC_GAIN          0.00015f
#define CELL_MON_CELL_ADC_OFFSET        1.5f

#define CELL_MON_AUX_ADC_GAIN           0.00015f
#define CELL_MON_AUX_ADC_OFFSET         1.5f

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    COMPARE_THRESHOLD_5_1_mV = 0,
    COMPARE_THRESHOLD_8_1_mV,
    COMPARE_THRESHOLD_9_mV,
    COMPARE_THRESHOLD_10_mV,
    COMPARE_THRESHOLD_15_mV,
    COMPARE_THRESHOLD_20_mV,
    COMPARE_THRESHOLD_25_mV,
    COMPARE_THRESHOLD_40_mV
} COMPARISON_THRESHOLD_E;

typedef enum
{
    AUX_SOAK_DISABLED =     0x0,
    AUX_SOAK_TIME_32_US =   0x10,
    AUX_SOAK_TIME_64_US =   0x11,
    AUX_SOAK_TIME_128_US =  0x12,
    AUX_SOAK_TIME_256_US =  0x13,
    AUX_SOAK_TIME_512_US =  0x14,
    AUX_SOAK_TIME_1_MS =    0x15,
    AUX_SOAK_TIME_2_MS =    0x16,
    AUX_SOAK_TIME_4_1_MS =  0x18,
    AUX_SOAK_TIME_8_2_MS =  0x19,
    AUX_SOAK_TIME_16_4_MS = 0x1A,
    AUX_SOAK_TIME_32_8_MS = 0x1B,
    AUX_SOAK_TIME_65_5_MS = 0x1C,
    AUX_SOAK_TIME_131_MS =  0x1D,
    AUX_SOAK_TIME_262_MS =  0x1E,
    AUX_SOAK_TIME_524_MS =  0x1F
} AUX_SOAK_TIME_E;

typedef enum
{
    FILTER_DISABLED = 0,
    FILTER_CUTOFF_110_HZ,
    FILTER_CUTOFF_45_HZ,
    FILTER_CUTOFF_21_HZ,
    FILTER_CUTOFF_10_HZ,
    FILTER_CUTOFF_5_HZ,
    FILTER_CUTOFF_1_25_HZ,
    FILTER_CUTOFF_625_mHZ
} DIGITAL_FILTER_SETTING_E;

typedef enum
{
    NON_REDUNDANT_MODE = 0,
    REDUNDANT_MODE = (1 << 8)
} ADC_MODE_REDUNDANT_E;

typedef enum
{
    SINGLE_SHOT_MODE = 0,
    CONTINUOUS_MODE = (1 << 7)
} ADC_MODE_CONTINUOUS_E;

typedef enum
{
    DISCHARGE_DISABLED = 0,
    DISCHARGE_PERMITTED = (1 << 4)
} ADC_MODE_DISCHARGE_E;

typedef enum
{
    NO_FILTER_RESET = 0,
    FILTER_RESET = (1 << 2)
} ADC_MODE_FILTER_E;

typedef enum
{
    CELL_OPEN_WIRE_DISABLED = 0,
    CELL_OPEN_WIRE_EVEN,
    CELL_OPEN_WIRE_ODD
} ADC_MODE_CELL_OPEN_WIRE_E;

typedef enum
{
    AUX_OPEN_WIRE_DISABLED = 0,
    AUX_OPEN_WIRE_PULL_DOWN = (1 << 8),
    AUX_OPEN_WIRE_PULL_UP = ((1 << 8) | (1 << 7))
} ADC_MODE_AUX_OPEN_WIRE_E;

typedef enum
{
    AUX_ALL_CHANNELS = 0,
    AUX_GPIO1_ONLY = 0x01,
    AUX_GPIO2_ONLY = 0x02,
    AUX_GPIO3_ONLY = 0x03,
    AUX_GPIO4_ONLY = 0x04,
    AUX_GPIO5_ONLY = 0x05,
    AUX_GPIO6_ONLY = 0x06,
    AUX_GPIO7_ONLY = 0x07,
    AUX_GPIO8_ONLY = 0x08,
    AUX_GPIO9_ONLY = 0x09,
    AUX_GPIO10_ONLY = 0x0A,
    AUX_VREF2_ONLY = 0x10,
    AUX_VD_ONLY = 0x11,
    AUX_VA_ONLY = 0x12,
    AUX_ITEMP_ONLY = 0x13,
    AUX_VPV_ONLY = 0x14,
    AUX_VMV_ONLY = 0x15,
    AUX_VRES_ONLY = 0x16
} ADC_MODE_AUX_CHANNEL_E;

typedef enum
{
    RAW_CELL_VOLTAGE = 0,
    AVERAGED_CELL_VOLTAGE,
    FILTERED_CELL_VOLTAGE,
    NUM_CELL_VOLTAGE_TYPES
} CELL_VOLTAGE_TYPE_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct __attribute__((packed))
{
    // Byte 0
    COMPARISON_THRESHOLD_E comparisonThreshold : 3;
    uint8_t reserved1 : 4;
    uint8_t referenceOn : 1;

    // Byte 1
    uint8_t forceOscillatorCounterFast : 1;
    uint8_t forceOscillatorCounterSlow : 1;
    uint8_t assertSupplyError : 1;
    uint8_t supplyErrorSelect : 1;
    uint8_t assertThermalShutdownDetectedFault : 1;
    uint8_t assertSingleTrimError : 1;
    uint8_t assertMultipleTrimError : 1;
    uint8_t assertTestModeDetectedFault : 1;

    // Byte 2
    uint8_t reserved2 : 3;
    AUX_SOAK_TIME_E soakTime : 5;

    // Byte 3
    uint8_t gpo1State : 1;
    uint8_t gpo2State : 1;
    uint8_t gpo3State : 1;
    uint8_t gpo4State : 1;
    uint8_t gpo5State : 1;
    uint8_t gpo6State : 1;
    uint8_t gpo7State : 1;
    uint8_t gpo8State : 1;

    // Byte 4
    uint8_t gpo9State : 1;
    uint8_t gpo10State : 1;
    uint8_t reserved3 : 6;

    // Byte 5
    DIGITAL_FILTER_SETTING_E digitalFilterSetting : 3;
    uint8_t commBreak : 1;
    uint8_t muteStatus : 1;
    uint8_t snapStatus : 1;
    uint8_t reserved4 : 2;

} ADBMS_ConfigACellMonitor;

typedef struct __attribute__((packed))
{
    float undervoltageThreshold;
    float overvoltageThreshold;
    uint32_t dischargeTimeoutMinutes;
    bool dischargeCell[NUM_CELLS_PER_CELL_MONITOR];
} ADBMS_ConfigBCellMonitor;

typedef struct __attribute__((packed))
{
    float referenceVoltage;
    float dieTemp;
} ADBMS_StatusACellMonitor;

typedef struct __attribute__((packed))
{
    float digitalSupplyVoltage;
    float analogSupplyVoltage;
    float referenceResistorVoltage;
} ADBMS_StatusBCellMonitor;

typedef struct __attribute__((packed))
{
    uint32_t conversionCounter;

    uint8_t redundantAdcMultipleTrimError : 1;
    uint8_t redundantAdcTrimError : 1;
    uint8_t primaryAdcMultipleTrimError : 1;
    uint8_t primaryAdcTrimError : 1;
    uint8_t digitalUnderVoltage : 1;
    uint8_t digitalOverVoltage : 1;
    uint8_t analogUnderVoltage : 1;
    uint8_t analogOverVoltage : 1;

    uint8_t oscillatorFault : 1;
    uint8_t testModeDetected : 1;
    uint8_t thermalShutdownDetected : 1;
    uint8_t sleepDetected : 1;
    uint8_t spiFault : 1;
    uint8_t adcComparisonActive : 1;
    uint8_t supplyRailDeltaFault : 1;
    uint8_t supplyRailDeltaFaultLatching : 1;

    uint8_t cellAdcMismatchFault[NUM_CELLS_PER_CELL_MONITOR];
} ADBMS_StatusCCellMonitor;

typedef struct __attribute__((packed))
{
    uint8_t cellUnderVoltageFault[NUM_CELLS_PER_CELL_MONITOR];
    uint8_t cellOverVoltageFault[NUM_CELLS_PER_CELL_MONITOR];
    uint8_t oscillatorCounter;
} ADBMS_StatusDCellMonitor;

typedef struct __attribute__((packed))
{
    uint8_t gpi1State : 1;
    uint8_t gpi2State : 1;
    uint8_t gpi3State : 1;
    uint8_t gpi4State : 1;
    uint8_t gpi5State : 1;
    uint8_t gpi6State : 1;
    uint8_t gpi7State : 1;
    uint8_t gpi8State : 1;

    uint8_t gpi9State : 1;
    uint8_t gpi10State : 1;
    uint8_t reserved1 : 2;
    uint8_t revisionCode : 4;
} ADBMS_StatusECellMonitor;

typedef struct __attribute__((packed))
{
    ADBMS_ConfigACellMonitor configGroupA;
    ADBMS_ConfigBCellMonitor configGroupB;

    ADBMS_StatusACellMonitor statusGroupA;
    ADBMS_StatusBCellMonitor statusGroupB;
    ADBMS_StatusCCellMonitor statusGroupC;
    ADBMS_StatusDCellMonitor statusGroupD;
    ADBMS_StatusECellMonitor statusGroupE;

    float cellVoltage[NUM_CELLS_PER_CELL_MONITOR];
    float redundantCellVoltage[NUM_CELLS_PER_CELL_MONITOR];

    float auxVoltage[NUM_CELL_MONITOR_GPIO];
    float reduntantAuxVoltage[NUM_CELL_MONITOR_GPIO];

    float hvSupplyVoltage;
    float switch1Voltage;

    float dischargePWM[NUM_CELLS_PER_CELL_MONITOR];

    uint8_t serialId[REGISTER_SIZE_BYTES];
    uint8_t retentionRegister[REGISTER_SIZE_BYTES];

} ADBMS_CellMonitorData;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

TRANSACTION_STATUS_E startCellConversions(CHAIN_INFO_S* chainInfo, ADC_MODE_REDUNDANT_E redundantMode, ADC_MODE_CONTINUOUS_E continuousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_FILTER_E filterMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startRedundantCellConversions(CHAIN_INFO_S* chainInfo, ADC_MODE_CONTINUOUS_E continousMode, ADC_MODE_DISCHARGE_E dischargeMode, ADC_MODE_CELL_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startAuxConversions(CHAIN_INFO_S* chainInfo, ADC_MODE_AUX_CHANNEL_E auxChannel, ADC_MODE_AUX_OPEN_WIRE_E openWireMode);

TRANSACTION_STATUS_E startRedundantAuxConversions(CHAIN_INFO_S* chainInfo, ADC_MODE_AUX_CHANNEL_E auxChannel);

TRANSACTION_STATUS_E muteDischarge(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E unmuteDischarge(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E freezeRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E unfreezeRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E softReset(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E clearCellMonitorCellMonitorVoltageRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E clearCellMonitorFlags(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E readCellMonitorSerialId(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E writePwmRegisters(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readPwmRegisters(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E writeNVM(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readNVM(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E writeCellMonitorConfigA(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readCellMonitorConfigA(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E writeCellMonitorConfigB(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readCellMonitorConfigB(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readStatusA(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readStatusB(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readStatusC(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readStatusD(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readStatusE(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readCellVoltages(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor, CELL_VOLTAGE_TYPE_E cellVoltageType);

TRANSACTION_STATUS_E readRedundantCellVoltages(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readAuxVoltages(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

TRANSACTION_STATUS_E readRedundantAuxVoltages(CHAIN_INFO_S* chainInfo, ADBMS_CellMonitorData* cellMonitor);

#endif /* INC_ADBMS_CELL_MONITOR_H_ */
