#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "init.h"

#include "ServiceGraphic.h"

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

GraphicMessage GM;
GraphicTextOutArgs text1;
 
void vInit(){
	vTaskDelay(200);
	
	GM.selfAddr = &GM;
	GM.type = GRAPHIC_MSG_TYPE_CLEAR;
	xQueueSend(GraphicQueue, &(GM.selfAddr), ( TickType_t ) 0 );
	
	GM.type = GRAPHIC_MSG_TYPE_TEXTOUT;
	GM.argsList = &text1;
	text1.x = 1;
	text1.y = 1;
	text1.area_width = 100;
	text1.area_height = 16;
	text1.font_size = 12;
	text1.font_color = 255;
	text1.text = "Hello World";
	xQueueSend(GraphicQueue, &(GM.selfAddr) , ( TickType_t ) 0 );	
	
	//vTaskSuspend(NULL);
	vTaskDelete(NULL);
	for(;;){
		
	}
	
}