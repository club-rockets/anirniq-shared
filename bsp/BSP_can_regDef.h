/*
 * BSP_can_regDef.h
 *
 * Created on: 17 mars 2019
 * Author: Marc-Andre Denis
 *
 * Contains CAN interface registers definition
 * To be uset only by BSP_CAN.c
 *
 */
#ifndef _BSP_CAN_REGDEF
#define _BSP_CAN_REGDEF

#include "BSP_CAN.h"
#include "stdint.h"

#define BOARD_EMERGENCY_ID_SHIFTED     (0 << BOARD_ID_SHIFT)
#define BOARD_MISSION_ID_SHIFTED       (1 << BOARD_ID_SHIFT)
#define BOARD_COMMUNICATION_ID_SHIFTED (2 << BOARD_ID_SHIFT)
#define BOARD_ACQUISITION_ID_SHIFTED   (3 << BOARD_ID_SHIFT)
#define BOARD_MOTHERBOARD_ID_SHIFTED   (4 << BOARD_ID_SHIFT)

/** CAN Message IDs **/
/* Emergency */
#define CAN_EMERGENCY_TEST_INDEX 0

/* Mission */
#define CAN_MISSION_STATUS_INDEX        0
#define CAN_MISSION_ROCKET_STATUS_INDEX 1
#define CAN_MISSION_CHARGE_STATUS_INDEX 2

/* Communication */
#define CAN_COMMUNICATION_STATUS_INDEX 0

#define CAN_COMMUNICATION_CONTROL_SLEEP_INDEX        1
#define CAN_COMMUNICATION_CONTROL_EJECT_DROGUE_INDEX 2
#define CAN_COMMUNICATION_CONTROL_EJECT_MAIN_INDEX   3

/* Acquisition */
#define CAN_ACQUISITION_STATUS_INDEX            0

#define CAN_ACQUISITION_AIR_PRESSURE_INDEX    1
#define CAN_ACQUISITION_AIR_TEMPERATURE_INDEX 2

#define CAN_ACQUISITION_ALTITUDE_INDEX          3
#define CAN_ACQUISITION_VERTICAL_VELOCITY_INDEX 4

#define CAN_ACQUISITION_ACCELERATION_X_INDEX 5
#define CAN_ACQUISITION_ACCELERATION_Y_INDEX 6
#define CAN_ACQUISITION_ACCELERATION_Z_INDEX 7

#define CAN_ACQUISITION_ATTITUDE_X_INDEX 8
#define CAN_ACQUISITION_ATTITUDE_Y_INDEX 9
#define CAN_ACQUISITION_ATTITUDE_Z_INDEX 10

#define CAN_ACQUISITION_MAGNETIC_FIELD_X_INDEX 11
#define CAN_ACQUISITION_MAGNETIC_FIELD_Y_INDEX 12
#define CAN_ACQUISITION_MAGNETIC_FIELD_Z_INDEX 13

#define CAN_ACQUISITION_IMU_TEMPERATURE_INDEX 14

#define CAN_ACQUISITION_GPS_LAT_INDEX 15
#define CAN_ACQUISITION_GPS_LON_INDEX 16

/* Motherboard */
#define CAN_MOTHERBOARD_STATUS_INDEX 0

#define CAN_MOTHERBOARD_3V3_VOLTAGE  1
#define CAN_MOTHERBOARD_3V3_AMPERAGE 2

#define CAN_MOTHERBOARD_5V_VOLTAGE  3
#define CAN_MOTHERBOARD_5V_AMPERAGE 4

#define CAN_MOTHERBOARD_VBATT_VOLTAGE  5
#define CAN_MOTHERBOARD_VBATT_AMPERAGE 6

extern volatile can_reg_t can_emergencyRegisters[];
extern volatile can_reg_t can_missionRegisters[];
extern volatile can_reg_t can_communicationRegisters[];
extern volatile can_reg_t can_acquisitionRegisters[];
extern volatile can_reg_t can_motherboardRegisters[];

extern volatile can_reg_t* can_registers[];

extern const uint8_t can_registersSize[];

#endif //_BSP_CAN_REGDEF
