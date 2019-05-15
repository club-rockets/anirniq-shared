/*
 * BSP_CAN.c
 *
 *  Created on: 15 mars 2019
 *      Author: Marc-Andre
 */

#include "BSP_CAN.H"
#include "BSP_can_regDef.h"
#include "stm32f4xx_hal.h"
#include "main.h"
#include "cmsis_os.h"
#include "string.h"

canInstance_t can1Instance = {0};

uint32_t __attribute__((weak)) can_canInit(){

	//initialise IO's
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	__HAL_RCC_GPIOB_CLK_ENABLE();

	/*Configure GPIO pins : PB8 PB9 */
	GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF9_CAN1;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	can1Instance.instance = CAN1;
	can1Instance.debugFreeze = 0;
	can1Instance.opMode = loopback;
	can1Instance.baudPrescaler = 50;
	can1Instance.timeQuanta1 = 8;
	can1Instance.timeQuanta2 = 1;
	can1Instance.timeReSync = 1;

	canInit(&can1Instance);
	//init interruption for fan 1 fifo 0
	can1Fifo0InitIt(&can1Instance);
	can1Fifo0RegisterCallback(can_regUpdateCallback);

	//init filters for boards

	canFilter_t filter = {0};

	filter.mask11.mask0 = BOARD_ID_MASK;
	filter.mask11.ID0 = BOARD_EMERGENCY_ID_SHIFTED;
	filter.mask11.mask1 = BOARD_ID_MASK;
	filter.mask11.ID1 = BOARD_MISSION_ID_SHIFTED;
	canSetFilter(&can1Instance,&filter,mask11Bit,0,0);

	filter.mask11.mask0 = BOARD_ID_MASK;
	filter.mask11.ID0 = BOARD_COMMUNICATION_ID_SHIFTED;
	filter.mask11.mask1 = BOARD_ID_MASK;
	filter.mask11.ID1 = BOARD_ACQUISITION_ID_SHIFTED;
	canSetFilter(&can1Instance,&filter,mask11Bit,1,0);

	filter.mask11.mask0 = BOARD_ID_MASK;
	filter.mask11.ID0 = BOARD_MOTHERBOARD_ID_SHIFTED;
	filter.mask11.mask1 = BOARD_ID_MASK;
	filter.mask11.ID1 = BOARD_MOTHERBOARD_ID_SHIFTED;

	canSetFilter(&can1Instance,&filter,mask11Bit,2,0);
	NVIC_SetPriority(20,10);
	return 0;
}

//block if no mailbox is available
uint32_t can_canSetRegisterData(uint32_t index,can_regData_u* data){
	if(CAN_BOARD >= 5 || index >= can_registersSize[CAN_BOARD]){
		return 0;
	}
	can1Fifo0DeInitIt(&can1Instance);
	can_registers[CAN_BOARD][index].data = *data;
	can_registers[CAN_BOARD][index].lastTick = HAL_GetTick();
	can1Fifo0InitIt(&can1Instance);
	//send register
	while(!canSendPacket(&can1Instance,(CAN_BOARD_ID<<1)|(index<<(1+BOARD_ID_SIZE)),0,CAN_REG_DATA_SIZE,data));

	//call callback
	if(can_registers[CAN_BOARD][index].changeCallback){
		can_registers[CAN_BOARD][index].changeCallback(CAN_BOARD,index);
	}

	return 1;
}


uint32_t can_getRegisterData(enum can_board board, uint32_t index, can_regData_u* reg){

	//since this is not an atomic operation,
	//make sure that the data was not modified by an interrupt during reading
	can1Fifo0DeInitIt(&can1Instance);
	*reg = can_registers[board][index].data;
	can1Fifo0InitIt(&can1Instance);

	return 1;
}

uint32_t can_getRegisterTimestamp(enum can_board board, uint32_t index){

	return can_registers[board][index].lastTick;
}

uint32_t can_setRegisterCallback(enum can_board board, uint32_t index, void (*callback)(uint32_t,uint32_t)){

	can_registers[board][index].changeCallback = callback;
	return 1;
}



void can_regUpdateCallback(void){
	canRXpacket_t packet = {0};
	uint32_t board,id;

	can1SfFifo0Get(&packet);
	board = ((packet.STID) & BOARD_ID_MASK) >> BOARD_ID_SHIFT;
	id = ((packet.STID) & MESSAGE_ID_MASK) >> (BOARD_ID_SHIFT+BOARD_ID_SIZE);

	//if board and id valid
	if(board <= 4 && id < can_registersSize[board]){
		//copy the data in the register
		memcpy((void*)(&can_registers[board][id].data),(void*)(&packet.data),sizeof(can_regData_u));
		//update the tick value
		can_registers[board][id].lastTick = HAL_GetTick();
		//call the register change callback if non-null
		if(can_registers[board][id].changeCallback){
			can_registers[board][id].changeCallback(board,id);
		}
	}
}

uint32_t can_canSetRegisterLoopback(uint32_t board, uint32_t index,can_regData_u* data){
	if(board >= 5 || index >= can_registersSize[board]){
		return 0;
	}
	//send register
	return canSendPacket(&can1Instance,(board<<1)|(index<<(1+BOARD_ID_SIZE)),0,CAN_REG_DATA_SIZE,data);
}

//set local register without sending it to canbus (use for testing)
uint32_t can_canSetRegisterSelf(uint32_t board, uint32_t index,can_regData_u* data){
	if(board >= 5 || index >= can_registersSize[board]){
		return 0;
	}

	//disable can interrupt to avoid data corruption
	can1Fifo0DeInitIt(&can1Instance);
	can_registers[board][index].data = *data;
	can_registers[board][index].lastTick = HAL_GetTick();
	can1Fifo0InitIt(&can1Instance);

	if(can_registers[board][index].changeCallback){
		can_registers[board][index].changeCallback(board,index);
	}

	return 1;
}
