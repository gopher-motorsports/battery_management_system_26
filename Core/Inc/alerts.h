#ifndef INC_ALERTS_H_
#define INC_ALERTS_H_

/* ==================================================================== */
/* ============================= INCLUDES ============================= */
/* ==================================================================== */

#include "timer.h"
#include "updateCellMonitorTask.h"
#include "updatePackMonitorTask.h"

/* ==================================================================== */
/* ============================= DEFINES ============================== */
/* ==================================================================== */

#define OVERVOLTAGE_WARNING_ALERT_SET_TIME_MS     0
#define OVERVOLTAGE_WARNING_ALERT_CLEAR_TIME_MS   0

#define OVERVOLTAGE_FAULT_ALERT_SET_TIME_MS       5000
#define OVERVOLTAGE_FAULT_ALERT_CLEAR_TIME_MS     5000

#define UNDERVOLTAGE_WARNING_ALERT_SET_TIME_MS    0
#define UNDERVOLTAGE_WARNING_ALERT_CLEAR_TIME_MS  0

#define UNDERVOLTAGE_FAULT_ALERT_SET_TIME_MS      5000
#define UNDERVOLTAGE_FAULT_ALERT_CLEAR_TIME_MS    5000

#define MAX_CELL_IMBALANCE_V                      0.1f
#define CELL_IMBALANCE_ALERT_SET_TIME_MS          1000
#define CELL_IMBALANCE_ALERT_CLEAR_TIME_MS        1000

#define OVERTEMPERATURE_WARNING_ALERT_SET_TIME_MS   500
#define OVERTEMPERATURE_WARNING_ALERT_CLEAR_TIME_MS 500

#define OVERTEMPERATURE_FAULT_ALERT_SET_TIME_MS     5000
#define OVERTEMPERATURE_FAULT_ALERT_CLEAR_TIME_MS   5000

#define SDC_FAULT_ALERT_SET_TIME_MS   0
#define SDC_FAULT_ALERT_CLEAR_TIME_MS 0

#define BAD_VOLTAGE_SENSE_STATUS_ALERT_SET_TIME_MS    2000
#define BAD_VOLTAGE_SENSE_STATUS_ALERT_CLEAR_TIME_MS  2000

#define BAD_CELL_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS    1000
#define BAD_CELL_TEMP_SENSE_STATUS_ALERT_CLEAR_TIME_MS  1000

#define BAD_BOARD_TEMP_SENSE_STATUS_ALERT_SET_TIME_MS    1000
#define BAD_BOARD_TEMP_SENSE_STATUS_ALERT_CLEAR_TIME_MS  1000

// The minimum percent of cell temps that must be monitored to pass rules
#define MIN_PERCENT_CELL_TEMPS_MONITORED             25
#define INSUFFICIENT_TEMP_SENSOR_ALERT_SET_TIME_MS    5000
#define INSUFFICIENT_TEMP_SENSOR_ALERT_CLEAR_TIME_MS  5000

#define BMB_COMMUNICATION_FAILURE_ALERT_SET_TIME_MS   5000
#define BMB_COMMUNICATION_FAILURE_ALERT_CLEAR_TIME_MS 5000

#define PACK_OVERCURRENT_FAULT_ALERT_SET_TIME_MS    500
#define PACK_OVERCURRENT_FAULT_ALERT_CLEAR_TIME_MS  500

#define PACK_VOLTAGE_OUT_OF_RANGE_ALERT_SET_TIME_MS       500
#define PACK_VOLTAGE_OUT_OF_RANGE_ALERT_CLEAR_TIME_MS     500





// #define STACK_VS_SEGMENT_IMBALANCE_ALERT_SET_TIME_MS    1000
// #define STACK_VS_SEGMENT_IMBALANCE_ALERT_CLEAR_TIME_MS  1000

// #define CHARGER_OVERVOLTAGE_ALERT_SET_TIME_MS      2000
// #define CHARGER_OVERVOLTAGE_ALERT_CLEAR_TIME_MS    2000

// #define CHARGER_OVERCURRENT_ALERT_SET_TIME_MS      2000
// #define CHARGER_OVERCURRENT_ALERT_CLEAR_TIME_MS    2000

// #define CHARGER_VOLTAGE_MISMATCH_ALERT_SET_TIME_MS      2000
// #define CHARGER_VOLTAGE_MISMATCH_ALERT_CLEAR_TIME_MS    2000

// #define CHARGER_CURRENT_MISMATCH_ALERT_SET_TIME_MS      2000
// #define CHARGER_CURRENT_MISMATCH_ALERT_CLEAR_TIME_MS    2000

// #define CHARGER_DIAGNOSTIC_ALERT_SET_TIME_MS      2000
// #define CHARGER_DIAGNOSTIC_ALERT_CLEAR_TIME_MS    2000

