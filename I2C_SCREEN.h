// No copyright. Vladislav Aleinik, 2019

#include "stm32f0xx_ll_i2c.h"
#include <string.h>

//=============================================================================================
// CONFIGURING
//=============================================================================================

static void I2C_Config()
{
	// Set up the GPIO
	LL_AHB1_GRP1_EnableClock(LL_AHB1_GRP1_PERIPH_GPIOB);

	// SCL - PB8
	LL_GPIO_SetPinMode      (GPIOB, LL_GPIO_PIN_8, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_8, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetAFPin_8_15   (GPIOB, LL_GPIO_PIN_8, LL_GPIO_AF_1);
	LL_GPIO_SetPinSpeed     (GPIOB, LL_GPIO_PIN_8, LL_GPIO_SPEED_FREQ_HIGH);

	// SDA - PB9
	LL_GPIO_SetPinMode      (GPIOB, LL_GPIO_PIN_9, LL_GPIO_MODE_ALTERNATE);
	LL_GPIO_SetPinOutputType(GPIOB, LL_GPIO_PIN_9, LL_GPIO_OUTPUT_OPENDRAIN);
	LL_GPIO_SetAFPin_8_15   (GPIOB, LL_GPIO_PIN_9, LL_GPIO_AF_1);
	LL_GPIO_SetPinSpeed     (GPIOB, LL_GPIO_PIN_9, LL_GPIO_SPEED_FREQ_HIGH);
	
	// Configure the I2C module
	LL_RCC_SetI2CClockSource(LL_RCC_I2C1_CLKSOURCE_SYSCLK);
	LL_I2C_Disable(I2C1);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_I2C1);
	LL_I2C_DisableAnalogFilter(I2C1);
	LL_I2C_SetDigitalFilter(I2C1, 1);
	
	LL_I2C_SetTiming(I2C1, 0x50330309);
	LL_I2C_DisableClockStretching(I2C1);
	LL_I2C_SetMasterAddressingMode(I2C1, LL_I2C_ADDRESSING_MODE_7BIT);
	LL_I2C_SetMode(I2C1, LL_I2C_MODE_I2C);
	LL_I2C_Enable(I2C1);
}

static void I2C_SendCmd(uint8_t byte);

static void SCREEN_Config()
{
    // Set display OFF
    I2C_SendCmd(0xAE);

    // Set addressing mode
    // Vertical addressing mode
    I2C_SendCmd(0x20);
    I2C_SendCmd(0x10);

    // Vertical flip: 0xC0 - on, 0xC8 - off
    I2C_SendCmd(0xC8);

    // Set start line address 0-63
    I2C_SendCmd(0x40);

    // Set contrast level: 0-255
    I2C_SendCmd(0x81);
    I2C_SendCmd(0xFF);

    // Horizontal flip: 0xA1 - on, 0xA0 - off
    I2C_SendCmd(0xA1);

    // Normal colo - 0xA6, Inverse - 0xA7
    I2C_SendCmd(0xA6);

    // Number of active lines: 16 - 64
    I2C_SendCmd(0xA8);
    I2C_SendCmd(0x3F);

    // Render GRAM: 0xA4 - render, 0xA5 - black screen
    I2C_SendCmd(0xA4);

    // Set display offset: 0-63
    I2C_SendCmd(0xD3);
    I2C_SendCmd(0x00);

    // Set display refresh rate:
    // 7-4 bits set osc freq, 0-3 sets resfresh ratio
    I2C_SendCmd(0xD5);
    I2C_SendCmd(0xF0);

    // Set flipping config
    I2C_SendCmd(0xDA);
    I2C_SendCmd(0x12);

    // Enable charge pump
    I2C_SendCmd(0x8D);
    I2C_SendCmd(0x14);

    // Turn on display
    I2C_SendCmd(0xAF);
}

void I2C_SCREEN_Config()
{
	I2C_Config();

	// Wait untile I2C configuring is done
	for (unsigned delay = 2000000; delay > 0; --delay);

	SCREEN_Config();
}

