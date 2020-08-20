#include <stdlib.h>
#include "touch.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"
int test;

	void fup(mouse *m) {
		BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTCYAN);
		BSP_LCD_DrawCircle(m->x, m->y, 50);
	}
	void fdown(mouse *m) {
		BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTRED);
		BSP_LCD_DrawCircle(m->x, m->y, 50);
	}
	void fmove(mouse *m) {
		BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTYELLOW);
		BSP_LCD_DrawCircle(m->x, m->y, 50);
		BSP_LCD_DrawLine(m->x, m->y, m->x + 10*m->dx, m->y + 10*m->dy);
	}



mouse	m ={fup,fdown,fmove,0};
TS_StateTypeDef TS_State;

uint32_t	*pData, trigger;

void mouseInit(uint32_t x,uint32_t y) {
	BSP_TS_Init(x, y);
	pData=malloc(2*x*sizeof(uint32_t));
	HAL_ADC_Start_DMA(&hadc3, pData, 2*x);
}

void mouseScan(void) {
	BSP_TS_GetState(&TS_State);
	if(TS_State.touchDetected) {
		if(m.detected == 0) {
			m.x = TS_State.touchX[0];
			m.y = TS_State.touchY[0];
			m.dx = m.dy = 0;
			if(m.down)
				m.down(&m);
		} else {
			m.dx = TS_State.touchX[0] - m.x;
			m.dy = TS_State.touchY[0] - m.y;
			m.x = TS_State.touchX[0];
			m.y = TS_State.touchY[0];
		}
		m.detected = TS_State.touchDetected;
		if(m.move && (m.dx || m.dy))
			m.move(&m);
	} else {
		if(m.detected && m.up)
			m.up(&m);
		m.detected=0;
	}
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc) {
	trigger = hadc->DMA_Handle->StreamIndex;
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
//	HAL_ADC_Start_DMA(hadc, pData, BSP_LCD_GetXSize());
}
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
//	HAL_ADC_Start_DMA(hadc, pData, BSP_LCD_GetXSize());
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
		HAL_ADC_Start_DMA(hadc, pData, BSP_LCD_GetXSize());
}
