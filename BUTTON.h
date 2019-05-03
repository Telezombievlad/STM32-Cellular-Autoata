// No copyright. Vladislav Aleinik, 2019

//=============================================================================================
// CONFIGURING
//=============================================================================================

void BUTTON_Config()
{
	// Enabling clocking on GPIO pin
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOA);

	// Configuring pin
    LL_GPIO_SetPinMode(GPIOA, LL_GPIO_PIN_0, LL_GPIO_MODE_INPUT);
    LL_GPIO_SetPinPull(GPIOA, LL_GPIO_PIN_0, LL_GPIO_PULL_DOWN);
}

//=============================================================================================
// DIFFERENT PRESS HANDLERS
//=============================================================================================

static void DoNothing() {}

static void (*BUTTON_onActualPress  )() = DoNothing;
static void (*BUTTON_onActualUnpress)() = DoNothing;
static void (*BUTTON_ifPressed      )() = DoNothing;
static void (*BUTTON_ifNotPressed   )() = DoNothing;

void BUTTON_SetHandlerOnActualPress(void (*onActualPress)())
{
	BUTTON_onActualPress = onActualPress;
}

void BUTTON_SetHandlerOnActualUnpress(void (*onActualUnpress)())
{
	BUTTON_onActualUnpress = onActualUnpress;
}

void BUTTON_SetHandlerIfPressed(void (*ifPressed)())
{
	BUTTON_ifPressed = ifPressed;
}

void BUTTON_SetHandlerIfNotPressed(void (*ifNotPressed)())
{
	BUTTON_ifNotPressed = ifNotPressed;
}

//=============================================================================================
// BUTTON PRESS RESOLUTION
//=============================================================================================

static       uint8_t BUTTON_saturation = 0;
static       bool    BUTTON_pressed    = false;
static const uint8_t MAX_SATURATION    = 60;
static const uint8_t UNPRESS_LEVEL     = 15;
static const uint8_t   PRESS_LEVEL     = 45;

void BUTTON_UpdateState()
{
	if (LL_GPIO_IsInputPinSet(GPIOA, LL_GPIO_PIN_0))
	{
		if (BUTTON_saturation < MAX_SATURATION) BUTTON_saturation += 1;
	}
	else
	{
		if (BUTTON_saturation > 0) BUTTON_saturation -= 1;
	}

	if (BUTTON_pressed && BUTTON_saturation < UNPRESS_LEVEL)
	{
		BUTTON_pressed = false;
		BUTTON_onActualUnpress();
	}

	if (!BUTTON_pressed && BUTTON_saturation > PRESS_LEVEL)
	{
		BUTTON_pressed = true;
		BUTTON_onActualPress();
	}

	if (BUTTON_pressed)
	{
		BUTTON_ifPressed();
	}
	else
	{
		BUTTON_ifNotPressed();
	}
}

bool BUTTON_IsPressed()
{
	return BUTTON_pressed;
}