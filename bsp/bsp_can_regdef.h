/*
 * bsp_can_regdef.h
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

#include "bsp_can.h"
#include "stdint.h"

#include "id.h"

#define BOARD_EMERGENCY_ID_SHIFTED     (0 << BOARD_ID_SHIFT)
#define BOARD_MISSION_ID_SHIFTED       (1 << BOARD_ID_SHIFT)
#define BOARD_COMMUNICATION_ID_SHIFTED (2 << BOARD_ID_SHIFT)
#define BOARD_ACQUISITION_ID_SHIFTED   (3 << BOARD_ID_SHIFT)
#define BOARD_MOTHERBOARD_ID_SHIFTED   (4 << BOARD_ID_SHIFT)

extern volatile can_reg_t can_emergencyRegisters[];
extern volatile can_reg_t can_missionRegisters[];
extern volatile can_reg_t can_communicationRegisters[];
extern volatile can_reg_t can_acquisitionRegisters[];
extern volatile can_reg_t can_motherboardRegisters[];

extern volatile can_reg_t* can_registers[];

extern const uint8_t can_registersSize[];

#endif //_BSP_CAN_REGDEF
