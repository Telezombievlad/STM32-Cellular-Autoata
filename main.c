// Copyright 2019 Vladislav Aleinik
// ToDo:
// (1) Add ENCODER support
// (2) Add PWF

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"

#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_tim.h"

#include "stm32f0xx_ll_exti.h"

#include <errno.h>
#include <math.h>
#include <stdbool.h>

//=============================================================================================
// GLOBAL MC STATE
//=============================================================================================

struct State
{
	// Button
	bool upCountingMode;

	// Cells simulation:
	uint8_t cursorX;
	uint8_t cursorY;
	bool stateUnderCursor;
	bool simulationPlaying;
	unsigned iteration;

	// Iteartion timing
	unsigned iterTick;
	unsigned stepPeriod;
};

volatile struct State MC_STATE = {0, 0, 0, 0, false, 0, 0, 1000};

//=============================================================================================
// RCC_config
//=============================================================================================

static void RCC_config()
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

	// Enable Internal High Speed clock
	LL_RCC_HSI_Enable();
	while (LL_RCC_HSI_IsReady() != 1);

	// 8MHz /2 * 12 == 48MHz
	LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2,
	                            LL_RCC_PLL_MUL_12);

	// Enable PLL
	LL_RCC_PLL_Enable();
	while (LL_RCC_PLL_IsReady() != 1);

	// Enabling clocking on AHB bus
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
	while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

	SystemCoreClockUpdate();
}

//=============================================================================================
// 7-SEGMENT DISPLAY MODULE
//=============================================================================================

#include "SEG7.h"

//=============================================================================================
// ENCODER MODULE
//=============================================================================================

#include "ENCODER.h"

//=============================================================================================
// BUTTON MODULE
//=============================================================================================

#include "BUTTON.h"

void OnActualButtonPress()
{
	MC_STATE.simulationPlaying = !MC_STATE.simulationPlaying; 
}
 
//=============================================================================================
// I2C SCREEN MODULE
//=============================================================================================

#include "I2C_SCREEN.h"

//=============================================================================================
// CELLULAR AUTOMATA LOGIC
//=============================================================================================

#include "CELLS.h"

//=============================================================================================
// USART
//=============================================================================================

#include "USART.h"

enum Command
{
	MOVE_CURSOR_L,
	MOVE_CURSOR_R,
	MOVE_CURSOR_U,
	MOVE_CURSOR_D,
	DIRTY_MOVE_CURSOR_L,
	DIRTY_MOVE_CURSOR_R,
	DIRTY_MOVE_CURSOR_U,
	DIRTY_MOVE_CURSOR_D,
	TOOGLE_CELL_STATE,
	SIMULATION_PLAY,
	SIMULATION_PAUSE,
	SIMULATION_RESET
};

void executeCommand(enum Command cmd)
{
	// If DIRTY
	if ((cmd & 0xFC) == 0x04 && !MC_STATE.simulationPlaying)
	{
		MC_STATE.stateUnderCursor = !MC_STATE.stateUnderCursor;
	}

	// If MOVE
	if ((cmd & 0xF8) == 0x00 && !MC_STATE.simulationPlaying)
		I2C_SCREEN_SetPixel(MC_STATE.cursorX, MC_STATE.cursorY, MC_STATE.stateUnderCursor);
	
	// If LEFT
	if (cmd == MOVE_CURSOR_L || cmd == DIRTY_MOVE_CURSOR_L)
	{
		if (MC_STATE.cursorX == 0) MC_STATE.cursorX = SCREEN_SIZE_X - 1;
		else MC_STATE.cursorX -= 1;
	}

	// If RIGHT
	if (cmd == MOVE_CURSOR_R || cmd == DIRTY_MOVE_CURSOR_R)
	{
		if (MC_STATE.cursorX == SCREEN_SIZE_X - 1) MC_STATE.cursorX = 0;
		else MC_STATE.cursorX += 1;
	}

	// If UP
	if (cmd == MOVE_CURSOR_U || cmd == DIRTY_MOVE_CURSOR_U)
	{
		if (MC_STATE.cursorY == 0) MC_STATE.cursorY = SCREEN_SIZE_Y - 1;
		else MC_STATE.cursorY -= 1;
	}

	// If DOWN
	if (cmd == MOVE_CURSOR_D || cmd == DIRTY_MOVE_CURSOR_D)
	{
		if (MC_STATE.cursorY == SCREEN_SIZE_Y - 1) MC_STATE.cursorY = 0;
		else MC_STATE.cursorY += 1;
	}

	switch (cmd)
	{
		case TOOGLE_CELL_STATE:
		{
			MC_STATE.stateUnderCursor = !MC_STATE.stateUnderCursor;
			break;
		}
		case SIMULATION_PLAY:
		{
			MC_STATE.simulationPlaying = true;
			break;
		}
		case SIMULATION_PAUSE:
		{
			MC_STATE.simulationPlaying = false;
			break;
		}
		case SIMULATION_RESET:
		{
			MC_STATE.simulationPlaying = false;
			MC_STATE.iteration = 0;
			CELLS_ResetCells();
			break;
		}
	}

	// If MOVE
	if ((cmd & 0xF8) == 0x00 && !MC_STATE.simulationPlaying)
		MC_STATE.stateUnderCursor = I2C_SCREEN_GetPixel(MC_STATE.cursorX, MC_STATE.cursorY);
}

