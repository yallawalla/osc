#include <stdlib.h>
#include "touch.h"
#include "stm32746g_discovery_ts.h"
#include "stm32746g_discovery_lcd.h"

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

mouse	m ={ fup, fdown, fmove, 0};
TS_StateTypeDef TS_State;

uint16_t	*pData, *trigger, ndtr, holdoff=3000;

void mouseInit(uint32_t x,uint32_t y) {
	BSP_TS_Init(x, y);
	pData=calloc(1,4*2*x*sizeof(uint16_t));
	HAL_ADC_Start_DMA(&hadc3, (uint32_t *)pData, 4*2*x);
	HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
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
	if(!trigger && !holdoff) {
		ndtr=hadc->DMA_Handle->Instance->NDTR;
		trigger=&pData[2*4*BSP_LCD_GetXSize() - ndtr];
		trigger = (uint16_t *)((uint32_t)trigger & ~3);
		holdoff=200;
		
		BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
		for(int i=0; i<BSP_LCD_GetXSize(); i+=BSP_LCD_GetXSize()/10)
			BSP_LCD_DrawVLine(i, 0, BSP_LCD_GetYSize());
		for(int i=0; i<BSP_LCD_GetYSize(); i+=BSP_LCD_GetYSize()/5)
			BSP_LCD_DrawHLine(0, i, BSP_LCD_GetXSize());
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	if(trigger && (trigger < &pData[BSP_LCD_GetXSize()] || trigger > &pData[3*BSP_LCD_GetXSize()/4])) {
		HAL_ADC_Stop_DMA(&hadc3);
		uint16_t *p=trigger-48*2*1;
		if(p < pData)
			p=&pData[4*2*BSP_LCD_GetXSize() + p - trigger];
		for(int i=0; i<BSP_LCD_GetXSize(); ++i) {
			BSP_LCD_DrawPixel(i,(0x7ff-(*p))/30+BSP_LCD_GetYSize()/2,LCD_COLOR_LIGHTYELLOW);
			++p;
			BSP_LCD_DrawPixel(i,(0x7ff-(*p))/30+BSP_LCD_GetYSize()/2,LCD_COLOR_LIGHTCYAN);
			if (++p >= &pData[4*2*BSP_LCD_GetXSize()])
				p=pData;
		}
		trigger=NULL;
		HAL_ADC_Start_DMA(&hadc3, (uint32_t *)pData, 4*2*BSP_LCD_GetXSize());
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if(trigger && (trigger > &pData[BSP_LCD_GetXSize()] && trigger < &pData[3*BSP_LCD_GetXSize()/4])) {
		HAL_ADC_Stop_DMA(&hadc3);
		uint16_t *p=trigger-48*2*1;
		for(int i=0; i<BSP_LCD_GetXSize(); ++i) {
			BSP_LCD_DrawPixel(i,(0x7ff-(*p))/30+BSP_LCD_GetYSize()/2,LCD_COLOR_LIGHTYELLOW);
			++p;
			BSP_LCD_DrawPixel(i,(0x7ff-(*p))/30+BSP_LCD_GetYSize()/2,LCD_COLOR_LIGHTCYAN);
			if (++p >= &pData[4*2*BSP_LCD_GetXSize()])
				p=pData;
		}
		holdoff=1000;
		trigger=NULL;
		HAL_ADC_Start_DMA(&hadc3, (uint32_t *)pData, 4*2*BSP_LCD_GetXSize());
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
		HAL_ADC_Stop_DMA(&hadc3);
}

void	HAL_SYSTICK_Callback(void) {
	if(holdoff)
		--holdoff;
}

