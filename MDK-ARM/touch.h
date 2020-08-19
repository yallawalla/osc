#ifndef __TOUCH_H
#define __TOUCH_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stm32f7xx_hal.h"


typedef struct {
	void(*up)(uint32_t, uint32_t);
	void(*down)(uint32_t, uint32_t);
	void(*move)(uint32_t,uint32_t);
	uint32_t 	detected;
	int32_t 	x,dx;
	int32_t 	y,dy;
} mouse;	

void mouseInit(uint32_t,uint32_t);
void mouseScan(void);


#endif	//__TOUCH_H
