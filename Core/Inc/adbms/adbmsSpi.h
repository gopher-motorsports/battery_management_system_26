#ifndef INC_ADBMS_SPI_H_
#define INC_ADBMS_SPI_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include <stdint.h>
#include <stdbool.h>
#include "main.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

/* ADBMS Register addresses */

#define WRCFGA      0x0001 // Write Configuration Register Group A
#define WRCFGB      0x0024 // Write Configuration Register Group B
#define RDCFGA      0x0002 // Read Configuration Register Group A
#define RDCFGB      0x0026 // Read Configuration Register Group B
#define RDCVA       0x0004 // Read Cell Voltage Register Group A
#define RDCVB       0x0006 // Read Cell Voltage Register Group B
#define RDCVC       0x0008 // Read Cell Voltage Register Group C
#define RDCVD       0x000A // Read Cell Voltage Register Group D
#define RDCVE       0x0009 // Read Cell Voltage Register Group E
#define RDCVF       0x000B // Read Cell Voltage Register Group F
#define RDACA       0x0044 // Read Averaged Cell Voltage Register Group A
#define RDACB       0x0046 // Read Averaged Cell Voltage Register Group B
#define RDACC       0x0048 // Read Averaged Cell Voltage Register Group C
#define RDACD       0x004A // Read Averaged Cell Voltage Register Group D
#define RDACE       0x0049 // Read Averaged Cell Voltage Register Group E
#define RDACF       0x004B // Read Averaged Cell Voltage Register Group F
#define RDSVA       0x0003 // Read S Voltage Register Group A
#define RDSVB       0x0005 // Read S Voltage Register Group B
#define RDSVC       0x0007 // Read S Voltage Register Group C
#define RDSVD       0x000D // Read S Voltage Register Group D
#define RDSVE       0x000E // Read S Voltage Register Group E
#define RDSVF       0x000F // Read S Voltage Register Group F
#define RDFCA       0x0012 // Read Filter Cell Voltage Register Group A
#define RDFCB       0x0013 // Read Filter Cell Voltage Register Group B
#define RDFCC       0x0014 // Read Filter Cell Voltage Register Group C
#define RDFCD       0x0015 // Read Filter Cell Voltage Register Group D
#define RDFCE       0x0016 // Read Filter Cell Voltage Register Group E
#define RDFCF       0x0017 // Read Filter Cell Voltage Register Group F
#define RDAUXA      0x0019 // Read Auxiliary Register Group A
#define RDAUXB      0x001A // Read Auxiliary Register Group B
#define RDAUXC      0x001B // Read Auxiliary Register Group C
#define RDAUXD      0x001F // Read Auxiliary Register Group D
#define RDRAXA      0x001C // Read Redundant Auxiliary Register Group A
#define RDRAXB      0x001D // Read Redundant Auxiliary Register Group B
#define RDRAXC      0x001E // Read Auxiliary Redundant Register Group C
#define RDRAXD      0x0025 // Read Auxiliary Redundant Register Group D
#define RDSTATA     0x0030 // Read Status Register Group A
#define RDSTATB     0x0031 // Read Status Register Group B
#define RDSTATC     0x0032 // Read Status Register Group C
#define RDSTATD     0x0033 // Read Status Register Group D
#define RDSTATE     0x0034 // Read Status Register Group E
#define WRPWMA      0x0020 // Write PWM Register Group A
#define RDPWMA      0x0022 // Read PWM Register Group A
#define WRPWMB      0x0021 // Write PWM Register Group B
#define RDPWMB      0x0023 // Read PWM Register Group B
#define CMDIS       0x0040 // LPCM Disable
#define CMEN        0x0041 // LPCM Enable
#define CMHB2       0x0043 // LPCM Heartbeat
#define WRCMCFG     0x0058 // Write LPCM Configuration Register
#define RDCMCFG     0x0059 // Read LPCM Configuration Register
#define WRCMCELLT   0x005A // Write LPCM Cell Threshold
#define RDCMCELLT   0x005B // Read LPCM Cell Threshold
#define WRCMGPIOT   0x005C // Write LPCM GPIO Threshold
#define RDCMGPIOT   0x005D // Read LPCM GPIO Threshold
#define CLRCMFLAG   0x005E // Clear LPCM Flags
#define RDCMFLAG    0x005F // Read LPCM Flags
#define ADCV        0x0260 // Start Cell Voltage ADC Conversion and Poll Status
#define ADSV        0x0168 // Start S-ADC Conversion and Poll Status
#define ADAX        0x0410 // Start AUX ADC Conversions and Poll Status
#define ADAX2       0x0400 // Start AUX2 ADC Conversions and Poll Status
#define CLRCELL     0x0711 // Clear Cell Voltage Register Groups
#define CLRFC       0x0714 // Clear Filtered Cell Voltage Register Groups
#define CLRAUX      0x0712 // Clear Auxiliary Register Groups
#define CLRSPIN     0x0716 // Clear S-Voltage Register Groups
#define CLRFLAG     0x0717 // Clear Flags
#define CLOVUV      0x0715 // Clear OVUV
#define WRCOMM      0x0721 // Write COMM Register Group
#define RDCOMM      0x0722 // Read COMM Register Group
#define STCOMM      0x0723 // Start I2C/SPI Communication
#define MUTE        0x0028 // Mute Discharge
#define UNMUTE      0x0029 // Unmute Discharge
#define RDSID       0x002C // Read Serial ID Register Group
#define RSTCC       0x002E // Reset Command Counter
#define SNAP        0x002D // Snapshot
#define UNSNAP      0x002F // Release Snapshot
#define SRST        0x0027 // Soft Reset
#define ULRR        0x0038 // Unlock Retention Register
#define WRRR        0x0039 // Write Retention Registers
#define RDRR        0x003A // Read Retention Registers

