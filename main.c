// No copyright. Vladislav Aleinik, 2019

#include "stm32f0xx_ll_rcc.h"
#include "stm32f0xx_ll_system.h"
#include "stm32f0xx_ll_bus.h"
#include "stm32f0xx_ll_gpio.h"

#include "stm32f0xx_ll_usart.h"

#include "stm32f0xx_ll_cortex.h"
#include "stm32f0xx_ll_utils.h"
#include "stm32f0xx_ll_tim.h"

#include "stm32f0xx_ll_exti.h"

// ToDo:
// [X] Speedy and stable cursor
// [X] Keyboard shit
// [X] Add encoder and 7-segment display
// [X] Add LED screen

//=============================================================================================
// CONFIGURING STUFF
//=============================================================================================

static void rcc_config()
{
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);

    LL_RCC_HSI_Enable();
    while (LL_RCC_HSI_IsReady() != 1);

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI_DIV_2, LL_RCC_PLL_MUL_12);

    LL_RCC_PLL_Enable();
    while (LL_RCC_PLL_IsReady() != 1);

    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL);

    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);

    SystemCoreClock = 48000000;
}

static void systick_config()
{
	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

	LL_TIM_SetPrescaler(TIM1, 48-1);

	// Set timer mode
	LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);

	// Set auto-reload value
	LL_TIM_SetAutoReload(TIM1, 1000-1);

	// Enable interrupts
	LL_TIM_EnableIT_UPDATE(TIM1);

	// Turn the counter on
	LL_TIM_EnableCounter(TIM1);

	// Set SysTick interrupt priority
	NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 0);
}

static void diods_config()
{
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_8, LL_GPIO_MODE_OUTPUT);
	LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_9, LL_GPIO_MODE_OUTPUT);
}

void screen_config()
{

}

//=============================================================================================
// MICROCONTROLLER STATE
//=============================================================================================

const int SIZE_X = 128;
const int SIZE_Y = 64;
const unsigned ARR_SIZE = 0x2000;
struct MC_State
{
	// Cell states
	uint8_t curCells[ARR_SIZE];
	uint8_t newCells[ARR_SIZE];

	bool simulationPlaying;

	// Cursor
	int cursorX;
	int cursorY;

	// Simulation tick
	unsigned tick;
};

struct MC_State MC_STATE = {{0}, {0}, 0, 1, 1, 0};

//=============================================================================================
// CELLULAR AUTOMATA LOGIC
//=============================================================================================

#include "CellularAutomata.h"

//=============================================================================================
// USART INTERACTION
//=============================================================================================

#include "USART.h"

//=============================================================================================
// RECIEVED COMMANDS
//=============================================================================================

enum Command
{
	MOVE_CURSOR_LEFT,
	MOVE_CURSOR_RIGHT,
	MOVE_CURSOR_UP,
	MOVE_CURSOR_DOWN,
	TOOGLE_CELL_STATE,
	SIMULATION_PLAY,
	SIMULATION_PAUSE,
	SIMULATION_RESET
};

void executeCommand(uint8_t cmd)
{
	switch (cmd)
	{
		case MOVE_CURSOR_LEFT:
		{
			if (MC_STATE.cursorX == 0) MC_STATE.cursorX = SIZE_X - 1;
			else MC_STATE.cursorX -= 1;
			break;
		}
		case MOVE_CURSOR_RIGHT:
		{
			if (MC_STATE.cursorX == SIZE_X - 1) MC_STATE.cursorX = 0;
			else MC_STATE.cursorX += 1;
			break;
		}
		case MOVE_CURSOR_UP:
		{
			if (MC_STATE.cursorY == 0) MC_STATE.cursorY = SIZE_Y - 1;
			else MC_STATE.cursorY -= 1;
			break;
		}
		case MOVE_CURSOR_DOWN:
		{
			if (MC_STATE.cursorY == SIZE_Y - 1) MC_STATE.cursorY = 0;
			else MC_STATE.cursorY += 1;
			break;
		}
		case TOOGLE_CELL_STATE:
		{
			toogleCellState(MC_STATE.cursorX, MC_STATE.cursorY);
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
			resetCells();
			break;
		}
	}
}

//=============================================================================================
// MICROCONTROLLER LOGIC
//=============================================================================================

void tick()
{
	MC_STATE.tick = (MC_STATE.tick + 1) % 1000; 
}

//=============================================================================================
// INTERACT WITH MC
//=============================================================================================

void writeState()
{
	tick();
  
	if (MC_STATE.tick > 500) LL_GPIO_SetOutputPin  (GPIOC, LL_GPIO_PIN_8);   
	else                     LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_8);  
 
	if (MC_STATE.tick > 500) LL_GPIO_SetOutputPin  (GPIOC, LL_GPIO_PIN_9);   
	else                     LL_GPIO_ResetOutputPin(GPIOC, LL_GPIO_PIN_9);
}

//=============================================================================================
// INTERRUPTS
//=============================================================================================

void TIM1_CC_IRQHandler(void)
{
	LL_TIM_ClearFlag_CC1(TIM1);

	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_8);

	if (!isEmptyQueue())
	{
		executeCommand(waitForCommand());
	}

	writeState();
}

//=============================================================================================
// MAIN
//=============================================================================================

int main()
{
	rcc_config();
	systick_config(); 
	screen_config();    
	diods_config();       
	usart_config();   
   
	LL_GPIO_SetOutputPin(GPIOC, LL_GPIO_PIN_9);      

	while (true) {}

	return 0;
}