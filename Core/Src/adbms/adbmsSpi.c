/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */
#include "adbms/adbmsSpi.h"
#include "utils.h"
#include <string.h>
#include <stdio.h>


/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Size of CRC per transaction packet
#define CRC_SIZE_BYTES          2

// CRC lookup table size
#define CRC_LUT_SIZE            256

// Command CRC parameters
#define CRC_CMD_SEED            0x0020
#define CRC_CMD_POLY            0x8B32
#define CRC_CMD_SIZE            16

// Data CRC parameters
#define CRC_DATA_SEED           0x0010
#define CRC_DATA_POLY           0x008F
#define CRC_DATA_SIZE           10

// Command counter parameter
#define COMMAND_COUNTER_BITS    6
#define MAX_COMMAND_COUNTER     63
#define MIN_COMMAND_COUNTER     1

// Size of transaction packets
#define COMMAND_PACKET_LENGTH    (COMMAND_SIZE_BYTES + CRC_SIZE_BYTES)
#define REGISTER_PACKET_LENGTH   (REGISTER_SIZE_BYTES + CRC_SIZE_BYTES)

//Read Serial ID Register Group
#define RDSID 0x002C

//Reset Command Counter
#define RSTCC 0x002E

// Number of Read attempts before returning error
#define TRANSACTION_ATTEMPTS    3

// SPI timeout period
#define SPI_TIMEOUT_MS          10

// Max SPI Buffer size
#define MAX_SPI_BUFFER    256

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static uint8_t txBuffer[MAX_SPI_BUFFER];
static uint8_t rxBuffer[MAX_SPI_BUFFER];

/* ==================================================================== */
/* ============================ CRC TABLES ============================ */
/* ==================================================================== */

// CRC lookup table for command PEC
const uint16_t commandCrcTable[CRC_LUT_SIZE] =
{
    0x0000, 0x8B32, 0x9D56, 0x1664, 0xB19E, 0x3AAC, 0x2CC8, 0xA7FA, 0xE80E, 0x633C, 0x7558, 0xFE6A, 0x5990, 0xD2A2, 0xC4C6, 0x4FF4,
    0x5B2E, 0xD01C, 0xC678, 0x4D4A, 0xEAB0, 0x6182, 0x77E6, 0xFCD4, 0xB320, 0x3812, 0x2E76, 0xA544, 0x02BE, 0x898C, 0x9FE8, 0x14DA,
    0xB65C, 0x3D6E, 0x2B0A, 0xA038, 0x07C2, 0x8CF0, 0x9A94, 0x11A6, 0x5E52, 0xD560, 0xC304, 0x4836, 0xEFCC, 0x64FE, 0x729A, 0xF9A8,
    0xED72, 0x6640, 0x7024, 0xFB16, 0x5CEC, 0xD7DE, 0xC1BA, 0x4A88, 0x057C, 0x8E4E, 0x982A, 0x1318, 0xB4E2, 0x3FD0, 0x29B4, 0xA286,
    0xE78A, 0x6CB8, 0x7ADC, 0xF1EE, 0x5614, 0xDD26, 0xCB42, 0x4070, 0x0F84, 0x84B6, 0x92D2, 0x19E0, 0xBE1A, 0x3528, 0x234C, 0xA87E,
    0xBCA4, 0x3796, 0x21F2, 0xAAC0, 0x0D3A, 0x8608, 0x906C, 0x1B5E, 0x54AA, 0xDF98, 0xC9FC, 0x42CE, 0xE534, 0x6E06, 0x7862, 0xF350,
    0x51D6, 0xDAE4, 0xCC80, 0x47B2, 0xE048, 0x6B7A, 0x7D1E, 0xF62C, 0xB9D8, 0x32EA, 0x248E, 0xAFBC, 0x0846, 0x8374, 0x9510, 0x1E22,
    0x0AF8, 0x81CA, 0x97AE, 0x1C9C, 0xBB66, 0x3054, 0x2630, 0xAD02, 0xE2F6, 0x69C4, 0x7FA0, 0xF492, 0x5368, 0xD85A, 0xCE3E, 0x450C,
    0x4426, 0xCF14, 0xD970, 0x5242, 0xF5B8, 0x7E8A, 0x68EE, 0xE3DC, 0xAC28, 0x271A, 0x317E, 0xBA4C, 0x1DB6, 0x9684, 0x80E0, 0x0BD2,
    0x1F08, 0x943A, 0x825E, 0x096C, 0xAE96, 0x25A4, 0x33C0, 0xB8F2, 0xF706, 0x7C34, 0x6A50, 0xE162, 0x4698, 0xCDAA, 0xDBCE, 0x50FC,
    0xF27A, 0x7948, 0x6F2C, 0xE41E, 0x43E4, 0xC8D6, 0xDEB2, 0x5580, 0x1A74, 0x9146, 0x8722, 0x0C10, 0xABEA, 0x20D8, 0x36BC, 0xBD8E,
    0xA954, 0x2266, 0x3402, 0xBF30, 0x18CA, 0x93F8, 0x859C, 0x0EAE, 0x415A, 0xCA68, 0xDC0C, 0x573E, 0xF0C4, 0x7BF6, 0x6D92, 0xE6A0,
    0xA3AC, 0x289E, 0x3EFA, 0xB5C8, 0x1232, 0x9900, 0x8F64, 0x0456, 0x4BA2, 0xC090, 0xD6F4, 0x5DC6, 0xFA3C, 0x710E, 0x676A, 0xEC58,
    0xF882, 0x73B0, 0x65D4, 0xEEE6, 0x491C, 0xC22E, 0xD44A, 0x5F78, 0x108C, 0x9BBE, 0x8DDA, 0x06E8, 0xA112, 0x2A20, 0x3C44, 0xB776,
    0x15F0, 0x9EC2, 0x88A6, 0x0394, 0xA46E, 0x2F5C, 0x3938, 0xB20A, 0xFDFE, 0x76CC, 0x60A8, 0xEB9A, 0x4C60, 0xC752, 0xD136, 0x5A04,
    0x4EDE, 0xC5EC, 0xD388, 0x58BA, 0xFF40, 0x7472, 0x6216, 0xE924, 0xA6D0, 0x2DE2, 0x3B86, 0xB0B4, 0x174E, 0x9C7C, 0x8A18, 0x012A
};

