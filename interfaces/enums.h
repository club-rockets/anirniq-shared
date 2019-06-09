/*
 * enums.h
 *
 *  Created on: May 24, 2019
 *      Author: Hugo Lemoine
 */

#ifndef INTERFACES_ENUMS_H_
#define INTERFACES_ENUMS_H_


typedef enum rocket_state {
	INITIALISATION,
	STANDBY_ON_PAD,
	LAUNCH,
	POWERED_ASCENT,
	ENGINE_BURNOUT,
	COASTING_ASCENT,
	APOGEE_REACHED,
	DROGUE_DEPLOYMENT,
	DROGUE_DESCENT,
	MAIN_DEPLOYMENT,
	MAIN_DESCENT,
	LANDING,
	RECOVERY,
	PICKEDUP
} rocket_state;

typedef enum charge_state {
	NO_CHARGE,
	DROGUE_SENSE,
	MAIN_SENSE,
	BOTH_SENSE
} charge_state;

enum can_board{
	EMERGENCY = 0,
	MISSION,
	COMMUNICATION,
	ACQUISITION,
	MOTHERBOARD
};

enum comState{
	PAD_TRANSMISSION,
	LAUNCH_TRANSMISSION
};

enum ejectionCommand{
	NO_EJECTION = 0,
	EJECTION
};

#endif /* INTERFACES_ENUMS_H_ */
