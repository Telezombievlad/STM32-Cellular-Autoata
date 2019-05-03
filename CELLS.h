// No copyright. Vladislav Aleinik, 2019

#include <stdlib.h>
#include <string.h> 

//!!! This module is actually using I2C_SCREEN video memory

//=============================================================================================
// CELLULAR AUTOMATA STATE
//=============================================================================================

static const int INT_CELLS_SIZE_X = 128;
static const int INT_CELLS_SIZE_Y = 64;
uint8_t CELLS_new[0x400];

//=============================================================================================
// CELLULAR AUTOMATA LOGIC
//=============================================================================================

uint8_t CELLS_StateAt(int x, int y)
{
	if (x < 0                ) x += SCREEN_SIZE_X;
	if (x >= INT_CELLS_SIZE_X) x -= SCREEN_SIZE_X;

	if (y < 0                ) y += SCREEN_SIZE_Y;
	if (y >= INT_CELLS_SIZE_Y) y -= SCREEN_SIZE_Y;

	if (I2C_SCREEN_GetPixel(x, y))
		return 1;
	else 
		return 0;
}

void CELLS_SetStateAt(int x, int y, bool newState)
{
	if (x < 0                ) x += SCREEN_SIZE_X;
	if (x >= INT_CELLS_SIZE_X) x -= SCREEN_SIZE_X;

	if (y < 0                ) y += SCREEN_SIZE_Y;
	if (y >= INT_CELLS_SIZE_Y) y -= SCREEN_SIZE_Y;

	I2C_SCREEN_SetPixel(x, y, newState);
}

static uint8_t GameOfLife(int x, int y);
void CELLS_CalculateIteration()
{
	for (uint8_t x = 0; x < SCREEN_SIZE_X; ++x)
	{
		for (uint8_t y = 0; y < SCREEN_SIZE_Y; ++y)
		{
			if (GameOfLife(x, y))
				CELLS_new[SCREEN_SIZE_X * (y / 8) + x] |=  (1 << (y % 8));
			else
				CELLS_new[SCREEN_SIZE_X * (y / 8) + x] &= ~(1 << (y % 8));
		}
	}

	memcpy(I2C_SCREEN_vidmem, CELLS_new, ARR_SIZE);
}

void CELLS_ResetCells()
{
	I2C_SCREEN_Clear(0);
}

//=============================================================================================
// RULES AND STARTING POSITIONS
//=============================================================================================

static uint8_t GameOfLife(int x, int y)
{
	uint8_t aliveCount = CELLS_StateAt(x-1, y-1) +
	                     CELLS_StateAt(x-1, y+0) +
	                     CELLS_StateAt(x-1, y+1) +
	                     CELLS_StateAt(x+0, y-1) +
	                     CELLS_StateAt(x+0, y+1) +
	                     CELLS_StateAt(x+1, y-1) +
	                     CELLS_StateAt(x+1, y+0) +
	                     CELLS_StateAt(x+1, y+1);

	if (aliveCount <= 1) return 0;
	if (aliveCount == 3) return 1;
	if (aliveCount >= 4) return 0;

	return CELLS_StateAt(x, y);
}
