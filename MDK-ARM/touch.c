#include <stdio.h>
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

uint16_t	*pData, holdoff=3000;
int32_t		ndtr,lcdw;

void mouseInit(uint32_t x,uint32_t y) {
	lcdw=x;
	ndtr=EOF;
	BSP_TS_Init(x, y);
	pData=calloc(1,4*2*x*sizeof(uint16_t));
	HAL_ADC_Start_DMA(&hadc3, (uint32_t *)pData, 4*2*x);
	//HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
	HAL_TIM_OnePulse_Start(&htim1, TIM_CHANNEL_1);
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
	
	
	if(holdoff == 1) {
		uint32_t n =(((2*4*lcdw - ndtr) & ~1) - 2*lcdw/10) % (2*4*lcdw);
		BSP_LCD_Clear(LCD_COLOR_DARKGRAY);
		BSP_LCD_SetTextColor(LCD_COLOR_LIGHTGRAY);
		for(int i=0; i<BSP_LCD_GetXSize(); i+=BSP_LCD_GetXSize()/10)
			BSP_LCD_DrawVLine(i, 0, BSP_LCD_GetYSize());
		for(int i=0; i<BSP_LCD_GetYSize(); i+=BSP_LCD_GetYSize()/5)
			BSP_LCD_DrawHLine(0, i, BSP_LCD_GetXSize());	
		for(int i=0; i<lcdw; ++i) {
			BSP_LCD_DrawPixel(i,(0x7ff-(0xfff & pData[n]))/30+BSP_LCD_GetYSize()/2,LCD_COLOR_LIGHTYELLOW);
			BSP_LCD_DrawPixel(i,(0x7ff-(0xfff & pData[n+1]))/30+BSP_LCD_GetYSize()/2,LCD_COLOR_LIGHTCYAN);
			n = (n+2) % (2*4*lcdw);
		}
		HAL_ADC_Start_DMA(&hadc3, (uint32_t *)pData, 4*2*lcdw);
		ndtr=EOF;
		holdoff=0;
			__HAL_TIM_ENABLE(&htim1);

	}
}

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef* hadc) {
	if(ndtr == EOF) {
		ndtr=hadc->DMA_Handle->Instance->NDTR;
	}
}

void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef* hadc) {
	if(ndtr == EOF || holdoff)
		return;
	if(ndtr <= 2*lcdw || ndtr >= 3*2*lcdw) {
		HAL_ADC_Stop_DMA(&hadc3);
		holdoff=200;
	}
}

void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc) {
	if(ndtr == EOF || holdoff)
		return;
	if(ndtr > 2*lcdw || ndtr < 3*2*lcdw) {
		HAL_ADC_Stop_DMA(&hadc3);
		holdoff=200;
	}
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc) {
//		HAL_ADC_Stop_DMA(&hadc3);
}

void	HAL_SYSTICK_Callback(void) {
	if(holdoff)
		--holdoff;
}