// CRC lookup table for data PEC
const uint16_t dataCrcTable[CRC_LUT_SIZE] =
{
    0x0000, 0x048F, 0x091E, 0x0D91, 0x123C, 0x16B3, 0x1B22, 0x1FAD, 0x24F7, 0x2078, 0x2DE9, 0x2966, 0x36CB, 0x3244, 0x3FD5, 0x3B5A,
    0x49EE, 0x4D61, 0x40F0, 0x447F, 0x5BD2, 0x5F5D, 0x52CC, 0x5643, 0x6D19, 0x6996, 0x6407, 0x6088, 0x7F25, 0x7BAA, 0x763B, 0x72B4,
    0x93DC, 0x9753, 0x9AC2, 0x9E4D, 0x81E0, 0x856F, 0x88FE, 0x8C71, 0xB72B, 0xB3A4, 0xBE35, 0xBABA, 0xA517, 0xA198, 0xAC09, 0xA886,
    0xDA32, 0xDEBD, 0xD32C, 0xD7A3, 0xC80E, 0xCC81, 0xC110, 0xC59F, 0xFEC5, 0xFA4A, 0xF7DB, 0xF354, 0xECF9, 0xE876, 0xE5E7, 0xE168,
    0x2737, 0x23B8, 0x2E29, 0x2AA6, 0x350B, 0x3184, 0x3C15, 0x389A, 0x03C0, 0x074F, 0x0ADE, 0x0E51, 0x11FC, 0x1573, 0x18E2, 0x1C6D,
    0x6ED9, 0x6A56, 0x67C7, 0x6348, 0x7CE5, 0x786A, 0x75FB, 0x7174, 0x4A2E, 0x4EA1, 0x4330, 0x47BF, 0x5812, 0x5C9D, 0x510C, 0x5583,
    0xB4EB, 0xB064, 0xBDF5, 0xB97A, 0xA6D7, 0xA258, 0xAFC9, 0xAB46, 0x901C, 0x9493, 0x9902, 0x9D8D, 0x8220, 0x86AF, 0x8B3E, 0x8FB1,
    0xFD05, 0xF98A, 0xF41B, 0xF094, 0xEF39, 0xEBB6, 0xE627, 0xE2A8, 0xD9F2, 0xDD7D, 0xD0EC, 0xD463, 0xCBCE, 0xCF41, 0xC2D0, 0xC65F,
    0x4EE1, 0x4A6E, 0x47FF, 0x4370, 0x5CDD, 0x5852, 0x55C3, 0x514C, 0x6A16, 0x6E99, 0x6308, 0x6787, 0x782A, 0x7CA5, 0x7134, 0x75BB,
    0x070F, 0x0380, 0x0E11, 0x0A9E, 0x1533, 0x11BC, 0x1C2D, 0x18A2, 0x23F8, 0x2777, 0x2AE6, 0x2E69, 0x31C4, 0x354B, 0x38DA, 0x3C55,
    0xDD3D, 0xD9B2, 0xD423, 0xD0AC, 0xCF01, 0xCB8E, 0xC61F, 0xC290, 0xF9CA, 0xFD45, 0xF0D4, 0xF45B, 0xEBF6, 0xEF79, 0xE2E8, 0xE667,
    0x94D3, 0x905C, 0x9DCD, 0x9942, 0x86EF, 0x8260, 0x8FF1, 0x8B7E, 0xB024, 0xB4AB, 0xB93A, 0xBDB5, 0xA218, 0xA697, 0xAB06, 0xAF89,
    0x69D6, 0x6D59, 0x60C8, 0x6447, 0x7BEA, 0x7F65, 0x72F4, 0x767B, 0x4D21, 0x49AE, 0x443F, 0x40B0, 0x5F1D, 0x5B92, 0x5603, 0x528C,
    0x2038, 0x24B7, 0x2926, 0x2DA9, 0x3204, 0x368B, 0x3B1A, 0x3F95, 0x04CF, 0x0040, 0x0DD1, 0x095E, 0x16F3, 0x127C, 0x1FED, 0x1B62,
    0xFA0A, 0xFE85, 0xF314, 0xF79B, 0xE836, 0xECB9, 0xE128, 0xE5A7, 0xDEFD, 0xDA72, 0xD7E3, 0xD36C, 0xCCC1, 0xC84E, 0xC5DF, 0xC150,
    0xB3E4, 0xB76B, 0xBAFA, 0xBE75, 0xA1D8, 0xA557, 0xA8C6, 0xAC49, 0x9713, 0x939C, 0x9E0D, 0x9A82, 0x852F, 0x81A0, 0x8C31, 0X88BE
};

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

