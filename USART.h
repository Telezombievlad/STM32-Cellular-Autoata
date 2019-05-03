// No copyright. Vladislav Aleinik, 2019

#include "stm32f0xx_ll_usart.h"

//=============================================================================================
// CONFIGURING
//=============================================================================================

void USART_Config(void)
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

// Circular buffer here:
static const uint8_t CMD_BUFFER_SIZE = 64;
static uint8_t USART_cmds[64];
static uint8_t USART_bufferBeg = 0;
static uint8_t USART_elements  = 0;

void USART1_IRQHandler()
{
	if (LL_USART_IsActiveFlag_RXNE(USART1))
	{
		uint8_t cmd = LL_USART_ReceiveData8(USART1);

        if (USART_elements == CMD_BUFFER_SIZE) return;
        
        USART_cmds[(USART_bufferBeg + USART_elements) % CMD_BUFFER_SIZE] = cmd;
        USART_elements++;
	}
    if (LL_USART_IsActiveFlag_IDLE(USART1))
    {
        LL_USART_ClearFlag_IDLE(USART1);
    }
}

bool USART_CommandReady()
{
	return USART_elements > 0;
}

uint8_t USART_GetCommand()
{	
    while (!USART_CommandReady());

    uint8_t toReturn = USART_cmds[USART_bufferBeg];
    
    USART_elements--;
    USART_bufferBeg = (USART_bufferBeg + 1) % CMD_BUFFER_SIZE;

	return toReturn;
}