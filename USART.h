// No copyright. Vladislav Aleinik, 2019

//=============================================================================================
// CONFIGURING
//=============================================================================================

// static void usart_config(void)
// {
// 	// Configure pins
// 	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);	
// 	// Transmitter
// 	//LL_GPIO_SetPinMode  (GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
// 	//LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_0);
// 	//LL_GPIO_SetPinSpeed (GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_HIGH);
// 	// Reciever
// 	LL_GPIO_SetPinMode  (GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
// 	LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_0);
//     LL_GPIO_SetPinSpeed (GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH); 
// 	// Enable USART clocking
// 	LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
// 	LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);

// 	// Configure USART settings
// 	LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_RX); 
//     //LL_USART_SetParity(USART1, LL_USART_PARITY_NONE); 
//     // LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B); 
//     //LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1); 
//     //LL_USART_SetTransferBitOrder(USART1, LL_USART_BITORDER_LSBFIRST); 
//     //LL_USART_SetBaudRate(USART1, SystemCoreClock, LL_USART_OVERSAMPLING_16, 115200);

//     // Enable IDLE and RXNotEmpty interrupts
//     LL_USART_EnableIT_IDLE(USART1);
//     LL_USART_EnableIT_RXNE(USART1);
    
//     // Wait for USART to come living
//     LL_USART_Enable(USART1);
//     while (!LL_USART_IsActiveFlag_REACK(USART1));

//     // Enable the NVIC handling of USART
//     NVIC_SetPriority(USART1_IRQn, 0);
//     NVIC_EnableIRQ(USART1_IRQn);
// }

static void usart_config(void)
{
    /*
     * Setting USART pins
     */
    LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);
    //USART1_TX
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_6, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_6, LL_GPIO_AF_0);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_6, LL_GPIO_SPEED_FREQ_HIGH);
    //USART1_RX
    LL_GPIO_SetPinMode(GPIOB, LL_GPIO_PIN_7, LL_GPIO_MODE_ALTERNATE);
    LL_GPIO_SetAFPin_0_7(GPIOB, LL_GPIO_PIN_7, LL_GPIO_AF_0);
    LL_GPIO_SetPinSpeed(GPIOB, LL_GPIO_PIN_7, LL_GPIO_SPEED_FREQ_HIGH); 
    /*
     * USART Set clock source
     */
    LL_APB1_GRP2_EnableClock(LL_APB1_GRP2_PERIPH_USART1);
    LL_RCC_SetUSARTClockSource(LL_RCC_USART1_CLKSOURCE_PCLK1);
    /*
     * USART Setting
     */
    LL_USART_SetTransferDirection(USART1, LL_USART_DIRECTION_TX_RX);
    LL_USART_SetParity(USART1, LL_USART_PARITY_NONE);
    LL_USART_SetDataWidth(USART1, LL_USART_DATAWIDTH_8B);
    LL_USART_SetStopBitsLength(USART1, LL_USART_STOPBITS_1);
    LL_USART_SetTransferBitOrder(USART1, LL_USART_BITORDER_LSBFIRST);
    LL_USART_SetBaudRate(USART1, SystemCoreClock,
                         LL_USART_OVERSAMPLING_16, 115200);
    LL_USART_EnableIT_IDLE(USART1);
    LL_USART_EnableIT_RXNE(USART1);
    /*
     * USART turn on
     */
    LL_USART_Enable(USART1);
    while (!(LL_USART_IsActiveFlag_TEACK(USART1) &&
             LL_USART_IsActiveFlag_REACK(USART1)));
    /*
     * Turn on NVIC interrupt line
     */
    NVIC_SetPriority(USART1_IRQn, 0);
    NVIC_EnableIRQ(USART1_IRQn);
    return;
}

//=============================================================================================
// USART RECIEVING PROCESS
//=============================================================================================

struct USART_State 
{
	uint8_t cmd;
	bool active;
};

static USART_State USART_STATE = {0, 0};

void USART1_IRQHandler()
{
	if (LL_USART_IsActiveFlag_RXNE(USART1))
	{
		USART_STATE.cmd = LL_USART_ReceiveData8(USART1);
		USART_STATE.active = true;
	}
}

bool isEmptyQueue()
{
	return !USART_STATE.active;
}

uint8_t waitForCommand()
{
	while (!USART_STATE.active);

	USART_STATE.active = false;

	return USART_STATE.cmd;
}