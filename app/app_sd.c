
#include "app_sd.h"
#include "fatfs.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"

#include <stdio.h>
#include <string.h>

extern uint8_t retSD;    /* Return value for SD */
extern char SDPath[4];   /* SD logical drive path */
extern FATFS SDFatFS;    /* File system object for SD logical drive */
extern FIL SDFile;       /* File object for SD */

extern osThreadId app_SDHandle;

QueueHandle_t sd_queue;
uint8_t qbuff[SD_QUEUE_SIZE * SD_QUEUE_BLOCK_SIZE];
StaticQueue_t sq_queueBuff;


uint32_t createDir(char* path);

void stk_SD(void const * argument){

	sd_queue = xQueueCreateStatic(SD_QUEUE_SIZE,SD_QUEUE_BLOCK_SIZE,qbuff,&sq_queueBuff);

	//this string contains the path to the currently used file
	char filePath[16] = {0};

	uint32_t fileCounter = 0;
	uint32_t sizeCounter = 0;
	uint32_t syncTimer = 0;

	struct sd_log rxData = {0};
	BaseType_t queueRet;

	if( f_mount(&SDFatFS,SDPath,1) != FR_OK){
		//the card is not present, suspend the task
		vTaskSuspend(0);
	}

	//create a new directory to avoid overwriting old data
	uint32_t dirCounter = createDir(filePath);


	sprintf(filePath,"%03lu/%03lu.txt",dirCounter,++fileCounter);
	//open the new file
	f_open(&SDFile,filePath,FA_CREATE_ALWAYS|FA_WRITE);

	float a;
	int32_t b;

	while(1){

		queueRet = xQueueReceive(sd_queue,&rxData,5000);
		//read from q here

		if(queueRet == pdTRUE){
			//write data
			switch(rxData.type){
			case type_INT:
				sizeCounter += f_printf(&SDFile,"%s\t%lu\t%ld\n",rxData.title,rxData.time,rxData.data.INT);
				break;
			case type_UINT:
				sizeCounter += f_printf(&SDFile,"%s\t%lu\t%lu\n",rxData.title,rxData.time,rxData.data.UINT);
				break;
			case type_FLOAT:
				//the fatfs library does not support floats
				b = (int32_t)(rxData.data.FLOAT);
				a = (rxData.data.FLOAT);
				sizeCounter += f_printf(&SDFile,"%s\t%lu\t%ld",rxData.title,rxData.time,b);
				a-=(float)b;
				if(a<0) a = 1-a;
				b = (int32_t)(a*1000.0);
				sizeCounter += f_printf(&SDFile,".%ld\n",b);
			}

		}



		//flush every 10sec
		if(osKernelSysTick() - syncTimer >= 10000){

			syncTimer = osKernelSysTick();
			f_sync(&SDFile);
		}

		//if file too large create a new one (10mb)
		if(sizeCounter > 10000000){
			sizeCounter = 0;
			//close the current file
			f_close(&SDFile);
			//create a new file name
			sprintf(filePath,"%03lu/%03lu.txt",dirCounter,++fileCounter);
			//open the new file
			f_open(&SDFile,filePath,FA_CREATE_ALWAYS|FA_WRITE);
		}
	}

}


uint32_t createDir(char* path){
	//check for a unused directory name
	uint32_t dirCounter = 0;

		do{
			dirCounter += 1;
			//check if 00x directory already exist
			sprintf(path,"%03lu",dirCounter);
		}while( f_stat(path,0) == FR_OK && dirCounter <= 999);

	//create a new directory
	f_mkdir(path);

	return dirCounter;
}

//add the value to the sd queue, name : data
uint32_t sd_writeInt(char* name,int32_t data){
	struct sd_log dataStr;
	dataStr.data.INT = data;
	dataStr.time = osKernelSysTick();
	dataStr.type = type_INT;
	dataStr.title = name;

	return xQueueSend(sd_queue,&dataStr,0);
}

uint32_t sd_writeUint(char* name,uint32_t data){
	struct sd_log dataStr;
	dataStr.data.UINT = data;
	dataStr.time = osKernelSysTick();
	dataStr.type = type_UINT;
	dataStr.title = name;

	return xQueueSend(sd_queue,&dataStr,0);
}

uint32_t sd_writeFloat(char* name,float data){
	struct sd_log dataStr;
	dataStr.data.FLOAT = data;
	dataStr.time = osKernelSysTick();
	dataStr.type = type_FLOAT;
	dataStr.title = name;

	return xQueueSend(sd_queue,&dataStr,0);
}