/**
 * @brief Calculate a command CRC across a given command code
 * @param packet Byte array of command code
 * @param numBytes Size of byte array
 */
static uint16_t calculateCommandCrc(uint8_t *packet, uint32_t numBytes);

/**
 * @brief Calculate a data CRC across a given data packet
 * @param packet Byte array of data
 * @param numBytes Size of byte array
 * @param commandCounter Data packet command counter to include in CRC calculation
 */
static uint16_t calculateDataCrc(uint8_t *packet, uint32_t numBytes, uint8_t commandCounter);

 /**
 * @brief Send a command over isospi
 * @param portInstance
 * @return Transaction status error code
 */
 static TRANSACTION_STATUS_E sendSPI(PORT_INSTANCE_S *portInstance, uint8_t packetLength);

/**
 * @brief Send a command over isospi
 * @param command Command code to send
 * @param portInstance Isospi port on which to issue command
 * @return Transaction status error code
 */
static TRANSACTION_STATUS_E sendCommand(uint16_t command, PORT_INSTANCE_S *portInstance);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

/**
 * @brief Calculate a command CRC across a given command code
 * @param packet Byte array of command code
 * @param numBytes Size of byte array
 */
static uint16_t calculateCommandCrc(uint8_t *packet, uint32_t numBytes)
{
    // Begin crc calculation with intial value
    uint16_t crc = CRC_CMD_SEED;

    // For each byte of data, use lookup table to efficiently calculate crc
    for(uint32_t i = 0; i < numBytes; i++)
    {
        // Determine the next look up table index from the current crc and next data byte
        uint8_t index = (uint8_t)((crc >> (CRC_CMD_SIZE - BITS_IN_BYTE)) ^ packet[i]);

        // Calculate the next intermediate crc from the current crc and look up table
        crc = ((crc << BITS_IN_BYTE) ^ (uint16_t)(commandCrcTable[index]));
    }

    return crc;
}

