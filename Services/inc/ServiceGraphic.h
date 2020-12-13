

#ifndef SERVICE_GRAPHIC_H
#define SERVICE_GRAPHIC_H

/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"



typedef enum {
	GRAPHIC_MSG_TYPE_NOP,
	GRAPHIC_MSG_TYPE_CLEAR,
	GRAPHIC_MSG_TYPE_FLUSH,
	GRAPHIC_MSG_TYPE_REALTIME_FLUSH_ON,
	GRAPHIC_MSG_TYPE_REALTIME_FLUSH_OFF,
	GRAPHIC_MSG_TYPE_TEXTOUT,
	GRAPHIC_MSG_TYPE_PIXON
}GraphicMsgType;

typedef struct GraphicMessage{
	void *selfAddr;
	unsigned int type;
	void *argsList;
}GraphicMessage;

typedef struct GraphicTextOutInfo{
	unsigned int x;
	unsigned int y;
	unsigned int area_width;
	unsigned int area_height;
	unsigned int font_size;
	unsigned int font_color;
	unsigned char *text;
}GraphicTextOutArgs;

QueueHandle_t GraphicQueue;

void vServiceGraphic( void *pvParameters );

#endif
