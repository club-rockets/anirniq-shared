/*
 * BSP_can_regDef.h
 *
 *  Created on: 17 mars 2019
 *      Author: Marc-Andre Denis
 *
 *	Contains can intervace registers definition
 *	To be uset only by BSP_CAN.c
 *
 */
#ifndef _BSP_CAN_REGDEF
#define _BSP_CAN_REGDEF

#include "BSP_CAN.h"
#include "stdint.h"

#define BOARD_EMERGENCY_ID_SHIFTED		(0<<BOARD_ID_SHIFT)
#define BOARD_MISSION_ID_SHIFTED		(1<<BOARD_ID_SHIFT)
#define BOARD_COMMUNICATION_ID_SHIFTED	(2<<BOARD_ID_SHIFT)
#define BOARD_ACQUISITION_ID_SHIFTED	(3<<BOARD_ID_SHIFT)
#define BOARD_MOTHERBOARD_ID_SHIFTED 	(4<<BOARD_ID_SHIFT)

#define CAN_EMERGENCY_TEST_INDEX 0

extern volatile can_reg_t can_emergencyRegisters[];

#define CAN_MISSION_HEARTBEAT_INDEX 0

extern volatile can_reg_t can_missionRegisters[];

#define CAN_COMMUNICATION_HEARTBEAT_INDEX 0

extern volatile can_reg_t can_communicationRegisters[];

#define CAN_ACQUISITION_HEARTBEAT_INDEX 0

extern volatile can_reg_t can_acquisitionRegisters[];

#define CAN_MOTHERBOARD_HEARTBEAT_INDEX 0

extern volatile can_reg_t can_motherboardRegisters[];

extern volatile can_reg_t* can_registers[];

extern const uint8_t can_registersSize[];


#endif //_BSP_CAN_REGDEF