/**
 * @brief Calculate a data CRC across a given data packet
 * @param packet Byte array of data
 * @param numBytes Size of byte array
 * @param commandCounter Data packet command counter to include in CRC calculation
 */
static uint16_t calculateDataCrc(uint8_t *packet, uint32_t numBytes, uint8_t commandCounter)
{
    // Begin crc calculation with intial value
    uint16_t crc = CRC_DATA_SEED;

    // For each byte of data, use lookup table to efficiently calculate crc
    for(uint32_t i = 0; i < numBytes; i++)
    {
        // Determine the next look up table index from the current crc and next data byte
        uint8_t index = (uint8_t)((crc >> (CRC_DATA_SIZE - BITS_IN_BYTE)) ^ packet[i]);

        // Calculate the next intermediate crc from the current crc and look up table
        crc = ((crc << BITS_IN_BYTE) ^ (uint16_t)(dataCrcTable[index]));
    }

    // Clear bit shift residue
    crc &= 0x03FF;

    // Determine the next look up table index from the current crc and next data byte
    uint8_t index = (uint8_t)((crc >> (CRC_DATA_SIZE - COMMAND_COUNTER_BITS)) ^ commandCounter);

    // Calculate the next intermediate crc from the current crc and look up table
    crc = ((crc << COMMAND_COUNTER_BITS) ^ (uint16_t)(dataCrcTable[index]));

    // Clear bit shift residue
    crc &= 0x03FF;

    return crc;
}

 /**
 * @brief Activate port
 * @param portInstance
 * @param numDevs
 * @param usDelay
 */
void activatePort(PORT_INSTANCE_S *portInstance, uint8_t numDevs, uint32_t usDelay)
{
    for(uint8_t i = 0; i < (numDevs + 1); i++)
    {
        HAL_GPIO_WritePin(portInstance->csPort, portInstance->csPin, GPIO_PIN_RESET);
        HAL_GPIO_WritePin(portInstance->csPort, portInstance->csPin, GPIO_PIN_SET);        
        delayMicroseconds(usDelay);
    }

}

 /**
 * @brief Send a command over isospi
 * @param portInstance
 * @return Transaction status error code
 */
 static TRANSACTION_STATUS_E sendSPI(PORT_INSTANCE_S *portInstance, uint8_t packetLength)
 {
    // Open SPI port
    HAL_GPIO_WritePin(portInstance->csPort, portInstance->csPin, GPIO_PIN_RESET);
    if(taskNotifySPI(portInstance->spiHandle, txBuffer, rxBuffer, packetLength, SPI_TIMEOUT_MS) != SPI_SUCCESS)
    {
        // Close SPI port
        HAL_GPIO_WritePin(portInstance->csPort, portInstance->csPin, GPIO_PIN_SET);
        return TRANSACTION_SPI_ERROR;
    }

    // Close SPI port
    HAL_GPIO_WritePin(portInstance->csPort, portInstance->csPin, GPIO_PIN_SET);
    return TRANSACTION_SUCCESS;
 }

/**
 * @brief Send a command over isospi
 * @param command Command code to send
 * @param portInstance Isospi port on which to issue command
 * @return Transaction status error code
 */
