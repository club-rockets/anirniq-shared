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

volatile can_reg_t can_emergencyRegisters[] = {
		//one random value for now
		{
				.name = "test",
				.lastTick = 0,
				.changeCallback = 0,
				.data = {0}
		}
};

volatile can_reg_t can_missionRegisters[] = {
		{
				.name = "heartBeat",
				.lastTick = 0,
				.changeCallback = 0,
				.data = {0}
		}
};

volatile can_reg_t can_communicationRegisters[] = {
		{
				.name = "heartBeat",
				.lastTick = 0,
				.changeCallback = 0,
				.data = {0}
		}
};

volatile can_reg_t can_acquisitionRegisters[] = {
		{
				.name = "heartBeat",
				.lastTick = 0,
				.changeCallback = 0,
				.data = {0}
		}
};

volatile can_reg_t can_motherboardRegisters[] = {
		{
				.name = "heartBeat",
				.lastTick = 0,
				.changeCallback = 0,
				.data = {0}
		},
		{
			.name = "batteryVoltage",
			.lastTick = 0,
			.changeCallback = 0,
			.data = {0}
		}
};

volatile can_reg_t* can_registers[] = {
		can_emergencyRegisters,
		can_missionRegisters,
		can_communicationRegisters,
		can_acquisitionRegisters,
		can_motherboardRegisters,
};

const uint8_t can_registersSize[] = {
		sizeof(can_emergencyRegisters)/sizeof(can_reg_t),
		sizeof(can_missionRegisters)/sizeof(can_reg_t),
		sizeof(can_communicationRegisters)/sizeof(can_reg_t),
		sizeof(can_acquisitionRegisters)/sizeof(can_reg_t),
		sizeof(can_motherboardRegisters)/sizeof(can_reg_t)
};


#endif //_BSP_CAN_REGDEF
