// No copyright. Vladislav Aleinik, 2019

#include <stdlib.h>
#include <string.h> 

//=============================================================================================
// CELLULAR AUTOMATA LOGIC
//=============================================================================================

const unsigned STATE_COUNT = 2;

inline uint8_t stateAt(int x, int y)
{
	if (x < 0      ) x += SIZE_X;
	if (x >= SIZE_X) x -= SIZE_X;

	if (y < 0      ) y += SIZE_Y;
	if (y >= SIZE_Y) y -= SIZE_Y;

	return MC_STATE.curCells[x * SIZE_Y + y];
}

void toogleCellState(int x, int y)
{
	MC_STATE.curCells[x * SIZE_Y + y] += 1;
	MC_STATE.curCells[x * SIZE_Y + y] %= STATE_COUNT;
}

uint8_t ruleGameOfLife(int x, int y)
{
	uint8_t aliveCount = stateAt(x-1, y-1) +
	                     stateAt(x-1, y+0) +
	                     stateAt(x-1, y+1) +
	                     stateAt(x+0, y-1) +
	                     stateAt(x+0, y+1) +
	                     stateAt(x+1, y-1) +
	                     stateAt(x+1, y+0) +
	                     stateAt(x+1, y+1);

	if (aliveCount <= 1) return 0;
	if (aliveCount == 3) return 1;
	if (aliveCount >= 4) return 0;

	return stateAt(x, y);
}

void calculateIteration()
{
	for (int x = 0; x < SIZE_X; ++x)
	{
		for (int y = 0; y < SIZE_Y; ++y)
		{
			MC_STATE.newCells[x * SIZE_Y + y] = ruleGameOfLife(x, y);
		}
	}

	memcpy(MC_STATE.curCells, MC_STATE.newCells, ARR_SIZE * sizeof(uint8_t));
}

void resetCells()
{
	for (int x = 0; x < SIZE_X; ++x)
	{
		for (int y = 0; y < SIZE_Y; ++y)
		{
			MC_STATE.curCells[x * SIZE_Y + y] = 0;
		}
	}
}