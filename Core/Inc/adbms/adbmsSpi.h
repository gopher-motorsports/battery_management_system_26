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

#define BITS_IN_BYTE        8
#define BITS_IN_WORD        16
#define BYTE_SIZE_DEC       256
#define BYTES_IN_WORD       2

#define COMMAND_SIZE_BYTES              2
#define LPCM_REGISTER_SIZE_BYTES        2
#define REGISTER_SIZE_BYTES             6
#define EXTENDED_REGISTER_SIZE_BYTES    20

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

    // Timer used for delay in activatePort function
    TIM_HandleTypeDef *delayTimerHandle;

} CHAIN_INFO_S;


/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

void activatePort(CHAIN_INFO_S* chainInfo, uint32_t usDelay);

void incCommandCounter(CHAIN_INFO_S *chainInfo);

TRANSACTION_STATUS_E updateChainStatus(CHAIN_INFO_S *chainInfo);

TRANSACTION_STATUS_E commandChain(uint16_t command, CHAIN_INFO_S *chainInfo);

TRANSACTION_STATUS_E writeChain(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *txData, uint8_t registerSize);

TRANSACTION_STATUS_E readChain(uint16_t command, CHAIN_INFO_S *chainInfo, uint8_t *rxData, uint8_t registerSize);

TRANSACTION_STATUS_E freezeRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E unfreezeRegisters(CHAIN_INFO_S* chainInfo);

TRANSACTION_STATUS_E softReset(CHAIN_INFO_S* chainInfo);

#endif /* INC_ADBMS_SPI_H_ */
