#include "touch.h"
#include "stm32746g_discovery_ts.h"

int test;

	void fup(uint32_t x, uint32_t y) {
		test=1;
	}
	void fdown(uint32_t x, uint32_t y) {
		test=2;
	}
	void fmove(uint32_t x,uint32_t y) {
		test=3;
	}



mouse	m ={fup,fdown,fmove,0};
TS_StateTypeDef TS_State;

void mouseInit(uint32_t x,uint32_t y) {
	BSP_TS_Init(x, y);
}

void mouseScan(void) {
	BSP_TS_GetState(&TS_State);
	if(TS_State.touchDetected) {
		if(m.detected == 0) {
			m.x = TS_State.touchX[0];
			m.y = TS_State.touchY[0];
			m.dx = m.dy = 0;
			if(m.down)
				m.down(m.x,m.y);
		} else {
			m.dx = TS_State.touchX[0] - m.x;
			m.dy = TS_State.touchY[0] - m.y;
			m.x = TS_State.touchX[0];
			m.y = TS_State.touchY[0];
		}
		m.detected = TS_State.touchDetected;
		if(m.move && (m.dx || m.dy))
			m.move(m.dx,m.dy);
	} else {
		if(m.detected && m.up)
			m.up(m.x,m.y);
		m.detected=0;
	}
}
