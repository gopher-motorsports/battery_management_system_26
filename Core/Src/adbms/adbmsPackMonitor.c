/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "adbms/adbmsPackMonitor.h"

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
#define RDCFGA      0x0002 // Read Configuration Register Group A
#define RDCFGB      0x0026 // Read Configuration Register Group B
#define RDCOMM      0x0722 // Read COMM Register Group
#define WRCFGA      0x0001 // Write Configuration Register Group A
#define WRCFGB      0x0024 // Write Configuration Register Group B
#define WRCOMM      0x0721 // Write COMM Register Group
#define STCOMM      0x0723 // Send COMM Register
#define RDALLI      0x000C // Read IxADC and VBxADC Results
#define RDALLV      0x0035 // Read All External Input V1ADC Results and V2ADC V9, V10 Results
#define RDALLR      0x0011 // Read All External Input V2ADC Results and V1ADC V7, V8 Results
#define RDALLX      0x0051 // Read All AUX ADC Results
#define RDALLC      0x0010 // Read All Configuration, Flag, and Status Registers

/* END Pack Monitor Register Addresses */

#define TRANSACTION_SIZE_BYTES_48_BIT       48 / BITS_IN_BYTE
#define TRANSACTION_SIZE_BYTES_160_BIT      160 / BITS_IN_BYTE

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static uint8_t transactionBuffer[20]; // TODO: Change size to #define

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

TRANSACTION_STATUS_E softReset(CHAIN_INFO_S* chainInfo)
{
    return commandChain(SRST, chainInfo);
}

TRANSACTION_STATUS_E freezeRegisters(CHAIN_INFO_S* chainInfo)
{
    return commandChain(SNAP, chainInfo);
}

TRANSACTION_STATUS_E unfreezeRegisters(CHAIN_INFO_S* chainInfo)
{
    return commandChain(UNSNAP, chainInfo);
}

// TODO: Figure out configuration for OPT[3:0] in start adc commands
// TRANSACTION_STATUS_E startPackChannelOneConversions(CHAIN_INFO_S* chainInfo)
// {
//     return commandChain(ADI1, chainInfo);
// }

TRANSACTION_STATUS_E clearAllVoltageRegisters(CHAIN_INFO_S* chainInfo)
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

TRANSACTION_STATUS_E clearAllFlags(CHAIN_INFO_S* chainInfo)
{
    memset(transactionBuffer, 0xFF, TRANSACTION_SIZE_BYTES_48_BIT);

    TRANSACTION_STATUS_E status = writeChain(CLRFLAG, chainInfo, transactionBuffer);

    activatePort(chainInfo, TIME_READY_US);

    return status;
}