static TRANSACTION_STATUS_E sendCommand(uint16_t command, PORT_INSTANCE_S *portInstance)
{
    // Populate the tx buffer with the command word
    txBuffer[0] = (uint8_t)(command >> BITS_IN_BYTE);
    txBuffer[1] = (uint8_t)(command);

    // Calculate the CRC on the command word and place the tx buffer
    uint16_t commandCRC = calculateCommandCrc(txBuffer, COMMAND_SIZE_BYTES);
    txBuffer[2] = (uint8_t)(commandCRC >> BITS_IN_BYTE);
    txBuffer[3] = (uint8_t)(commandCRC);

    return sendSPI(portInstance, COMMAND_PACKET_LENGTH);
}

/**
 * @brief Write data over isospi - data buffer should include 6 bytes per device
 * @param command Command code to initiate write transaction
 * @param numDevs Number of chain devices to write to
 * @param txBuff Byte array of data to write to device chain. Device data should be ordered in the direction of PortA to PortB
 * @param port Isospi port on which to issue command
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E writeRegister(uint16_t command, uint32_t numDevs, uint8_t *txBuff, PORT_INSTANCE_S *portInstance)
{
    // Size in bytes: Command Word(2) + Command CRC(2) + [Register data(6) + Data CRC(2)] * numDevs
    uint32_t packetLength = COMMAND_PACKET_LENGTH + (numDevs * REGISTER_PACKET_LENGTH);

    // Populate the tx buffer with the command word
    txBuffer[0] = (uint8_t)(command >> BITS_IN_BYTE);
    txBuffer[1] = (uint8_t)(command);

    // Calculate the CRC on the command word and place the tx buffer
    uint16_t commandCRC = calculateCommandCrc(txBuffer, COMMAND_SIZE_BYTES);
    txBuffer[2] = (uint8_t)(commandCRC >> BITS_IN_BYTE);
    txBuffer[3] = (uint8_t)(commandCRC);

    // For each device, append a copy of the register data and corresponding CRC to the tx buffer
    for(uint32_t i = 0; i < numDevs; i++)
    {
        // Calculate the CRC on the register data packet (2 byte CRC on 6 byte packet)
        uint16_t dataCRC = calculateDataCrc(txBuff + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES, 0);

        // Copy the write data from the provided txBuff and the calculated crc into the txBuffer to be sent over SPI
        // The furthest device from the port receives the first indexed data from txBuff, so data is pasted big endian
        memcpy(txBuffer + COMMAND_PACKET_LENGTH + (i * REGISTER_PACKET_LENGTH), txBuff + (i * REGISTER_SIZE_BYTES), REGISTER_SIZE_BYTES);
        txBuffer[COMMAND_PACKET_LENGTH + (i * REGISTER_PACKET_LENGTH) + REGISTER_SIZE_BYTES] = (uint8_t)(dataCRC >> BITS_IN_BYTE);
        txBuffer[COMMAND_PACKET_LENGTH + (i * REGISTER_PACKET_LENGTH) + REGISTER_SIZE_BYTES + 1] = (uint8_t)(dataCRC);
    }

    // SPIify
    return sendSPI(portInstance, packetLength);
}

/**
 * @brief Helper function to process all data CRCs from a read register buffer
 * @param numDevs Number of chain devices to read from
 * @param registerBuffer The raw data buffer recieved from the isospi transaction
 * @param rxBuff Byte array of data to populate with data from device chain
 * @param port Isospi port on which to command was issued
 * @return Transaction status error code
 */
