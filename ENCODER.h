// No copyright. Vladislav Aleinik, 2019

//=============================================================================================
// CONFIGURING
//=============================================================================================

void ENCODER_Config()
{   
	// Enabling clocking on GPIOC
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOC);

    // Configuring pins
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetPinMode(GPIOC, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);

    // Setting pull mode
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_6, LL_GPIO_PULL_DOWN);
    LL_GPIO_SetPinPull(GPIOC, LL_GPIO_PIN_7, LL_GPIO_PULL_DOWN);

    // Linking pins to TIM3 timer
    LL_GPIO_SetAFPin_0_7(GPIOC, LL_GPIO_PIN_6, LL_GPIO_AF_2);
	LL_GPIO_SetAFPin_0_7(GPIOC, LL_GPIO_PIN_7, LL_GPIO_AF_2);

    // Starting a general purpose timer
    LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_TIM3);

    // Setting TIM3 to encoder mode
    LL_TIM_SetEncoderMode(TIM3, LL_TIM_ENCODERMODE_X4_TI12); 

    // Setting polarity
	LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH1, LL_TIM_IC_POLARITY_FALLING); 
	LL_TIM_IC_SetPolarity(TIM3, LL_TIM_CHANNEL_CH2, LL_TIM_IC_POLARITY_FALLING);

	// Setting max value
	LL_TIM_SetAutoReload(TIM3, 499);

    // Stuff
	LL_TIM_EnableCounter(TIM3);
}

//=============================================================================================
// MICROCONTROLLER INTERACTION
//=============================================================================================

unsigned ENCODER_GetValue()
{
    return LL_TIM_GetCounter(TIM3);
}

void ENCODER_SetValue(unsigned newState)
{
    LL_TIM_SetCounter(TIM3, newState);
}

bool ENCODER_IsUpCounting()
{
    return LL_TIM_GetCounterMode(TIM3) == LL_TIM_COUNTERMODE_UP;
}