#define ADV         0x0430
#define ADX         0x0530
#define CLRO        0x0713

/* END ADBMS Register addresses */

#define BITS_IN_BYTE        8
#define BITS_IN_WORD        16
#define BYTE_SIZE_DEC       256
#define BYTES_IN_WORD       2

#define COMMAND_SIZE_BYTES       2
#define REGISTER_SIZE_BYTES      6

// Time for ADBMS device to wake
#define TIME_WAKE_US            500

// Time for ADBMS device to transition from idle state
#define TIME_READY_US           10

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    TRANSACTION_CHAIN_BREAK_ERROR = 0,
    TRANSACTION_SPI_ERROR,
    TRANSACTION_POR_ERROR,
    TRANSACTION_COMMAND_COUNTER_ERROR,
    TRANSACTION_SUCCESS
} TRANSACTION_STATUS_E;

typedef enum
{
    CELL_MONITOR = 0,
    PACK_MONITOR,
    NUM_DEVICE_TYPES
} DEVICE_TYPE_E;

typedef enum
{
    PORTA = 0,
    PORTB,
    NUM_PORTS
} PORT_E;

typedef enum
{
    MULTIPLE_CHAIN_BREAK = 0,
    SINGLE_CHAIN_BREAK,
    CHAIN_COMPLETE
} CHAIN_STATUS_E;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    SPI_HandleTypeDef *spiHandle;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
} PORT_INSTANCE_S;

typedef struct 
{
    PORT_INSTANCE_S commPorts[NUM_PORTS];

    // Number of devices
    uint8_t numDevs;

    // The current status of the isospi chain
    CHAIN_STATUS_E chainStatus;

    // The number of reachable devices on each port
    uint32_t availableDevices[NUM_PORTS];

    // The current port to be used for isospi transactions
    // Alternate between ports after each write or read
    PORT_E currentPort;

    // The command counter
    uint8_t localCommandCounter;
} CHAIN_INFO_S;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void activatePort(PORT_INSTANCE_S *portInstance, uint8_t numDevs, uint32_t usDelay);

void incCommandCounter(CHAIN_INFO_S *chainInfo);

TRANSACTION_STATUS_E updateChainStatus(CHAIN_INFO_S *chainInfo);

TRANSACTION_STATUS_E commandChain(uint16_t command, CHAIN_INFO_S *chainInfo);

TRANSACTION_STATUS_E writeChain(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *txData);

TRANSACTION_STATUS_E readChain(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *rxData);

#endif /* INC_ADBMS_SPI_H_ */
