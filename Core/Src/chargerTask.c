/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "main.h"
#include "chargerTask.h"
#include "charger.h"
#include "GopherCAN.h"
#include "gopher_sense.h"
#include "packData.h"
#include "updateCellMonitorTask.h"
#include "updatePackMonitorTask.h"
#include <stdlib.h>
#include <string.h>

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

// Communication timeout ms
#define ELCON_CHARGER_COMM_TIMEOUT  3000
#define POWER_LIMIT_COMM_TIMEOUT  10000

// Power limit
#define DEFAULT_POWER_LIMIT_W       1500.0f
#define ABSOLUTE_POWER_LIMIT_W      6000.0f

// Voltage limit
#define MAX_CHARGE_VOLTAGE_V                MAX_CELL_VOLTAGE * NUM_SERIES_CELLS

// Current limit
#define MAH_TO_AH                           (1.0f / 1000.0f)
#define MAX_CHARGE_CURRENT_A                CELL_CAPACITY_MAH * MAH_TO_AH * MAX_C_RATING * NUM_PARALLEL_CELLS

// Voltage threshold to switch to CV mode
#define CELL_VOLTAGE_CV_THRES               4.15f
#define CELL_VOLTAGE_CV_HYS                 0.15f

#define CELL_IMBALANCE_THRESHOLD            0.01f

/* ==================================================================== */
/* ========================= LOCAL VARIABLES ========================== */
/* ==================================================================== */

static chargerTaskData_S taskData;

chargerTaskData_S publicChargerTaskData;

/* ==================================================================== */
/* =================== LOCAL FUNCTION DECLARATIONS ==================== */
/* ==================================================================== */

static float getPowerLimit();
static float getCurrentLimit(float powerLimit, float packVoltage);

/* ==================================================================== */
/* =================== LOCAL FUNCTION DEFINITIONS ===================== */
/* ==================================================================== */

static float getPowerLimit()
{
    // Calculate the power limit
    float powerLimit = DEFAULT_POWER_LIMIT_W;
    if(((HAL_GetTick() - chargingPowerLimit.info.last_rx) <= POWER_LIMIT_COMM_TIMEOUT) && (chargingPowerLimit.info.last_rx != 0))
    {
        powerLimit = chargingPowerLimit.data;
        
        // Clamp power limit
        if(powerLimit > ABSOLUTE_POWER_LIMIT_W)
        {
            powerLimit = ABSOLUTE_POWER_LIMIT_W;
        }
        else if(powerLimit < 0.0f)
        {
            powerLimit = 0.0f;
        }
    }

    return powerLimit;
}

static float getCurrentLimit(float powerLimit, float packVoltage)
{
    // Calculate the current request
    float currentLimit = powerLimit / packVoltage;

    // Clamp current limit
    if(currentLimit > MAX_CHARGE_CURRENT_A)
    {
        currentLimit = MAX_CHARGE_CURRENT_A;
    }
    else if(currentLimit < 0.0f)
    {
        currentLimit = 0.0f;
    }

    return currentLimit;
}

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DEFINITIONS ==================== */
/* ==================================================================== */

void initChargerTask()
{

}