//=============================================================================================
// SYSTEM TIMER MODULE
//=============================================================================================

#include "SYSTICK.h"

void TickHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(TIM1);

	//================================
	// SEG7
	//================================

	// Update 7-segment display number
	SEG7_SetNumber(MC_STATE.iteration);

	// Push SEG7 state to MC
	SEG7_SetNumberQuarter(SYSTICK_GetTicks());
	SEG7_PushDisplayStateToMC();

	//================================
	// BUTTON
	//================================

    // Update button state
	BUTTON_UpdateState();

	//================================
	// ENCODER
	//================================

	if (ENCODER_GetValue() < 100) ENCODER_SetValue(110);
	if (ENCODER_GetValue() > 500) ENCODER_SetValue(490);

	MC_STATE.stepPeriod = ENCODER_GetValue()/2;
	MC_STATE.iterTick = (MC_STATE.iterTick + 1) % MC_STATE.stepPeriod;

	//================================
	// USART
	//================================

	if (USART_CommandReady())  
	{
		uint8_t cmd = USART_GetCommand();  

		executeCommand(cmd);

		I2C_SCREEN_Flush();
	}

	//================================
	// CURSOR
	//================================

	if (!MC_STATE.simulationPlaying)
	{
		if (SYSTICK_GetTicks() % 1000 == 0)
		{
			I2C_SCREEN_WriteUpdatePixel(MC_STATE.cursorX, MC_STATE.cursorY, 0);
		}
		
		if (SYSTICK_GetTicks() % 1000 == 500)
		{
			I2C_SCREEN_WriteUpdatePixel(MC_STATE.cursorX, MC_STATE.cursorY, 1);
		} 
	}  
 
	//================================
	// CELLS
	//================================

	if (MC_STATE.simulationPlaying && MC_STATE.iterTick == 0)
	{
		MC_STATE.iteration = (MC_STATE.iteration + 1) % 10000;

		I2C_SCREEN_SetPixel(MC_STATE.cursorX, MC_STATE.cursorY, MC_STATE.stateUnderCursor);

		CELLS_CalculateIteration();

		MC_STATE.stateUnderCursor = I2C_SCREEN_GetPixel(MC_STATE.cursorX, MC_STATE.cursorY);
		I2C_SCREEN_Flush();
	}
}

//=============================================================================================
// MAIN
//=============================================================================================

int main(void)
{
	RCC_config();
	
	SEG7_Config();

	ENCODER_Config();

	BUTTON_Config();
	BUTTON_SetHandlerOnActualPress(OnActualButtonPress);

	I2C_SCREEN_Config();

	I2C_SCREEN_Clear(0);
	I2C_SCREEN_Flush();

	USART_Config();	

	SYSTICK_Config(TickHandler);

	while (1) {}

	return 0;
}
