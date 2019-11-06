#include "main.h"
#include "numlcd.h"

void lcd_init(lcd_struct_t* lcd){

	lcd->lastOp = lcd_lastOpread;

	if(lcd->interfaceSize == lcd_size4){
		lcd->interfaceSize = lcd_size8;
		// setup sequence for 4 bit bus
		lcd_write(lcd,lcd_command,0b00110000);
		HAL_Delay(10);
		lcd_write(lcd,lcd_command,0b00110000);
		HAL_Delay(10);
		lcd_write(lcd,lcd_command,0b00110000);
		HAL_Delay(10);
		lcd_write(lcd,lcd_command,0b00100000);
		HAL_Delay(10);

		lcd->interfaceSize = lcd_size4;
		lcd_write(lcd,lcd_command,0b00101000);
		HAL_Delay(10);
		//display on and cursor and blinking cursor
		lcd_write(lcd,lcd_command,0b00001111);
		HAL_Delay(10);

		lcd_write(lcd,lcd_command,0b00000110);
		HAL_Delay(50);
		lcd_write(lcd,lcd_command,0b00000001);
		HAL_Delay(50);
	}

}


void lcd_write(lcd_struct_t* lcd,enum lcd_opType op,uint8_t data){

	GPIO_InitTypeDef GPIO_InitStruct = {0};

	//Setup the bus for write
	//make all data lines output
	int i = 0;
	for(;i<8 && lcd->lastOp == lcd_lastOpread;i++){
		//clear bits
		if(lcd->dataPorts[i] == 0) continue;
		GPIO_InitStruct.Pin = lcd->dataPins[i];
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
		HAL_GPIO_Init(lcd->dataPorts[i], &GPIO_InitStruct);
	}
	lcd->lastOp = lcd_lastOpWrite;

	//clear the rw line
	HAL_GPIO_WritePin(lcd->rwPort,lcd->rwPin,0);

	//set the write type
	HAL_GPIO_WritePin(lcd->rsPort,lcd->rsPin,op == lcd_data);

	if(lcd->interfaceSize == lcd_size4){
		int j = 0;
		for(;j<2;j++){
			//drive the e line high
			HAL_GPIO_WritePin(lcd->ePort,lcd->ePin,1);

			//write the data
			for(i=4;i<8;i++){
				HAL_GPIO_WritePin(lcd->dataPorts[i],lcd->dataPins[i],data & (1<<i));
			}

			HAL_Delay(1);
			//drive the e line low
			HAL_GPIO_WritePin(lcd->ePort,lcd->ePin,0);
			HAL_Delay(1);
			data = data<<4;
		}

	}
	else if(lcd->interfaceSize == lcd_size8){
		//drive the e line high
		HAL_GPIO_WritePin(lcd->ePort,lcd->ePin,1);

		//write the data
		for(i=0;i<8;i++){
			if(lcd->dataPorts[i] == 0) continue;
			HAL_GPIO_WritePin(lcd->dataPorts[i],lcd->dataPins[i],data & (1<<i));
		}

		HAL_Delay(1);
		//drive the e line low
		HAL_GPIO_WritePin(lcd->ePort,lcd->ePin,0);
		HAL_Delay(1);
	}
}

void lcd_setCursor(lcd_struct_t* lcd,uint8_t x,uint8_t y){
	uint8_t poss=0;
	switch(y){
		case 0 : poss = 0;	  break;
		case 1 : poss = 0x40; break;
		case 2 : poss = 0x14; break;
		case 3 : poss = 0x54; break;
	}
	poss +=x;
	lcd_write(lcd,lcd_command,0b10000000 | poss);
}
