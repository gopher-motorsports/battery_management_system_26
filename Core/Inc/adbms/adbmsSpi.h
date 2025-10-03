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

// Max SPI Buffer size
#define MAX_SPI_BUFFER    256

#define BITS_IN_BYTE        8
#define BITS_IN_WORD        16
#define BYTE_SIZE_DEC       256
#define BYTES_IN_WORD       2

#define COMMAND_SIZE_BYTES       2
#define REGISTER_SIZE_BYTES      6

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

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    SPI_HandleTypeDef *spiHandle;
    GPIO_TypeDef *csPort;
    uint16_t csPin;
    // TODO: find new location for localCommandCounter
    uint8_t localCommandCounter;
} PORT_INSTANCE_S;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

 /**
 * @brief Activate port
 * @param portInstance
 * @param numDevs
 * @param usDelay
 */
void activatePort(PORT_INSTANCE_S *portInstance, uint8_t numDevs, uint32_t usDelay);

/**
 * @brief Write data over isospi - data buffer should include 6 bytes per device
 * @param command Command code to initiate write transaction
 * @param numDevs Number of chain devices to write to
 * @param txBuff Byte array of data to write to device chain. Device data should be ordered in the direction of PortA to PortB
 * @param port Isospi port on which to issue command
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writeRegister(uint16_t command, uint32_t numDevs, uint8_t *txBuff, PORT_INSTANCE_S *portInstance);


#endif /* INC_ADBMS_SPI_H_ */