static TRANSACTION_STATUS_E processReadRegisterCRCs(uint32_t numDevs, uint8_t *registerBuffer, uint8_t *rxBuff, PORT_INSTANCE_S *portInstance)
{
    TRANSACTION_STATUS_E returnStatus = TRANSACTION_SUCCESS;
    uint8_t registerData[REGISTER_SIZE_BYTES];

    for(uint32_t j = 0; j < numDevs; j++)
    {
        // Extract the register data for each device into a temporary array
        memcpy(registerData, registerBuffer + (COMMAND_PACKET_LENGTH + (j * REGISTER_PACKET_LENGTH)), REGISTER_SIZE_BYTES);

        // Extract the CRC and Command Counter sent with the corresponding register data
        uint16_t pec0 = registerBuffer[COMMAND_PACKET_LENGTH + (j * REGISTER_PACKET_LENGTH) + REGISTER_SIZE_BYTES];
        uint16_t pec1 = registerBuffer[COMMAND_PACKET_LENGTH + (j * REGISTER_PACKET_LENGTH) + REGISTER_SIZE_BYTES + 1];
        uint16_t registerCRC = ((pec0 << BITS_IN_BYTE) | (pec1)) & 0x03FF;
        uint8_t deviceCommandCounter = (uint8_t)pec0 >> (BITS_IN_BYTE - COMMAND_COUNTER_BITS);

        // If the CRC is incorrect for the data sent, retry the spi transaction
        if(calculateDataCrc(registerData, REGISTER_SIZE_BYTES, deviceCommandCounter) == registerCRC)
        {
            // If there is a command counter error, track the error to be returned later
            // This allows us to finish checking if there is a chain break or crc error before returning
            if(deviceCommandCounter != portInstance->localCommandCounter)
            {
                // A single Power on reset error will take priority over an already present command counter error
                if((deviceCommandCounter != 0) && (returnStatus != TRANSACTION_POR_ERROR))
                {
                    returnStatus = TRANSACTION_COMMAND_COUNTER_ERROR;
                }
                else
                {
                    returnStatus = TRANSACTION_POR_ERROR;
                }
            }

            // Populate rx buffer with local register data
            // This happens only if there is no crc error, but regardless of if there is a command counter error
            // The first indexed data from the read is from the closest device to the port, so data is pasted big endian
            memcpy(rxBuff + (j * REGISTER_SIZE_BYTES), registerData, REGISTER_SIZE_BYTES);
        }
        else
        {
            // Return CRC error
            return TRANSACTION_CHAIN_BREAK_ERROR;
        }
    }

    return returnStatus;
}

uint16_t readRegisterCounter = 0;

/**
 * @brief Read data over isospi - data buffer will be populated with 6 bytes per device
 * @param command Command code to initiate read transaction
 * @param numDevs Number of chain devices to read from
 * @param rxBuff Byte array of data to populate with data from device chain
 * @param port Isospi port on which to issue command
 * @param localCommandCounter The array of command counters to be incremented. Each index should hold a device type
 * @param packMonitorIndex The index of the pack monitor device as seen from the current port
 * @return Transaction status error code
 */
TRANSACTION_STATUS_E readRegister(uint16_t command, uint32_t numDevs, uint8_t *rxBuff, PORT_INSTANCE_S *portInstance/*, uint32_t *localCommandCounter, uint32_t packMonitorIndex*/)
{
    // Size in bytes: Command Word(2) + Command CRC(2) + [Register data(6) + Data CRC(2)] * numDevs
    uint32_t packetLength = COMMAND_PACKET_LENGTH + (numDevs * REGISTER_PACKET_LENGTH);

    // Clear tx buffer array
    memset(txBuffer, 0, packetLength);

    // Populate the tx buffer with the command word
    txBuffer[0] = (uint8_t)(command >> BITS_IN_BYTE);
    txBuffer[1] = (uint8_t)(command);

    // Calculate the CRC on the command word and place the tx buffer
    uint16_t commandCRC = calculateCommandCrc(txBuffer, COMMAND_SIZE_BYTES);
    txBuffer[2] = (uint8_t)(commandCRC >> BITS_IN_BYTE);
    txBuffer[3] = (uint8_t)(commandCRC);

    for(int32_t i = 0; i < TRANSACTION_ATTEMPTS; i++)
    {
        // SPIify!
        if(sendSPI(portInstance, packetLength) != TRANSACTION_SUCCESS)
        {
            return TRANSACTION_SPI_ERROR;
        }

        TRANSACTION_STATUS_E returnStatus = processReadRegisterCRCs(numDevs, rxBuffer, rxBuff, portInstance);
        if(returnStatus != TRANSACTION_CHAIN_BREAK_ERROR)
        {
            return returnStatus;
        }
    }
    // If there are enough failed attempts with crc errors, return chain break error
    printf("transaction failure\n");
    return TRANSACTION_CHAIN_BREAK_ERROR;
}