//=============================================================================================
// I2C INTERACTION
//=============================================================================================

static void I2C_SendCmd(uint8_t byte)
{
    /*
     * Initiate transmission
     * Display address = 0x78
     */
    LL_I2C_HandleTransfer(I2C1, 0x78, LL_I2C_ADDRSLAVE_7BIT,
                          2, LL_I2C_MODE_AUTOEND,
                          LL_I2C_GENERATE_START_WRITE);
    /*
     * Send Control byte (Co = 0, D/C# = 0)
     */
    while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, 0x00);
    
    // Send cmd 
    while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, byte);
    
    // Wait for end of transmission
    while (LL_I2C_IsActiveFlag_TC(I2C1));
}

static void I2C_SendData(uint8_t* bytes, uint8_t size)
{
    uint8_t i;

    /*
     * Initiate transmission
     * Display address = 0x78
     */
    LL_I2C_HandleTransfer(I2C1, 0x78, LL_I2C_ADDRSLAVE_7BIT,
                          size + 1, LL_I2C_MODE_AUTOEND,
                          LL_I2C_GENERATE_START_WRITE);
    /*
     * Send Control byte (Co = 0, D/C# = 1)
     */
    while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
    LL_I2C_TransmitData8(I2C1, 0x40);
    
    // Send data
    for (i = 0; i < size; i++) {
        while (!LL_I2C_IsActiveFlag_TXIS(I2C1));
        LL_I2C_TransmitData8(I2C1, bytes[i]);
    }

    // Wait for end of transmission
    while (LL_I2C_IsActiveFlag_TC(I2C1));
}

//=============================================================================================
// VIDEO MEMORY MAP
//=============================================================================================

const unsigned SCREEN_SIZE_X = 128;
const unsigned SCREEN_SIZE_Y = 64;  
const unsigned ARR_SIZE = 0x400; // 128 * 64 / 8;
uint8_t I2C_SCREEN_vidmem[0x400];

enum Colors
{
	SCREEN_COLOR_BLACK = 0x00,
	SCREEN_COLOR_WHITE = 0xFF
};

void I2C_SCREEN_Clear(bool color)
{
	memset(I2C_SCREEN_vidmem, color ? SCREEN_COLOR_WHITE : SCREEN_COLOR_BLACK, ARR_SIZE);
}

void I2C_SCREEN_Flush()
{
    for (uint8_t i = 0; i < 8; i++)
    {
        // Set page start address (index of vertical byte)
        I2C_SendCmd(0xB0 + i);
        // Set lower column index
        I2C_SendCmd(0x00);
        // Set higher column index
        I2C_SendCmd(0x10);
        // Send 128 pixels
        I2C_SendData(&I2C_SCREEN_vidmem[SCREEN_SIZE_X * i], SCREEN_SIZE_X);
    }
}

void I2C_SCREEN_SetPixel(uint8_t x, uint8_t y, bool color)
{
    if (x >= SCREEN_SIZE_X || y >= SCREEN_SIZE_Y) return;

    if (color) I2C_SCREEN_vidmem[SCREEN_SIZE_X * (y / 8) + x] |=  (1 << (y % 8));
    else       I2C_SCREEN_vidmem[SCREEN_SIZE_X * (y / 8) + x] &= ~(1 << (y % 8));
}

bool I2C_SCREEN_GetPixel(uint8_t x, uint8_t y)
{
    return I2C_SCREEN_vidmem[SCREEN_SIZE_X * (y / 8) + x] & (1 << (y % 8));
}

void I2C_SCREEN_WriteUpdatePixel(uint8_t x, uint8_t y, bool color)
{
    I2C_SCREEN_SetPixel(x, y, color);

    I2C_SendCmd(0xB0 + y/8);
    // Set lower column index
    I2C_SendCmd(x & 0x0F);
    // Set higher column index
    I2C_SendCmd(0x10 + (x / 16));
    // Send 128 pixels
    I2C_SendData(&I2C_SCREEN_vidmem[SCREEN_SIZE_X * (y/8) + x], 1);
}

