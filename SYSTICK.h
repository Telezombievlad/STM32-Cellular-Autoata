// No copyright. Vladislav Aleinik, 2019

//=============================================================================================
// SYSTICk STATE
//=============================================================================================

static void (*SYSTICK_handler)();
static const unsigned FREQUENCY = 1000;
static unsigned SYSTICK_ticks = 0;

//=============================================================================================
// CONFIGURING
//=============================================================================================

static void SYSTICK_Config(void (*handler)())
{
	SYSTICK_handler = handler;

	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_TIM1);

	LL_TIM_SetPrescaler(TIM1, 48-1);

	// Set timer mode
	LL_TIM_SetCounterMode(TIM1, LL_TIM_COUNTERMODE_UP);

	// Set auto-reload value
	LL_TIM_SetAutoReload(TIM1, FREQUENCY-1);

	// Enable interrupts
	LL_TIM_EnableIT_UPDATE(TIM1);

	// Turn the counter on
	LL_TIM_EnableCounter(TIM1);

	// Set SysTick interrupt priority
	NVIC_EnableIRQ(TIM1_BRK_UP_TRG_COM_IRQn);
	NVIC_SetPriority(TIM1_BRK_UP_TRG_COM_IRQn, 1);
}

//=============================================================================================
// INTERRUPT HANDLER
//=============================================================================================

void TIM1_BRK_UP_TRG_COM_IRQHandler(void)
{
	LL_TIM_ClearFlag_UPDATE(TIM1);

	SYSTICK_handler();

	SYSTICK_ticks = (SYSTICK_ticks + 1) % FREQUENCY;
}
//=============================================================================================
// TICKS GETTER
//=============================================================================================

unsigned SYSTICK_GetTicks()
{
	return SYSTICK_ticks;
}