/* ==================================================================== */
/* ========================= ENUMERATED TYPES========================== */
/* ==================================================================== */

typedef enum
{
    ALERT_CLEARED = 0,	// Indicates alert has not been set yet
    ALERT_LATCHED,		// Indicates alert is no longer present, but was at one point (only applies for latching alerts)
    ALERT_SET			// Indicates alert is currently set
} AlertStatus_E;

typedef enum
{
    INFO_ONLY = 0,		// Only used for info no actual response
    DISABLE_BALANCING,	// Disables cell balancing
    EMERGENCY_BLEED,	// Emergencly bleed all the cells down
    DISABLE_CHARGING,	// Disable charging 
    LIMP_MODE,			// Limit max current out of pack
    BMS_FAULT,			// Set BMS fault to open shutdown circuit
    NUM_ALERT_RESPONSES
} AlertResponse_E;

// Alerts are bit encoded before being sent over GopherCAN
// Stored as BBBB bbbb
// Upper nibble stores info about which GopherCAN param the alert is in
// Lower nibble stores info about which bit of the uint8_t the alert is in

typedef enum {
  // Byte 0
  OVERVOLTAGE_WARNING_ALERT           = (0U << 4) | 0U,
  UNDERVOLTAGE_WARNING_ALERT          = (0U << 4) | 1U,
  OVERVOLTAGE_FAULT_ALERT             = (0U << 4) | 2U,
  UNDERVOLTAGE_FAULT_ALERT            = (0U << 4) | 3U,
  CELL_IMBALANCE_ALERT                = (0U << 4) | 4U,
  OVERTEMP_WARNING_ALERT              = (0U << 4) | 5U,
  OVERTEMP_FAULT_ALERT                = (0U << 4) | 6U,
  BAD_VOLTAGE_SENSE_STATUS_ALERT      = (0U << 4) | 7U,

  // Byte 1
  BAD_CELL_TEMP_SENSE_STATUS_ALERT    = (1U << 4) | 0U,
  BAD_BOARD_TEMP_SENSE_STATUS_ALERT   = (1U << 4) | 1U,
  INSUFFICIENT_TEMP_SENSORS_ALERT     = (1U << 4) | 2U,
  TELEMETRY_COMMUNICATION_ALERT       = (1U << 4) | 3U,

} CELL_MONITOR_GCAN_ALERT_ENCODE;

/* ==================================================================== */
/* ============================== STRUCTS============================== */
/* ==================================================================== */

typedef struct
{
    const char* alertName;
    // Whether the alert is latching or not
    const bool latching;
    // The current status of the alert
    AlertStatus_E alertStatus;
    // The timer used for qualifying the alert set/clear condition
    Timer_S alertTimer;
    // The time in ms required for the alert to be set
    const uint32_t setTime_MS;
    // The time in ms required for the alert to clear
    const uint32_t clearTime_MS;
    // Function pointer used to determine whether the alert is present or not
    bool alertConditionPresent;
    // The number of alert responses for this alert
    const uint32_t numAlertResponse;
    // Array of alert responses
    const AlertResponse_E* alertResponse;
    // Alerts are bit encoded before sending over GopherCAN
    uint8_t gcanAlertEncode;
} Alert_S;

/* ==================================================================== */
/* ====================== FUNCTION POINTER TYPES ====================== */
/* ==================================================================== */

typedef bool (*cellMonitorAlertCondition)(cellMonitorTaskData_S* taskData);
typedef bool (*packMonitorAlertCondition)(packMonitorTaskData_S* taskData);

/* ==================================================================== */
/* ======================= EXTERNAL VARIABLES ========================= */
/* ==================================================================== */

// Array of all alert structs
extern Alert_S* cellMonitorAlerts[];
extern Alert_S* packMonitorAlerts[];

// Function arrays
extern cellMonitorAlertCondition cellMonitorAlertConditionArray[];
extern packMonitorAlertCondition packMonitorAlertConditionArray[];

// The total number of alerts for each alert task
extern const uint32_t NUM_CELL_MONITOR_ALERTS;
extern const uint32_t NUM_PACK_MONITOR_ALERTS;

/* ==================================================================== */
/* =================== GLOBAL FUNCTION DECLARATIONS =================== */
/* ==================================================================== */

/*!
  @brief   Get the status of any given alert
  @param   alert - The alert data structure whose status to read
  @return  The current status of the alert
*/
AlertStatus_E getAlertStatus(Alert_S* alert);

/*!
  @brief   Run the alert monitor to update the status of the alert
  @param   alert - The Alert data structure
*/
void runAlertMonitor(Alert_S* alert);

void setBmsFault(bool set);

#endif /* INC_ALERTS_H_ */
