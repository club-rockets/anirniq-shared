/*
 * BSP_can_regDef.c
 *
 * Created on: 17 mars 2019
 * Author: Marc-Andre Denis
 *
 * Contains CAN interface registers definition
 *
 */

#include "stdint.h"
#include "bsp_can_regdef.h"

#define DEF_CAN_REGISTER(reg_name) \
    { \
        .name = reg_name, \
        .lastTick = 0, \
        .changeCallback = 0, \
        .data = { 0 } \
    }

volatile can_reg_t can_emergencyRegisters[] = {
    DEF_CAN_REGISTER("test")
};

volatile can_reg_t can_missionRegisters[] = {
    DEF_CAN_REGISTER("status"),
    DEF_CAN_REGISTER("rocket_status"),
    DEF_CAN_REGISTER("charge_status")
};

volatile can_reg_t can_communicationRegisters[] = {
    DEF_CAN_REGISTER("status"),
    DEF_CAN_REGISTER("command_sleep"),
    DEF_CAN_REGISTER("command_eject_drogue"),
    DEF_CAN_REGISTER("command_eject_main")
};

volatile can_reg_t can_acquisitionRegisters[] = {
    DEF_CAN_REGISTER("status"),
    DEF_CAN_REGISTER("air_pressure"),
    DEF_CAN_REGISTER("air_temperature"),
    DEF_CAN_REGISTER("altitude"),
    DEF_CAN_REGISTER("vertical_velocity"),
    DEF_CAN_REGISTER("acceleration_x"),
    DEF_CAN_REGISTER("acceleration_y"),
    DEF_CAN_REGISTER("acceleration_z"),
    DEF_CAN_REGISTER("attitude_x"),
    DEF_CAN_REGISTER("attitude_y"),
    DEF_CAN_REGISTER("attitude_z"),
    DEF_CAN_REGISTER("magnetic_field_x"),
    DEF_CAN_REGISTER("magnetic_field_y"),
    DEF_CAN_REGISTER("magnetic_field_z"),
    DEF_CAN_REGISTER("imu_temperature"),
    DEF_CAN_REGISTER("gps_lat"),
    DEF_CAN_REGISTER("gps_lon")
};

volatile can_reg_t can_motherboardRegisters[] = {
    DEF_CAN_REGISTER("status"),
    DEF_CAN_REGISTER("3v3_voltage"),
    DEF_CAN_REGISTER("3v3_amperage"),
    DEF_CAN_REGISTER("5v_voltage"),
    DEF_CAN_REGISTER("5v_amperage"),
    DEF_CAN_REGISTER("vbatt_voltage"),
    DEF_CAN_REGISTER("vbatt_amperage")
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