void runChargerTask()
{
    // Input telem and charger data
    cellMonitorTaskData_S cellTaskData;
    packMonitorTaskData_S packTaskData;

    vTaskSuspendAll();
    cellTaskData = publicCellMonitorTaskData;
    packTaskData = publicPackMonitorTaskData;
    xTaskResumeAll();

    // Check if the charger is connected
    bool chargerConnected = (chargerStatusByte.info.last_rx != 0) && ((HAL_GetTick() - chargerStatusByte.info.last_rx) < ELCON_CHARGER_COMM_TIMEOUT);

    // Get charger power limit
    taskData.chargerPowerLimit = getPowerLimit();

    // Charger state machine

    switch(taskData.chargerState)
    {
        case CHARGER_STATE_DISCONNECTED:
        {
            taskData.chargerVoltageSetpoint = 0.0f;
            taskData.chargerCurrentSetpoint = 0.0f;

            if(chargerConnected)
            {
                if(cellTaskData.maxCellVoltage >= MAX_CELL_VOLTAGE)
                {
                    float cellImbalance = (cellTaskData.maxCellVoltage - cellTaskData.minCellVoltage);
                    if(cellImbalance >= CELL_IMBALANCE_THRESHOLD)
                    {
                        taskData.chargerState = CHARGER_STATE_BALANCING;
                    }
                    else
                    {
                        taskData.chargerState = CHARGER_STATE_COMPLETE;
                    }
                }
                else if(cellTaskData.maxCellVoltage >= CELL_VOLTAGE_CV_THRES)
                {
                    taskData.chargerState = CHARGER_STATE_CONSTANT_VOLTAGE;
                }
                else
                {
                    taskData.chargerState = CHARGER_STATE_CONSTANT_CURRENT;
                }
            }
            break;
        }
        case CHARGER_STATE_CONSTANT_CURRENT:
        {
            // Request max voltage and current under the determined power limit
            taskData.chargerCurrentSetpoint = getCurrentLimit(taskData.chargerPowerLimit, packTaskData.packVoltage);
            taskData.chargerVoltageSetpoint = MAX_CHARGE_VOLTAGE_V;

            if(chargerConnected)
            {
                if(cellTaskData.maxCellVoltage >= CELL_VOLTAGE_CV_THRES)
                {
                    taskData.chargerState = CHARGER_STATE_CONSTANT_VOLTAGE;
                }
            }
            else
            {
                taskData.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        case CHARGER_STATE_CONSTANT_VOLTAGE:
        {
            // Request max voltage and current under the determined power limit
            float currentLimit = getCurrentLimit(taskData.chargerPowerLimit, MAX_CHARGE_VOLTAGE_V);

            // Get a scaling factor according to difference between highest cell voltage and max cell voltage 
            float deratingFactor = (MAX_CELL_VOLTAGE - cellTaskData.maxCellVoltage) / (MAX_CELL_VOLTAGE - CELL_VOLTAGE_CV_THRES);
            if(deratingFactor > 1.0f)
            {
                deratingFactor = 1.0f;
            }
            else if(deratingFactor < 0.0f)
            {
               deratingFactor = 0.0f; 
            }

            taskData.chargerCurrentSetpoint = (deratingFactor * currentLimit);
            taskData.chargerVoltageSetpoint = MAX_CHARGE_VOLTAGE_V;

            if(chargerConnected)
            {
                if(cellTaskData.maxCellVoltage >= MAX_CELL_VOLTAGE)
                {
                    float cellImbalance = (cellTaskData.maxCellVoltage - cellTaskData.minCellVoltage);
                    if(cellImbalance >= CELL_IMBALANCE_THRESHOLD)
                    {
                        // Rest?
                        taskData.chargerState = CHARGER_STATE_BALANCING;
                    }
                    else
                    {
                        taskData.chargerState = CHARGER_STATE_COMPLETE;
                    }
                }
                else if(cellTaskData.maxCellVoltage <= (CELL_VOLTAGE_CV_THRES - CELL_VOLTAGE_CV_HYS))
                {
                    taskData.chargerState = CHARGER_STATE_CONSTANT_CURRENT;
                }
            }
            else
            {
                taskData.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        case CHARGER_STATE_BALANCING:
        {
            taskData.chargerCurrentSetpoint = 0.0f;
            taskData.chargerVoltageSetpoint = 0.0f;
            

            if(chargerConnected)
            {
                float cellImbalance = (cellTaskData.maxCellVoltage - cellTaskData.minCellVoltage);
                if((cellTaskData.maxCellVoltage <= CELL_VOLTAGE_CV_THRES) || (cellImbalance <= CELL_IMBALANCE_THRESHOLD))
                {
                    taskData.chargerState = CHARGER_STATE_CONSTANT_VOLTAGE;
                }
            }
            else
            {
                taskData.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        case CHARGER_STATE_COMPLETE:
        {
            taskData.chargerCurrentSetpoint = 0.0f;
            taskData.chargerVoltageSetpoint = 0.0f;

            if(!chargerConnected)
            {
                taskData.chargerState = CHARGER_STATE_DISCONNECTED;
            }
            break;
        }
        default:
        {
            taskData.chargerState = CHARGER_STATE_DISCONNECTED;
            break;
        }
    }

    if(taskData.chargerState != CHARGER_STATE_DISCONNECTED)
    {
        sendChargerMessage(taskData.chargerVoltageSetpoint, taskData.chargerCurrentSetpoint, true);
        taskData.chargerVoltage = chargerVoltageSetPoint_V.data;
        taskData.chargerCurrent = chargerCurrentSetPoint_A.data;
        memcpy(&taskData.chargerStatus, &chargerStatusByte.data, 1);
    }
    else
    {
        taskData.chargerVoltage = 0.0f;
        taskData.chargerCurrent = 0.0f;
        memset(&taskData.chargerStatus, 0x00, 1);
    }

    vTaskSuspendAll();
    publicChargerTaskData = taskData;
    xTaskResumeAll();
}
